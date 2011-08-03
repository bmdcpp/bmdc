/*
 * Copyright (C) 2001-2011 Jacek Sieka, arnetheduck on gmail point com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include "stdinc.h"
#include "format.h"
#include "DownloadManager.h"

#include "QueueManager.h"
#include "Download.h"
#include "LogManager.h"
#include "SFVReader.h"
#include "User.h"
#include "File.h"
#include "FilteredFile.h"
#include "MerkleCheckOutputStream.h"
#include "UserConnection.h"
#include "ZUtils.h"

#include "ClientManager.h"

#include <limits>
#include <cmath>

// some strange mac definition
#ifdef ff
#undef ff
#endif

namespace dcpp {

static const string DOWNLOAD_AREA = "Downloads";

DownloadManager::DownloadManager() {
	TimerManager::getInstance()->addListener(this);
}

DownloadManager::~DownloadManager() throw() {
	TimerManager::getInstance()->removeListener(this);
	while(true) {
		{
			Lock l(cs);
			if(downloads.empty())
				break;
		}
		Thread::sleep(100);
	}
}

void DownloadManager::on(TimerManagerListener::Second, uint64_t aTick) throw() {
	typedef vector<pair<string, UserPtr> > TargetList;
	typedef vector<pair<uint64_t, UserPtr> > SlowChecks; //RSX++
	TargetList dropTargets;
	SlowChecks slowdl; //RSX++

	{
		Lock l(cs);

		DownloadList tickList;

		// Tick each ongoing download
		for(DownloadList::iterator i = downloads.begin(); i != downloads.end(); ++i) {
			if((*i)->getPos() > 0) {
				tickList.push_back(*i);
				(*i)->tick();
			}
		}

		if(tickList.size() > 0)
			fire(DownloadManagerListener::Tick(), tickList);

		// Automatically remove or disconnect slow sources
		if((uint32_t)(aTick / 1000) % SETTING(AUTODROP_INTERVAL) == 0) {
			for(DownloadList::iterator i = downloads.begin(); i != downloads.end(); ++i) {
				Download* d = *i;
				uint64_t timeElapsed = GET_TICK() - d->getStart();
				uint64_t timeInactive = GET_TICK() - d->getUserConnection().getLastActivity();
				uint64_t bytesDownloaded = d->getPos();
				bool timeElapsedOk = timeElapsed >= (uint32_t)SETTING(AUTODROP_ELAPSED) * 1000;
				bool timeInactiveOk = timeInactive <= (uint32_t)SETTING(AUTODROP_INACTIVITY) * 1000;
				bool speedTooLow = timeElapsedOk && timeInactiveOk && bytesDownloaded > 0 ?
					bytesDownloaded / timeElapsed * 1000 < (uint32_t)SETTING(AUTODROP_SPEED) : false;
				bool isUserList = d->getType() == Transfer::TYPE_FULL_LIST;
				bool onlineSourcesOk = isUserList ?
					true : QueueManager::getInstance()->countOnlineSources(d->getPath()) >= SETTING(AUTODROP_MINSOURCES);
				bool filesizeOk = !isUserList && d->getSize() >= ((int64_t)SETTING(AUTODROP_FILESIZE)) * 1024;
				bool dropIt = (isUserList && BOOLSETTING(AUTODROP_FILELISTS)) ||
					(filesizeOk && BOOLSETTING(AUTODROP_ALL));
				if(speedTooLow && onlineSourcesOk && dropIt) {
					if(BOOLSETTING(AUTODROP_DISCONNECT) && isUserList) {
						d->getUserConnection().disconnect();
					} else {
						if(BOOLSETTING(USE_SDL_KICK))
						{
							if(d->getType() == Transfer::TYPE_FULL_LIST && d->getStart() > 0) {
								if(d->isSet(Download::FLAG_CHECK_FILE_LIST)) {
									if(d->getAverageSpeed() < SETTING(SDL_SPEED)) {
										if(((aTick - d->getLastTick())/1000) > (uint32_t)SETTING(SDL_TIME)) {
											slowdl.push_back(make_pair(d->getAverageSpeed(), d->getUser()));
										}
								} else {
										d->setLastTick(aTick);
									}
								}
							}
						}
						dropTargets.push_back(make_pair(d->getPath(), d->getUser()));
					}
				}
			}
		}



	}
	//RSX++
	for(SlowChecks::iterator j = slowdl.begin(); j != slowdl.end(); ++j) {
		string cheat = str(boost::format("Too low download speed (%1%/s)") % Util::formatBytes(j->first));
		ClientManager::getInstance()->setCheating(j->second, "LowSpeed", cheat, SETTING(SDL_RAW), BOOLSETTING(SHOW_SDL_RAW), false, true, false, true);
	}
	//END

	for(TargetList::iterator i = dropTargets.begin(); i != dropTargets.end(); ++i) {
		QueueManager::getInstance()->removeSource(i->first, i->second, QueueItem::Source::FLAG_SLOW_SOURCE);
	}
}

void DownloadManager::checkIdle(const UserPtr& user) {
	Lock l(cs);
	for(UserConnectionList::iterator i = idlers.begin(); i != idlers.end(); ++i) {
		UserConnection* uc = *i;
		if(uc->getUser() == user) {
			uc->updated();
			return;
		}
	}
}

void DownloadManager::addConnection(UserConnectionPtr conn) {
	if(!conn->isSet(UserConnection::FLAG_SUPPORTS_TTHL) || !conn->isSet(UserConnection::FLAG_SUPPORTS_ADCGET)) {
		// Can't download from these...
		//RSX++ // No TTHL/ADCGET support///BMDC++ F->L
		ClientManager::getInstance()->setCheating(conn->getUser(), "", "No TTHL/ADCGET support", 0,true /*RSXPP_BOOLSETTING(SHOW_NO_TTHF)*/, true, true, true, true);//add settings NO_TTHF
		//END
		conn->getUser()->setFlag(User::OLD_CLIENT);
		QueueManager::getInstance()->removeSource(conn->getUser(), QueueItem::Source::FLAG_NO_TTHF);
		conn->disconnect();
		return;
	}

	conn->addListener(this);
	checkDownloads(conn);
}

bool DownloadManager::startDownload(QueueItem::Priority prio) {
	size_t downloadCount = getDownloadCount();

	bool full = (SETTING(DOWNLOAD_SLOTS) != 0) && (downloadCount >= (size_t)SETTING(DOWNLOAD_SLOTS));
	full = full || ((SETTING(MAX_DOWNLOAD_SPEED) != 0) && (getRunningAverage() >= (SETTING(MAX_DOWNLOAD_SPEED)*1024)));

	if(full) {
		bool extraFull = (SETTING(DOWNLOAD_SLOTS) != 0) && (getDownloadCount() >= (size_t)(SETTING(DOWNLOAD_SLOTS)+3));
		if(extraFull) {
			return false;
		}
		return prio == QueueItem::HIGHEST;
	}

	if(downloadCount > 0) {
		return prio != QueueItem::LOWEST;
	}

	return true;
}

void DownloadManager::checkDownloads(UserConnection* aConn) {
	dcassert(aConn->getDownload() == NULL);

	QueueItem::Priority prio = QueueManager::getInstance()->hasDownload(aConn->getUser());
	if(!startDownload(prio)) {
		removeConnection(aConn);
		return;
	}

	Download* d = QueueManager::getInstance()->getDownload(*aConn, aConn->isSet(UserConnection::FLAG_SUPPORTS_TTHL));

	if(!d) {
		Lock l(cs);
		aConn->setState(UserConnection::STATE_IDLE);
		idlers.push_back(aConn);
		return;
	}

	if(d->isSet(Download::FLAG_TESTSUR) && aConn->isSet(UserConnection::FLAG_NMDC)) {
		aConn->getListLen();
	}


	aConn->setState(UserConnection::STATE_SND);

	if(aConn->isSet(UserConnection::FLAG_SUPPORTS_XML_BZLIST) && d->getType() == Transfer::TYPE_FULL_LIST) {
		d->setFlag(Download::FLAG_XML_BZ_LIST);
	}

	{
		Lock l(cs);
		downloads.push_back(d);
	}
	fire(DownloadManagerListener::Requesting(), d);

	dcdebug("Requesting " I64_FMT "/" I64_FMT "\n", d->getStartPos(), d->getSize());

	aConn->send(d->getCommand(aConn->isSet(UserConnection::FLAG_SUPPORTS_ZLIB_GET)));
}

void DownloadManager::on(AdcCommand::SND, UserConnection* aSource, const AdcCommand& cmd) throw() {
	if(aSource->getState() != UserConnection::STATE_SND) {
		dcdebug("DM::onFileLength Bad state, ignoring\n");
		return;
	}

	const string& type = cmd.getParam(0);
	int64_t start = Util::toInt64(cmd.getParam(2));
	int64_t bytes = Util::toInt64(cmd.getParam(3));
	//
	//RSX++ // set filelist size
	const DownloadPtr download = aSource->getDownload();
	if(download && download->getPos() == 0 && download->isSet(Download::FLAG_CHECK_FILE_LIST)) {
		ClientManager::getInstance()->setListSize(aSource->getUser(), bytes, true);
	}
	//END


	if(type != Transfer::names[aSource->getDownload()->getType()]) {
		// Uhh??? We didn't ask for this...
		aSource->disconnect();
		return;
	}

	startData(aSource, start, bytes, cmd.hasFlag("ZL", 4));
}

void DownloadManager::startData(UserConnection* aSource, int64_t start, int64_t bytes, bool z) {
	Download* d = aSource->getDownload();
	dcassert(d != NULL);

	dcdebug("Preparing " I64_FMT ":" I64_FMT ", " I64_FMT ":" I64_FMT"\n", d->getStartPos(), start, d->getSize(), bytes);
	if(d->getSize() == -1) {
		if(bytes >= 0) {
			d->setSize(bytes);
		} else {
			failDownload(aSource, _("Invalid size"));
			return;
		}
	} else if(d->getSize() != bytes || d->getStartPos() != start) {
		// This is not what we requested...
		failDownload(aSource, _("Response does not match request"));
		return;
	}

	try {
		QueueManager::getInstance()->setFile(d);
	} catch(const FileException& e) {
		failDownload(aSource, str(F_("Could not open target file: %1%") % e.getError()));
		return;
	} catch(const Exception& e) {
		failDownload(aSource, e.getError());
		return;
	}

	if((d->getType() == Transfer::TYPE_FILE || d->getType() == Transfer::TYPE_FULL_LIST) && SETTING(BUFFER_SIZE) > 0 ) {
		d->setFile(new BufferedOutputStream<true>(d->getFile()));
	}

	if(d->getType() == Transfer::TYPE_FILE) {
		typedef MerkleCheckOutputStream<TigerTree, true> MerkleStream;

		d->setFile(new MerkleStream(d->getTigerTree(), d->getFile(), d->getStartPos()));
		d->setFlag(Download::FLAG_TTH_CHECK);
	}

	// Check that we don't get too many bytes
	d->setFile(new LimitedOutputStream<true>(d->getFile(), bytes));

	if(z) {
		d->setFlag(Download::FLAG_ZDOWNLOAD);
		d->setFile(new FilteredOutputStream<UnZFilter, true>(d->getFile()));
	}

	d->setStart(GET_TICK());
	d->tick();
	aSource->setState(UserConnection::STATE_RUNNING);

	fire(DownloadManagerListener::Starting(), d);

	if(d->getPos() == d->getSize()) {
		try {
			// Already finished? A zero-byte file list could cause this...
			endData(aSource);
		} catch(const Exception& e) {
			failDownload(aSource, e.getError());
		}
	} else {
		aSource->setDataMode();
	}
}

void DownloadManager::on(UserConnectionListener::Data, UserConnection* aSource, const uint8_t* aData, size_t aLen) throw() {
	Download* d = aSource->getDownload();
	dcassert(d != NULL);

	try {
		d->addPos(d->getFile()->write(aData, aLen), aLen);
		d->tick();

		if(d->getFile()->eof()) {
			endData(aSource);
			aSource->setLineMode(0);
		}
	} catch(const Exception& e) {
		failDownload(aSource, e.getError());
	}
}

/** Download finished! */
void DownloadManager::endData(UserConnection* aSource) {
	dcassert(aSource->getState() == UserConnection::STATE_RUNNING);
	Download* d = aSource->getDownload();
	dcassert(d != NULL);

	if(d->getType() == Transfer::TYPE_TREE) {
		d->getFile()->flush();

		int64_t bl = 1024;
		while(bl * (int64_t)d->getTigerTree().getLeaves().size() < d->getTigerTree().getFileSize())
			bl *= 2;
		d->getTigerTree().setBlockSize(bl);
		d->getTigerTree().calcRoot();

		if(!(d->getTTH() == d->getTigerTree().getRoot())) {
			// This tree is for a different file, remove from queue...
			removeDownload(d);
			fire(DownloadManagerListener::Failed(), d, _("Full tree does not match TTH root"));

			QueueManager::getInstance()->removeSource(d->getPath(), aSource->getUser(), QueueItem::Source::FLAG_BAD_TREE, false);

			QueueManager::getInstance()->putDownload(d, false);

			checkDownloads(aSource);
			return;
		}
		d->setTreeValid(true);
	} else {
		// First, finish writing the file (flushing the buffers and closing the file...)
		try {
			d->getFile()->flush();
		} catch(const Exception& e) {
			d->resetPos();
			failDownload(aSource, e.getError());
			return;
		}

		aSource->setSpeed(d->getAverageSpeed());
		aSource->updateChunkSize(d->getTigerTree().getBlockSize(), d->getSize(), GET_TICK() - d->getStart());

		dcdebug("Download finished: %s, size " I64_FMT ", downloaded " I64_FMT "\n", d->getPath().c_str(), d->getSize(), d->getPos());

		if(BOOLSETTING(LOG_DOWNLOADS) && (BOOLSETTING(LOG_FILELIST_TRANSFERS) || d->getType() == Transfer::TYPE_FILE)) {
			logDownload(aSource, d);
		}
	}

	removeDownload(d);
	fire(DownloadManagerListener::Complete(), d);

	QueueManager::getInstance()->putDownload(d, true);
	checkDownloads(aSource);
}

int64_t DownloadManager::getRunningAverage() {
	Lock l(cs);
	int64_t avg = 0;
	for(DownloadList::iterator i = downloads.begin(); i != downloads.end(); ++i) {
		Download* d = *i;
		avg += d->getAverageSpeed();
	}
	return avg;
}

void DownloadManager::logDownload(UserConnection* aSource, Download* d) {
	StringMap params;
	d->getParams(*aSource, params);
	LOG(LogManager::DOWNLOAD, params);
}

void DownloadManager::on(UserConnectionListener::MaxedOut, UserConnection* aSource) throw() {
	noSlots(aSource);
}

void DownloadManager::noSlots(UserConnection* aSource) {
	if(aSource->getState() != UserConnection::STATE_SND) {
		dcdebug("DM::noSlots Bad state, disconnecting\n");
		aSource->disconnect();
		return;
	}

	failDownload(aSource, _("No slots available"));
}

void DownloadManager::on(UserConnectionListener::Failed, UserConnection* aSource, const string& aError) throw() {
	{
		Lock l(cs);
 		idlers.erase(remove(idlers.begin(), idlers.end(), aSource), idlers.end());
	}
	failDownload(aSource, aError);
}

void DownloadManager::failDownload(UserConnection* aSource, const string& reason) {

	Download* d = aSource->getDownload();

	if(d) {
		removeDownload(d);
		fire(DownloadManagerListener::Failed(), d, reason);

		if (d->getType() == Transfer::TYPE_FULL_LIST && reason == string("Disconnected")) {
			ClientManager::getInstance()->fileListDisconnected(aSource->getUser());
		} else if( d->isSet(Download::FLAG_TESTSUR) ) {
			if(reason == string("No slots available"))
				ClientManager::getInstance()->setCheating(aSource->getUser(), "MaxedOut", "No slots for TestSUR - SlotLocker", -1, true, true, false, true, false);
			else
				ClientManager::getInstance()->setCheating(aSource->getUser(), reason, reason, -1, false, true, false, true, false);
			QueueManager::getInstance()->putDownload(d, true);
			removeConnection(aSource);
			return;
		}

		QueueManager::getInstance()->putDownload(d, false);
	}

	removeConnection(aSource);
}

void DownloadManager::removeConnection(UserConnectionPtr aConn) {
	dcassert(aConn->getDownload() == NULL);
	aConn->removeListener(this);
	aConn->disconnect();
}

void DownloadManager::removeDownload(Download* d) {
	if(d->getFile()) {
		if(d->getActual() > 0) {
			try {
				d->getFile()->flush();
			} catch(const Exception&) {
			}
		}
	}

	{
		Lock l(cs);
		dcassert(find(downloads.begin(), downloads.end(), d) != downloads.end());

		downloads.erase(remove(downloads.begin(), downloads.end(), d), downloads.end());
	}
}

void DownloadManager::on(UserConnectionListener::ListLength, UserConnection* aSource, const string& aListLength) throw() {
	ClientManager::getInstance()->setListLength(aSource->getUser(), aListLength);
}


void DownloadManager::on(UserConnectionListener::FileNotAvailable, UserConnection* aSource) throw() {
	if(!aSource->getDownload()) {
		aSource->disconnect(true);
		return;
	}
	fileNotAvailable(aSource);
}

/** @todo Handle errors better */
void DownloadManager::on(AdcCommand::STA, UserConnection* aSource, const AdcCommand& cmd) throw() {
	if(cmd.getParameters().size() < 2) {
		aSource->disconnect();
		return;
	}

	const string& err = cmd.getParameters()[0];
	if(err.length() != 3) {
		aSource->disconnect();
		return;
	}

	switch(Util::toInt(err.substr(0, 1))) {
	case AdcCommand::SEV_FATAL:
		aSource->disconnect();
		return;
	case AdcCommand::SEV_RECOVERABLE:
		switch(Util::toInt(err.substr(1))) {
		case AdcCommand::ERROR_FILE_NOT_AVAILABLE:
			fileNotAvailable(aSource);
			return;
		case AdcCommand::ERROR_SLOTS_FULL:
			noSlots(aSource);
			return;
		}
	case AdcCommand::SEV_SUCCESS:
		// We don't know any messages that would give us these...
		dcdebug("Unknown success message %s %s", err.c_str(), cmd.getParam(1).c_str());
		return;
	}
	aSource->disconnect();
}

void DownloadManager::on(UserConnectionListener::Updated, UserConnection* aSource) throw() {
	{
		Lock l(cs);
		UserConnectionList::iterator i = find(idlers.begin(), idlers.end(), aSource);
		if(i == idlers.end())
			return;
		idlers.erase(i);
	}

	checkDownloads(aSource);
}
//r
void DownloadManager::fileNotAvailable(UserConnection* aSource) {
	if(aSource->getState() != UserConnection::STATE_SND) {
		dcdebug("DM::fileNotAvailable Invalid state, disconnecting");
		aSource->disconnect();
		return;
	}

	Download* d = aSource->getDownload();
	dcassert(d != NULL);
	dcdebug("File Not Available: %s\n", d->getPath().c_str());

	removeDownload(d);
//	fire(DownloadManagerListener::Failed(), d, str(F_("%1%: File not available") % d->getTargetFileName()));

	if (d->getType() == Transfer::TYPE_FULL_LIST) {
		fire(DownloadManagerListener::Failed(), d, "Check complete, idle");
		ClientManager::getInstance()->setCheating(aSource->getUser(), "", "Filelist Not Available", -1, true/*RSXPP_BOOLSETTING(SHOW_FILELIST_NA)*/, false, true, false, true);//todo SETTIBG ACT
		QueueManager::getInstance()->putDownload(d, true);
		removeConnection(aSource);
		return;
	} else if (d->isSet(Download::FLAG_TESTSUR) || aSource->isSet(UserConnection::FLAG_TESTSURNA)) {
		dcdebug("TestSUR File not available\n");
		fire(DownloadManagerListener::Failed(), d, "Check complete, idle");

		if(!aSource->isSet(UserConnection::FLAG_TESTSURNA))
			ClientManager::getInstance()->setCheating(aSource->getUser(), "File Not Available", "N/A", /*"!TESTSUR_FLNOAVLABLE"*/-1, false, false, false, true, false);//todo actions
		else
			ClientManager::getInstance()->setCheating(aSource->getUser(), "File Not Available", "", /*"!TESTSUR_FLNOAVLABLE"*/-1, false, false, false, true, false);//todo actions

		QueueManager::getInstance()->putDownload(d, true, false);
	//	removeConnection(aSource);
		checkDownloads(aSource);
		return;
	}


	fire(DownloadManagerListener::Failed(), d, string("File not available") + d->getTargetFileName());

	QueueManager::getInstance()->removeSource(d->getPath(), aSource->getUser(), d->getType() == Transfer::TYPE_TREE ? QueueItem::Source::FLAG_NO_TREE : QueueItem::Source::FLAG_FILE_NOT_AVAILABLE, false);

	QueueManager::getInstance()->putDownload(d, false);

	checkDownloads(aSource);
}
//r end
void DownloadManager::onFailed(UserConnection* aSource, const string& aError) {
	{
		Lock l(cs);
 		idlers.erase(remove(idlers.begin(), idlers.end(), aSource), idlers.end());
	}
	failDownload(aSource, aError);
}

} // namespace dcpp
