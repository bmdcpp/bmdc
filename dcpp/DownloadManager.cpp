/*
 * Copyright (C) 2001-2012 Jacek Sieka, arnetheduck on gmail point com
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
#include "DownloadManager.h"
#include "ClientManager.h"
#include "QueueManager.h"
#include "Download.h"
#include "LogManager.h"
#include "User.h"
#include "File.h"
#include "FilteredFile.h"
#include "MerkleCheckOutputStream.h"
#include "UserConnection.h"
#include "ZUtils.h"

#include <limits>
#include <cmath>

// some strange mac definition
#ifdef ff
#undef ff
#endif

namespace dcpp {

DownloadManager::DownloadManager() {
	TimerManager::getInstance()->addListener(this);
}

DownloadManager::~DownloadManager() {
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

void DownloadManager::on(TimerManagerListener::Second, uint64_t aTick) noexcept {
	typedef vector<pair<string, UserPtr> > TargetList;
	TargetList dropTargets;

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

		if(tickList.empty())
			fire(DownloadManagerListener::Tick(), tickList);

		// Automatically remove or disconnect slow sources
		if((uint32_t)(aTick / 1000) % SETTING(AUTODROP_INTERVAL) == 0) {
			for(auto i = downloads.begin(); i != downloads.end(); ++i) {
				Download* d = *i;
				uint64_t timeElapsed = aTick - d->getStart();
				uint64_t timeInactive = aTick - d->getUserConnection().getLastActivity();
				uint64_t bytesDownloaded = d->getPos();
				bool timeElapsedOk = timeElapsed >= (uint32_t)SETTING(AUTODROP_ELAPSED) * 1000;
				bool timeInactiveOk = timeInactive <= (uint32_t)SETTING(AUTODROP_INACTIVITY) * 1000;
				bool speedTooLow = timeElapsedOk && timeInactiveOk && bytesDownloaded > 0 ?
					bytesDownloaded / timeElapsed * 1000 < (uint32_t)SETTING(AUTODROP_SPEED) : false;
				bool isUserList = d->getType() == Transfer::TYPE_FULL_LIST;
				bool onlineSourcesOk = isUserList ?
					true : QueueManager::getInstance()->countOnlineSources(d->getPath()) >= SETTING(AUTODROP_MINSOURCES);
				bool filesizeOk = !isUserList && d->getSize() >= ((int64_t)SETTING(AUTODROP_FILESIZE)) * 1024;
				bool dropIt = (isUserList && SETTING(AUTODROP_FILELISTS)) ||
					(filesizeOk && SETTING(AUTODROP_ALL));
				if(speedTooLow && onlineSourcesOk && dropIt) {
					if(SETTING(AUTODROP_DISCONNECT) && isUserList) {
						d->getUserConnection().disconnect();
					} else {
						dropTargets.push_back(make_pair(d->getPath(), d->getUser()));
					}
				}
			}
		}
	}
	for(auto i = dropTargets.begin(); i != dropTargets.end(); ++i) {
		QueueManager::getInstance()->removeSource(i->first, i->second, QueueItem::Source::FLAG_SLOW_SOURCE);
	}
}

void DownloadManager::checkIdle(const HintedUser& user) {
	Lock l(cs);
	for(UserConnectionList::iterator i = idlers.begin(); i != idlers.end(); ++i) {
		UserConnection* uc = *i;
		if(uc->getUser() == user.user) {
			if(Util::stricmp(uc->getHubUrl(), user.hint) != 0) {
				uc->setHubUrl(user.hint);
				uc->callAsync([this, uc] { revive(uc); });
			return;
		}
	}
  }
}

void DownloadManager::revive(UserConnection* uc) {
	{
		Lock l(cs);
		auto i = find(idlers.begin(), idlers.end(), uc);
		if(i == idlers.end())
			return;
		idlers.erase(i);
	}

	checkDownloads(uc);
}

void DownloadManager::addConnection(UserConnectionPtr conn) {
	if(!conn->isSet(UserConnection::FLAG_SUPPORTS_TTHL) || !conn->isSet(UserConnection::FLAG_SUPPORTS_ADCGET)) {//BMDC TTHF->TTHL
		// Can't download from these...
		// No TTHL/ADCGET support///BMDC++ F->L
		ClientManager::getInstance()->setCheating(conn->getHintedUser(), "", "No TTHL/ADCGET support", 0,true , true, true, true, true);//TODO: add settings NO_TTHF
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

	/*
	find mySID, better ways to get the correct one transferred here?
	the hinturl of the connection is updated to the hub where the connection reguest is coming from,
	so we should be able to find our own SID by finding the hub where the user is at (if we have a hint).
	*/
	string mySID = Util::emptyString;
	if(!aConn->getUser()->isNMDC() && (d->getType() == Transfer::TYPE_FULL_LIST || d->getType() == Transfer::TYPE_PARTIAL_LIST))
		mySID = ClientManager::getInstance()->findMySID(aConn->getHintedUser());


	aConn->send(d->getCommand(aConn->isSet(UserConnection::FLAG_SUPPORTS_ZLIB_GET),mySID));
}

void DownloadManager::on(AdcCommand::SND, UserConnection* aSource, const AdcCommand& cmd) noexcept {
	if(aSource->getState() != UserConnection::STATE_SND) {
		dcdebug("DM::onFileLength Bad state, ignoring\n");
		return;
	}

	const string& type = cmd.getParam(0);
	int64_t start = Util::toInt64(cmd.getParam(2));
	int64_t bytes = Util::toInt64(cmd.getParam(3));

	//RSX++ // set filelist size
	const DownloadPtr download = aSource->getDownload();
	if(download && download->getPos() == 0 && download->isSet(Download::FLAG_CHECK_FILE_LIST)) {
		ClientManager::getInstance()->setListSize(aSource->getHintedUser(), bytes, true);
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

void DownloadManager::on(UserConnectionListener::Data, UserConnection* aSource, const uint8_t* aData, size_t aLen) noexcept {
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

		if(SETTING(LOG_DOWNLOADS) && (SETTING(LOG_FILELIST_TRANSFERS) || d->getType() == Transfer::TYPE_FILE) ) {
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
	ParamMap params;
	d->getParams(*aSource, params);
	LOG(LogManager::DOWNLOAD, params);
}

void DownloadManager::on(UserConnectionListener::MaxedOut, UserConnection* aSource, string params) noexcept {
	noSlots(aSource,params);
}

void DownloadManager::noSlots(UserConnection* aSource, string param) {
	if(aSource->getState() != UserConnection::STATE_SND) {
		dcdebug("DM::noSlots Bad state, disconnecting\n");
		aSource->disconnect();
		return;
	}

	string extra = param.empty() ? Util::emptyString : _(" - Queued: ") + param;
	dcdebug("DM:noSlots No slot aviable %s",extra.c_str());
	failDownload(aSource, _("No slots available") + extra);
}

void DownloadManager::onFailed(UserConnection* aSource, const string& aError) {
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
		/**/
		if (d->getType() == Transfer::TYPE_FULL_LIST && (reason.find(_("Disconnected")) != string::npos) ) {
			ClientManager::getInstance()->fileListDisconnected(aSource->getHintedUser());
		} else if( d->isSet(Download::FLAG_TESTSUR) ) {
			if(reason.find(_("No slots available")) != string::npos)
				ClientManager::getInstance()->setCheating(aSource->getHintedUser(), "MaxedOut", "No slots for TestSUR - SlotLocker", -1, true, true, false, true, false);
			else
				ClientManager::getInstance()->setCheating(aSource->getHintedUser(), reason, "", -1, false, true, false, true, false);
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

void DownloadManager::on(UserConnectionListener::FileNotAvailable, UserConnection* aSource) noexcept {
	fileNotAvailable(aSource);
}

/** @todo Handle errors better */
void DownloadManager::on(AdcCommand::STA, UserConnection* aSource, const AdcCommand& cmd) noexcept {
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
			string param;
			noSlots(aSource, cmd.getParam("QP", 0, param) ? param : Util::emptyString);
			return;
		}
	case AdcCommand::SEV_SUCCESS:
		// We don't know any messages that would give us these...
		dcdebug("Unknown success message %s %s", err.c_str(), cmd.getParam(1).c_str());
		return;
	}
	aSource->disconnect();
}

void DownloadManager::on(UserConnectionListener::Updated, UserConnection* aSource) noexcept {
	{
		Lock l(cs);
		UserConnectionList::iterator i = find(idlers.begin(), idlers.end(), aSource);
		if(i == idlers.end())
			return;
		idlers.erase(i);
	}

	checkDownloads(aSource);
}

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
	//BMDC//RSX++
	if (d->getType() == Transfer::TYPE_FULL_LIST) {
		fire(DownloadManagerListener::Failed(), d, "Check complete, idle");
		ClientManager::getInstance()->setCheating(aSource->getHintedUser(), "", "Filelist Not Available", SETTING(FILELIST_NA_RAW), SETTING(SHOW_FILELIST_NA), false, true, false, true);
		QueueManager::getInstance()->putDownload(d, true);
		removeConnection(aSource);
		return;
	} else if (d->isSet(Download::FLAG_TESTSUR)) {
		dcdebug("TestSUR File not available\n");
		fire(DownloadManagerListener::Failed(), d, "Check complete, idle");
		ClientManager::getInstance()->setCheating(aSource->getHintedUser(), "File Not Available", "", -1, false, false, false, true, false);
		QueueManager::getInstance()->putDownload(d, true, false);
		checkDownloads(aSource);
		return;
	}
	//end//
	fire(DownloadManagerListener::Failed(), d, str(F_("%1%: File not available") % d->getTargetFileName()));

	QueueManager::getInstance()->removeSource(d->getPath(), aSource->getUser(), d->getType() == Transfer::TYPE_TREE ? QueueItem::Source::FLAG_NO_TREE : QueueItem::Source::FLAG_FILE_NOT_AVAILABLE, false);

	QueueManager::getInstance()->putDownload(d, false);

	checkDownloads(aSource);
}

void DownloadManager::on(UserConnectionListener::ListLength, UserConnection* aSource, const string& aListLength) noexcept {
	ClientManager::getInstance()->setListLength(aSource->getHintedUser(), aListLength);
}

} // namespace dcpp
