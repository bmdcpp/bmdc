/*
 * Copyright (C) 2001-2017 Jacek Sieka, arnetheduck on gmail point com
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
#include "QueueManager.h"

#include "ClientManager.h"
#include "ConnectionManager.h"
#include "Download.h"
#include "FilteredFile.h"
#include "FinishedItem.h"
#include "FinishedManager.h"
#include "HashManager.h"
#include "LogManager.h"
#include "MerkleCheckOutputStream.h"
#include "SearchManager.h"
#include "SearchResult.h"
#include "SFVReader.h"
#include "ShareManager.h"
#include "SimpleXML.h"
#include "UserConnection.h"
#include "version.h"
#include "ZUtils.h"
#include "FileReader.h"

#if !defined(_WIN32) && !defined(PATH_MAX) // Extra PATH_MAX check for Mac OS X
	#include <sys/syslimits.h>
#endif

#ifdef ff
#undef ff
#endif


#include "UserManager.h"

namespace dcpp {

QueueManager::FileQueue::~FileQueue() {
	for_each(queue.begin(),queue.end() , DeleteFunction2());
}

QueueItem* QueueManager::FileQueue::add(const string& aTarget, int64_t aSize,
						  int aFlags, QueueItem::Priority p, const string& aTempTarget,
						  time_t aAdded, const TTHValue& root)
{
	if(p == QueueItem::DEFAULT) {
		p = QueueItem::NORMAL;
		if(aSize <= SETTING(PRIO_HIGHEST_SIZE)*1024) {
			p = QueueItem::HIGHEST;
		} else if(aSize <= SETTING(PRIO_HIGH_SIZE)*1024) {
			p = QueueItem::HIGH;
		} else if(aSize <= SETTING(PRIO_NORMAL_SIZE)*1024) {
			p = QueueItem::NORMAL;
		} else if(aSize <= SETTING(PRIO_LOW_SIZE)*1024) {
			p = QueueItem::LOW;
		} else if(SETTING(PRIO_LOWEST)) {
			p = QueueItem::LOWEST;
		}
	}

	QueueItem* qi = new QueueItem(aTarget, aSize, p, aFlags, aAdded, root);

	if(qi->isSet(QueueItem::FLAG_USER_LIST)) {
		qi->setPriority(QueueItem::HIGHEST);
	}

	qi->setTempTarget(aTempTarget);

	dcassert(find(aTarget) == NULL);
	add(qi);
	return qi;
}

void QueueManager::FileQueue::add(QueueItem* qi) {
	if(lastInsert == queue.end())
		lastInsert = queue.insert(make_pair(const_cast<string*>(&qi->getTarget()), qi)).first;
	else
		lastInsert = queue.insert(lastInsert, make_pair(const_cast<string*>(&qi->getTarget()), qi));
}

void QueueManager::FileQueue::remove(QueueItem* qi) {
	if(lastInsert != queue.end() && Util::stricmp(*lastInsert->first, qi->getTarget()) == 0)
		++lastInsert;
	queue.erase(const_cast<string*>(&qi->getTarget()));
	delete qi;
}

QueueItem* QueueManager::FileQueue::find(const string& target) {
	auto i = queue.find(const_cast<string*>(&target));
	return (i == queue.end()) ? NULL : i->second;
}

QueueManager::QueueItemList QueueManager::FileQueue::find(const TTHValue& tth) {
	QueueItemList ql;
	for(auto i = queue.begin(); i != queue.end(); ++i) {
		QueueItem* qi = i->second;
		if(qi->getTTH() == tth) {
			ql.push_back(qi);
		}
	}
	return ql;
}

static QueueItem* findCandidate(QueueItem* cand, QueueItem::StringMap::iterator start, QueueItem::StringMap::iterator end, const StringList& recent) {
	for(auto i = start; i != end; ++i) {
		QueueItem* q = i->second;

		// We prefer to search for things that are not running...
		if((cand != NULL) && q->isRunning())
			continue;
		// No finished files
		if(q->isFinished())
			continue;
		// No user lists
		if(q->isSet(QueueItem::FLAG_USER_LIST))
			continue;
		// No checking items..
		if(q->isSet(QueueItem::FLAG_TESTSUR) || q->isSet(QueueItem::FLAG_CHECK_FILE_LIST))
			continue;
		// No paused downloads
		if(q->getPriority() == QueueItem::PAUSED)
			continue;
		// No files that already have more than AUTO_SEARCH_LIMIT online sources
		if(q->countOnlineUsers() >= SETTING(AUTO_SEARCH_LIMIT))
			continue;
		// Did we search for it recently?
		if(find(recent.begin(), recent.end(), q->getTarget()) != recent.end())
			continue;

		cand = q;

		if(cand->isWaiting())
			break;
	}
	return cand;
}

QueueItem* QueueManager::FileQueue::findAutoSearch(StringList& recent) {
	// We pick a start position at random, hoping that we will find something to search for...
	QueueItem::StringMap::size_type start = (QueueItem::StringMap::size_type)Util::rand((uint32_t)queue.size());

	auto i = queue.begin();
	advance(i, start);

	QueueItem* cand = findCandidate(NULL, i, queue.end(), recent);
	if(cand == NULL || cand->isRunning()) {
		cand = findCandidate(cand, queue.begin(), i, recent);
	}
	return cand;
}

void QueueManager::FileQueue::move(QueueItem* qi, const string& aTarget) {
	if(lastInsert != queue.end() && Util::stricmp(*lastInsert->first, qi->getTarget()) == 0)
		lastInsert = queue.end();
	queue.erase(const_cast<string*>(&qi->getTarget()));
	qi->setTarget(aTarget);
	add(qi);
}

void QueueManager::UserQueue::add(QueueItem* qi) {
	for(auto i = qi->getSources().begin(); i != qi->getSources().end(); ++i) {
		add(qi, i->getUser());
	}
}

void QueueManager::UserQueue::add(QueueItem* qi, const UserPtr& aUser) {
	auto& l = userQueue[qi->getPriority()][aUser];

	if(qi->getDownloadedBytes() > 0) {
		l.push_front(qi);
	} else {
		l.push_back(qi);
	}
}

QueueItem* QueueManager::UserQueue::getNext(const UserPtr& aUser, QueueItem::Priority minPrio, int64_t wantedSize) {
	int p = QueueItem::LAST - 1;

	do {
		auto i = userQueue[p].find(aUser);
		if(i != userQueue[p].end())
		{
			dcassert(!i->second.empty());
			for(auto j = i->second.begin(); j != i->second.end(); ++j)
			{
				QueueItem* qi = *j;
				if(qi->isWaiting()) {
					return qi;
				}

				// No segmented downloading when getting the tree
				if(qi->getDownloads()[0]->getType() == Transfer::TYPE_TREE) {
					continue;
				}
				if(!qi->isSet(QueueItem::FLAG_USER_LIST)) {
					int64_t blockSize = HashManager::getInstance()->getBlockSize(qi->getTTH());
					if(blockSize == 0)
						blockSize = qi->getSize();
					if(qi->getNextSegment(blockSize, wantedSize).getSize() == 0) {
						dcdebug("No segment for %s in %s, block " I64_FMT "\n", aUser->getCID().toBase32().c_str(), qi->getTarget().c_str(),(long long int)blockSize);
						continue;
					}
				}
				if(qi->isSet(QueueItem::FLAG_TESTSUR) || qi->isSet(QueueItem::FLAG_CHECK_FILE_LIST))// TESTSUR and checkFL not need next BMDC++
					continue;
				return qi;
			}
		}
		p--;
	} while(p >= minPrio);

	return NULL;
}

void QueueManager::UserQueue::addDownload(QueueItem* qi, Download* d) {
	qi->getDownloads().push_back(d);

	// Only one download per user...
	dcassert(running.find(d->getUser()) == running.end());
	running[d->getUser()] = qi;
}

void QueueManager::UserQueue::removeDownload(QueueItem* qi, const UserPtr& user) {
	running.erase(user);

	for(auto i = qi->getDownloads().begin(); i != qi->getDownloads().end(); ++i) {
		if((*i)->getUser() == user) {
			qi->getDownloads().erase(i);
			break;
		}
	}
}

void QueueManager::UserQueue::setPriority(QueueItem* qi, QueueItem::Priority p) {
	remove(qi, false);
	qi->setPriority(p);
	add(qi);
}

pair<size_t, int64_t> QueueManager::UserQueue::getQueued(const UserPtr& aUser) const {
	pair<size_t, int64_t> ret(0, 0);
	for(size_t i = QueueItem::LOWEST; i < QueueItem::LAST; ++i) {
		auto& ulm = userQueue[i];
		auto iulm = ulm.find(aUser);

		if(iulm == ulm.end()) {
			continue;
		}

		for(auto j = iulm->second.begin(); j != iulm->second.end(); ++j) {
			auto &qi = *j;
			++ret.first;
			if(qi->getSize() != -1) {
				ret.second += qi->getSize() - qi->getDownloadedBytes();
			}
		}
	}
	return ret;
}

QueueItem* QueueManager::UserQueue::getRunning(const UserPtr& aUser) {
	auto i = running.find(aUser);
	return (i == running.end()) ? 0 : i->second;
}

void QueueManager::UserQueue::remove(QueueItem* qi, bool removeRunning) {
	for(auto i = qi->getSources().begin(); i != qi->getSources().end(); ++i) {
		remove(qi, i->getUser(), removeRunning);
	}
}

void QueueManager::UserQueue::remove(QueueItem* qi, const UserPtr& aUser, bool removeRunning) {
	if(removeRunning && qi == getRunning(aUser)) {
		removeDownload(qi, aUser);
	}

	dcassert(qi->isSource(aUser));
	auto& ulm = userQueue[qi->getPriority()];
	auto j = ulm.find(aUser);
	dcassert(j != ulm.end());
	auto& l = j->second;
	auto i = find(l.begin(), l.end(), qi);
	dcassert(i != l.end());
	l.erase(i);

	if(l.empty()) {
		ulm.erase(j);
	}
}

void QueueManager::FileMover::moveFile(const string& source, const string& target) {
	Lock l(cs);
	files.push_back(make_pair(source, target));
	if(!active) {
		active = true;
		start();
	}
}

int QueueManager::FileMover::run() {
	for(;;) {
		FilePair next;
		{
			Lock l(cs);
			if(files.empty()) {
				active = false;
				return 0;
			}
			next = files.back();
			files.pop_back();
		}
		moveFile_(next.first, next.second);
	}
}

void QueueManager::Rechecker::add(const string& file) {
	Lock l(cs);
	files.push_back(file);
	if(!active) {
		active = true;
		start();
	}
}

int QueueManager::Rechecker::run() {
	while(true) {
		string file;
		{
			Lock l(cs);
			auto i = files.begin();
			if(i == files.end()) {
				active = false;
				return 0;
			}
			file = *i;
			files.erase(i);
		}

		QueueItem* q;
		int64_t tempSize;
		TTHValue tth;

		{
			Lock l(qm->cs);

			q = qm->fileQueue.find(file);
			if(!q || q->isSet(QueueItem::FLAG_USER_LIST))
				continue;

			qm->fire(QueueManagerListener::RecheckStarted(), q->getTarget());
			dcdebug("Rechecking %s\n", file.c_str());

			tempSize = File::getSize(q->getTempTarget());

			if(tempSize == -1) {
				qm->fire(QueueManagerListener::RecheckNoFile(), q->getTarget());
				continue;
			}

			if(tempSize < 64*1024) {
				qm->fire(QueueManagerListener::RecheckFileTooSmall(), q->getTarget());
				continue;
			}

			if(tempSize != q->getSize()) {
				File(q->getTempTarget(), File::WRITE, File::OPEN).setSize(q->getSize());
			}

			if(q->isRunning()) {
				qm->fire(QueueManagerListener::RecheckDownloadsRunning(), q->getTarget());
				continue;
			}

			tth = q->getTTH();
		}

		TigerTree tt;
		bool gotTree = HashManager::getInstance()->getTree(tth, tt);

		string tempTarget;

		{
			Lock l(qm->cs);

			// get q again in case it has been (re)moved
			q = qm->fileQueue.find(file);
			if(!q)
				continue;

			if(!gotTree) {
				qm->fire(QueueManagerListener::RecheckNoTree(), q->getTarget());
				continue;
			}

			//Clear segments
			q->resetDownloaded();

			tempTarget = q->getTempTarget();
		}

		TigerTree ttFile(tt.getBlockSize());

		try {
			FileReader(true).read(tempTarget, [&](const void* x, size_t n) {
				return ttFile.update(x, n), true;
			});
		} catch(const FileException & e) {
			dcdebug("Error while reading file: %s\n", e.what());
		}

		Lock l(qm->cs);

		// get q again in case it has been (re)moved
		q = qm->fileQueue.find(file);
		if(!q)
			continue;

		ttFile.finalize();

		if(ttFile.getRoot() == tth) {
			//If no bad blocks then the file probably got stuck in the temp folder for some reason
			qm->moveStuckFile(q);
			continue;
		}

		size_t pos = 0;
		for(auto i = tt.getLeaves().begin();i!= tt.getLeaves().end();++i)
		{
			for(auto j = ttFile.getLeaves().begin();j!= ttFile.getLeaves().end();++j)
			{
				TTHValue& our = *i;
				TTHValue& file = *j;
				if(our == file)
				{
					q->addSegment(Segment(pos,tt.getBlockSize()));
				}
			pos += tt.getBlockSize();
			}

		}
		/*for_each(tt.getLeaves(), ttFile.getLeaves(), [&](const TTHValue& our, const TTHValue& file) {
			if(our == file) {
				q->addSegment(Segment(pos, tt.getBlockSize()));
			}

			pos += tt.getBlockSize();
		});*/

		qm->rechecked(q);
	}

	return 0;
}

QueueManager::QueueManager() :
lastSave(0),
queueFile(Util::getPath(Util::PATH_USER_CONFIG) + "Queue.xml"),
rechecker(this),
dirty(true),
nextSearch(0)
{
	TimerManager::getInstance()->addListener(this);
	SearchManager::getInstance()->addListener(this);
	//ClientManager::getInstance()->addListener(this);
    UsersManager::getInstance()->addListener(this);

	File::ensureDirectory(Util::getListPath());
}

QueueManager::~QueueManager() {
	SearchManager::getInstance()->removeListener(this);
	TimerManager::getInstance()->removeListener(this);
	//ClientManager::getInstance()->removeListener(this);
    UsersManager::getInstance()->removeListener(this);
	saveQueue();

	if(!SETTING(KEEP_LISTS)) {
		string path = Util::getListPath();

		std::sort(protectedFileLists.begin(), protectedFileLists.end());

		auto filelists = File::findFiles(path, "*.xml*");
		std::sort(filelists.begin(), filelists.end());
		std::for_each(filelists.begin(), std::set_difference(filelists.begin(), filelists.end(),
			protectedFileLists.begin(), protectedFileLists.end(), filelists.begin()), &File::deleteFile);
	}
}

bool QueueManager::getTTH(const string& name, TTHValue& tth) noexcept {
	Lock l(cs);
	QueueItem* qi = fileQueue.find(name);
	if(qi) {
		tth = qi->getTTH();
		return true;
	}
	return false;
}

void QueueManager::on(TimerManagerListener::Minute, uint64_t aTick) noexcept {
	string searchString;
	bool online = false;
	StringList offlineChecks; //BMDC/RSXlike

	{
		Lock l(cs);

		if(SETTING(AUTO_SEARCH) && (aTick >= nextSearch) && (fileQueue.getSize() > 0)) {
			// We keep 30 recent searches to avoid duplicate searches
			while((recent.size() >= fileQueue.getSize()) || (recent.size() > 30)) {
				recent.erase(recent.begin());
			}

			QueueItem* qi = fileQueue.findAutoSearch(recent);
			if(qi) {
				searchString = qi->getTTH().toBase32();
				online = qi->hasOnlineUsers();
				recent.push_back(qi->getTarget());
				nextSearch = aTick + (online ? 120000 : 300000);
			}
		}

		for(auto i = fileQueue.getQueue().begin(); i != fileQueue.getQueue().end(); ++i) {
			if(i->second->isSet(QueueItem::FLAG_TESTSUR) || i->second->isSet(QueueItem::FLAG_CHECK_FILE_LIST)) {
				if(i->second->countOnlineUsers() == 0) {
					offlineChecks.push_back(i->second->getTarget());
				}
			}
		}

	}

	for(auto i = offlineChecks.begin(); i != offlineChecks.end(); ++i) {
		try {
			remove(*i);
		} catch(...) {
			// exception
		}
	}

	if(!searchString.empty()) {
		SearchManager::getInstance()->search(searchString, 0, SearchManager::TYPE_TTH, SearchManager::SIZE_DONTCARE, "auto");
	}
}

void QueueManager::addList(const HintedUser& aUser, int aFlags, const string& aInitialDir /* = Util::emptyString */) {
	add(aInitialDir, -1, TTHValue(), aUser, QueueItem::FLAG_USER_LIST | aFlags);
}

string QueueManager::getListPath(const HintedUser& user) {
	StringList nicks = ClientManager::getInstance()->getNicks(user);
	string nick = nicks.empty() ? Util::emptyString : Util::cleanPathChars(nicks[0]) + ".";
	return checkTarget(Util::getListPath() + nick + user.user->getCID().toBase32(), /*checkExistence*/ false);
}

void QueueManager::add(const string& aTarget, int64_t aSize, const TTHValue& root, const HintedUser& aUser,
	int aFlags /* = 0 */, bool addBad /* = true */)
{
	auto gotoend = [this,&aUser] (bool wantConnection) -> void {
		if(wantConnection && aUser.user->isOnline())
		ConnectionManager::getInstance()->getDownloadConnection(aUser); };
	bool wantConnection = true;

	// Check that we're not downloading from ourselves...
	if(aUser == ClientManager::getInstance()->getMe()) {
		throw QueueException(_("You're trying to download from yourself!"));
	}

	// Check if we're not downloading something already in our share
	if(SETTING(DONT_DL_ALREADY_SHARED)){
		if (ShareManager::getInstance()->isTTHShared(root)){
			throw QueueException(_("A file with the same hash already exists in your share"));
		}
	}

	string target = Util::emptyString;
	string tempTarget = Util::emptyString;
	if( ((aFlags & QueueItem::FLAG_USER_LIST) == QueueItem::FLAG_USER_LIST) )   {
		target = getListPath(aUser);
		tempTarget = aTarget;
	} else {
		target = checkTarget(aTarget, /*checkExistence*/ true);
	}

	// Check if it's a zero-byte file, if so, create and return...
	if(aSize == 0) {
		if(!SETTING(SKIP_ZERO_BYTE)) {
			File::ensureDirectory(target);
			File f(target, File::WRITE, File::CREATE);
		}
		return;
	}

	{
		Lock l(cs);

		// This will be pretty slow on large queues...
		if(SETTING(DONT_DL_ALREADY_QUEUED) && (!(aFlags & QueueItem::FLAG_USER_LIST) || !(aFlags & QueueItem::FLAG_TESTSUR))) {//FL TESTSUR
			auto ql = fileQueue.find(root);
			if (!ql.empty()) {
				// Found one or more existing queue items, lets see if we can add the source to them
				// Check if any of the existing queue items are for permanent downloads; if so then no addition
				// If all existing queue items are for pending temporary downloads then add a new queue item or the source
				bool sourceAdded = false, permanentExists = false;
				for(auto i = ql.begin(); i != ql.end(); ++i) {
					if(!(*i)->isSource(aUser)) {
						try {
							wantConnection = addSource(*i, aUser, addBad ? QueueItem::Source::FLAG_MASK : 0);
							sourceAdded = true;
						} catch(...) { }
					}
					if (!(*i)->isSet(QueueItem::FLAG_CLIENT_VIEW)) {
						permanentExists = true;
					}

				}

				if(!sourceAdded && permanentExists) {
					throw QueueException(_("This file is already queued"));
				}
				if (permanentExists) { gotoend(wantConnection); return;}// connect;
			}
		}

		QueueItem* q = fileQueue.find(target);
		if(q == NULL && !((aFlags & QueueItem::FLAG_USER_LIST) || (aFlags & QueueItem::FLAG_TESTSUR))) {
			QueueItemList ql;
			ql = fileQueue.find(root);
			if(!ql.empty()){
				dcassert(ql.size() == 1);
				q = ql.front();
			}
		}

		if(q == NULL) {
			q = fileQueue.add(target, aSize, aFlags, QueueItem::DEFAULT, tempTarget, GET_TIME(), root);
			fire(QueueManagerListener::Added(), q);
		} else {
			if(q->getSize() != aSize) {
				throw QueueException(_("A file with a different size already exists in the queue"));
			}
			if(!(root == q->getTTH())) {
				throw QueueException(_("A file with a different TTH root already exists in the queue")+target);
			}

			if(q->isFinished()) {
				throw QueueException(_("This file has already finished downloading"));
			}

			q->setFlag(aFlags);
		}

		wantConnection = addSource(q, aUser, addBad ? QueueItem::Source::FLAG_MASK : 0);
	}
	gotoend(wantConnection);
}

void QueueManager::readd(const string& target, const HintedUser& aUser) {
	bool wantConnection = false;
	{
		Lock l(cs);
		QueueItem* q = fileQueue.find(target);
		if(q && q->isBadSource(aUser)) {
			wantConnection = addSource(q, aUser, QueueItem::Source::FLAG_MASK);
		}
	}
	if(wantConnection && aUser.user->isOnline())
		ConnectionManager::getInstance()->getDownloadConnection(aUser);
}

void QueueManager::setDirty() {
	if(!dirty) {
		dirty = true;
		lastSave = GET_TICK();
	}
}

string QueueManager::checkTarget(const string& aTarget, bool checkExistence) {
#ifdef _WIN32
	if(aTarget.length() > MAX_PATH) {
		throw QueueException(_("Target filename too long"));
	}
	// Check that target starts with a drive or is an UNC path
	if( (aTarget[1] != ':' || aTarget[2] != '\\') &&
		(aTarget[0] != '\\' && aTarget[1] != '\\') ) {
		throw QueueException(_("Invalid target file (missing directory, check default download directory setting)"));
	}
#else
	if(aTarget.length() > PATH_MAX) {
		throw QueueException(_("Target filename too long"));
	}
	// Check that target contains at least one directory...we don't want headless files...
	if(aTarget[0] != '/') {
		throw QueueException(_("Invalid target file (missing directory, check default download directory setting)"));
	}
#endif

	string target = Util::validateFileName(aTarget);

	// Check that the file doesn't already exist...
	if(checkExistence && File::getSize(target) != -1) {
		throw FileException(_("File already exists at the target location"));
	}
	return target;
}

/** Add a source to an existing queue item */
bool QueueManager::addSource(QueueItem* qi, const HintedUser& aUser, Flags::MaskType addBad) {
	bool wantConnection = (qi->getPriority() != QueueItem::PAUSED) && !userQueue.getRunning(aUser);

	if(qi->isSource(aUser)) {
		if(qi->isSet(QueueItem::FLAG_USER_LIST) || qi->isSet(QueueItem::FLAG_TESTSUR)) {
			return wantConnection;
		}
		throw QueueException(string(F_("Duplicate source: ") + Util::getFileName(qi->getTarget())));
	}

	if(qi->isBadSourceExcept(aUser, addBad)) {
		throw QueueException(string(F_("Duplicate source: ") + Util::getFileName(qi->getTarget())));
	}

	qi->addSource(aUser);

	if(aUser.user->isSet(User::PASSIVE) && !ClientManager::getInstance()->isActive() ) {
		qi->removeSource(aUser, QueueItem::Source::FLAG_PASSIVE);
		wantConnection = false;
	} else if(qi->isFinished()) {
		wantConnection = false;
	} else {
		userQueue.add(qi, aUser);
	}

	fire(QueueManagerListener::SourcesUpdated(), qi);
	setDirty();

	return wantConnection;
}

void QueueManager::addDirectory(const string& aDir, const HintedUser& aUser, const string& aTarget, QueueItem::Priority p /* = QueueItem::DEFAULT */) noexcept {
	bool needList;
	{
		Lock l(cs);

		auto dp = directories.equal_range(aUser);

		for(auto i = dp.first; i != dp.second; ++i) {
			if(Util::stricmp(aTarget.c_str(), i->second->getName().c_str()) == 0)
				return;
		}

		// Unique directory, fine...
		directories.insert(make_pair(aUser, new DirectoryItem(aUser, aDir, aTarget, p)));
		needList = (dp.first == dp.second);
		setDirty();
	}

	if(needList) {
		try {
			addList(aUser, QueueItem::FLAG_DIRECTORY_DOWNLOAD);
		} catch(const Exception&) {
			// Ignore, we don't really care...
		}
	}
}

QueueItem::Priority QueueManager::hasDownload(const UserPtr& aUser) noexcept {
	Lock l(cs);
	QueueItem* qi = userQueue.getNext(aUser, QueueItem::LOWEST);
	if(!qi) {
		return QueueItem::PAUSED;
	}
	return qi->getPriority();
}
namespace {
typedef unordered_map<TTHValue, const DirectoryListing::File*> TTHMap;

// *** WARNING ***
// Lock(cs) makes sure that there's only one thread accessing this
static TTHMap tthMap;

void buildMap(const DirectoryListing::Directory* dir) noexcept {
	for(auto j = dir->directories.begin(); j != dir->directories.end(); ++j) {
		if(!(*j)->getAdls())
			buildMap(*j);
	}

	for(auto i = dir->files.begin(); i != dir->files.end(); ++i) {
		const DirectoryListing::File* df = *i;
		tthMap.insert(make_pair(df->getTTH(), df));
	}
}
}

int QueueManager::matchListing(const DirectoryListing& dl) noexcept {
	int matches = 0;
	{
		Lock l(cs);
		tthMap.clear();
		buildMap(dl.getRoot());

		for(QueueItem::StringMap::const_iterator i = fileQueue.getQueue().begin(); i != fileQueue.getQueue().end(); ++i) {
			QueueItem* qi = i->second;
			if(qi->isFinished())
				continue;
			if(qi->isSet(QueueItem::FLAG_USER_LIST))
				continue;
			TTHMap::iterator j = tthMap.find(qi->getTTH());
			if(j != tthMap.end() && i->second->getSize() == qi->getSize()) {
				try {
					addSource(qi, dl.getUser(), QueueItem::Source::FLAG_FILE_NOT_AVAILABLE);
				} catch(...) {
					// Ignore...
				}
				matches++;
			}
		}
	}
	if(matches > 0)
		ConnectionManager::getInstance()->getDownloadConnection(dl.getUser());
	return matches;
}

int64_t QueueManager::getPos(const string& target) noexcept {
	Lock l(cs);
	QueueItem* qi = fileQueue.find(target);
	if(qi) {
		return qi->getDownloadedBytes();
	}
	return -1;
}

int64_t QueueManager::getSize(const string& target) noexcept {
	Lock l(cs);
	QueueItem* qi = fileQueue.find(target);
	if(qi) {
		return qi->getSize();
	}
	return -1;
}

void QueueManager::getSizeInfo(int64_t& size, int64_t& pos, const string& target) noexcept {
	Lock l(cs);
	QueueItem* qi = fileQueue.find(target);
	if(qi) {
		size = qi->getSize();
		pos = qi->getDownloadedBytes();
	} else {
		size = -1;
	}
}


void QueueManager::move(const string& aSource, const string& aTarget) noexcept {
	string target = Util::validateFileName(aTarget);
	if(aSource == target)
		return;

	bool delSource = false;

	Lock l(cs);
	QueueItem* qs = fileQueue.find(aSource);
	if(qs) {
		// Don't move running downloads
		if(qs->isRunning()) {
			return;
		}
		// Don't move file lists
		if(qs->isSet(QueueItem::FLAG_USER_LIST))
			return;

		// Let's see if the target exists...then things get complicated...
		QueueItem* qt = fileQueue.find(target);
		if(qt == NULL || Util::stricmp(aSource, target) == 0) {
			// Good, update the target and move in the queue...
			fileQueue.move(qs, target);
			fire(QueueManagerListener::Moved(), qs, aSource);
			setDirty();
		} else {
			// Don't move to target of different size
			if(qs->getSize() != qt->getSize() || qs->getTTH() != qt->getTTH())
				return;

			for(QueueItem::SourceConstIter i = qs->getSources().begin(); i != qs->getSources().end(); ++i) {
				try {
					addSource(qt, i->getUser(), QueueItem::Source::FLAG_MASK);
				} catch(const Exception&) {
				}
			}
			delSource = true;
		}
	}

	if(delSource) {
		remove(aSource);
	}
}

StringList QueueManager::getTargets(const TTHValue& tth) {
	Lock l(cs);
	auto ql = fileQueue.find(tth);
	StringList sl;
	for(auto i = ql.begin(); i != ql.end(); ++i) {
		sl.push_back((*i)->getTarget());
	}
	return sl;
}

Download* QueueManager::getDownload(UserConnection& aSource, bool supportsTrees) noexcept {
	Lock l(cs);

	UserPtr& u = aSource.getUser();
	dcdebug("Getting download for %s...", u->getCID().toBase32().c_str());

	QueueItem* q = userQueue.getNext(u, QueueItem::LOWEST, aSource.getChunkSize());

	if(!q) {
		dcdebug("none\n");
		return 0;
	}

	// Check that the file we will be downloading to exists
	if(q->getDownloadedBytes() > 0) {
		int64_t tempSize = File::getSize(q->getTempTarget());
		if(tempSize != q->getSize()) {
			// <= 0.706 added ".antifrag" to temporary download files if antifrag was enabled...
			// 0.705 added ".antifrag" even if antifrag was disabled
			std::string antifrag = q->getTempTarget() + ".antifrag";
			if(File::getSize(antifrag) > 0) {
				File::renameFile(antifrag, q->getTempTarget());
				tempSize = File::getSize(q->getTempTarget());
			}
			if(tempSize != q->getSize()) {
				if(tempSize > 0 && tempSize < q->getSize()) {
					// Probably started with <=0.699 or with 0.705 without antifrag enabled...
					try {
						File(q->getTempTarget(), File::WRITE, File::OPEN).setSize(q->getSize());
					} catch(const FileException&) { }
				} else {
					// Temp target gone?
					q->resetDownloaded();
				}
			}
		}
	}

	Download* d = new Download(aSource, *q, q->getTarget(), supportsTrees);

	userQueue.addDownload(q, d);

	fire(QueueManagerListener::StatusUpdated(), q);
	dcdebug("found %s\n", q->getTarget().c_str());
	return d;
}

namespace {
class TreeOutputStream : public OutputStream {
public:
	TreeOutputStream(TigerTree& aTree) : tree(aTree), bufPos(0) {
	}

	virtual size_t write(const void* xbuf, size_t len) {
		size_t pos = 0;
		uint8_t* b = (uint8_t*)xbuf;
		while(pos < len) {
			size_t left = len - pos;
			if(bufPos == 0 && left >= TigerTree::BYTES) {
				tree.getLeaves().push_back(TTHValue(b + pos));
				pos += TigerTree::BYTES;
			} else {
				size_t bytes = min(TigerTree::BYTES - bufPos, left);
				memcpy(buf + bufPos, b + pos, bytes);
				bufPos += bytes;
				pos += bytes;
				if(bufPos == TigerTree::BYTES) {
					tree.getLeaves().push_back(TTHValue(buf));
					bufPos = 0;
				}
			}
		}
		return len;
	}

	virtual size_t flush() {
		return 0;
	}
private:
	TigerTree& tree;
	uint8_t buf[TigerTree::BYTES];
	size_t bufPos;
};

}

void QueueManager::setFile(Download* d) {
	if(d->getType() == Transfer::TYPE_FILE) {
		Lock l(cs);

		QueueItem* qi = fileQueue.find(d->getPath());
		if(!qi) {
			throw QueueException(_("Target removed"));
		}

		string target = d->getDownloadTarget();

		if(d->getSegment().getStart() > 0) {
			if(File::getSize(target) != qi->getSize()) {
				// When trying the download the next time, the resume pos will be reset
				throw QueueException(_("Target file is missing or wrong size"));
			}
		} else {
			File::ensureDirectory(target);
		}

		File* f = new File(target, File::WRITE, File::OPEN | File::CREATE | File::SHARED);

		if(f->getSize() != qi->getSize()) {
			f->setSize(qi->getSize());
		}

		f->setPos(d->getSegment().getStart());
		d->setFile(f);
	} else if(d->getType() == Transfer::TYPE_FULL_LIST) {
		string target = d->getPath();
		File::ensureDirectory(target);

		if(d->isSet(Download::FLAG_XML_BZ_LIST)) {
			target += ".xml.bz2";
		} else {
			target += ".xml";
		}
		d->setFile(new File(target, File::WRITE, File::OPEN | File::TRUNCATE | File::CREATE));
	} else if(d->getType() == Transfer::TYPE_PARTIAL_LIST) {
		d->setFile(new StringRefOutputStream(d->getPFS()));
	} else if(d->getType() == Transfer::TYPE_TREE) {
		d->setFile(new TreeOutputStream(d->getTigerTree()));
	}
}

void QueueManager::moveFile(const string& source, const string& target) {
	File::ensureDirectory(target);
	if(File::getSize(source) > MOVER_LIMIT) {
		mover.moveFile(source, target);
	} else {
		moveFile_(source, target);
	}
}

void QueueManager::moveFile_(const string& source, const string& target) {
	try {
		File::renameFile(source, target);
		getInstance()->fire(QueueManagerListener::FileMoved(), target);
	} catch(const FileException& e1) {
		// Try to just rename it to the correct name at least
		string newTarget = Util::getFilePath(source) + Util::getFileName(target);
		try {
			File::renameFile(source, newTarget);
			LogManager::getInstance()->message( _( ("Unable to move "+Util::addBrackets(source)+" to "+Util::addBrackets(target)+" ("+e1.getError()+") renamed to "+ Util::addBrackets(newTarget)).c_str()),LogManager::Sev::HIGH);
		} catch(const FileException& e2) {
			LogManager::getInstance()->message( _( ("Unable to move "+Util::addBrackets(source)+" to "+Util::addBrackets(target)+" ("+e1.getError()+") nor to rename to "+Util::addBrackets(newTarget)+" ("+e2.getError()+")").c_str()   ),LogManager::Sev::HIGH);
		}
	}
}

void QueueManager::moveStuckFile(QueueItem* qi) {
	moveFile(qi->getTempTarget(), qi->getTarget());

	if(qi->isFinished()) {
		userQueue.remove(qi);
	}

	string target = qi->getTarget();

	if(!SETTING(KEEP_FINISHED_FILES)) {
		fire(QueueManagerListener::Removed(), qi);
		fileQueue.remove(qi);
	 } else {
		qi->addSegment(Segment(0, qi->getSize()));
		fire(QueueManagerListener::StatusUpdated(), qi);
	}

	fire(QueueManagerListener::RecheckAlreadyFinished(), target);
}

void QueueManager::rechecked(QueueItem* qi) {
	fire(QueueManagerListener::RecheckDone(), qi->getTarget());
	fire(QueueManagerListener::StatusUpdated(), qi);

	setDirty();
}

void QueueManager::putDownload(Download* aDownload, bool finished, bool reportFinish /*= true*/) noexcept {
	HintedUserList getConn;
 	string fl_fname;
	HintedUser fl_user(UserPtr(), Util::emptyString);
	int fl_flag = 0;

	{
		Lock l(cs);

		delete aDownload->getFile();
		aDownload->setFile(nullptr);

		if(aDownload->getType() == Transfer::TYPE_PARTIAL_LIST ) {
			QueueItem* q = fileQueue.find(/*getListPath(aDownload->getHintedUser())*/aDownload->getPath());
			if(q) {
				if(finished) {
					fire(QueueManagerListener::PartialList(), aDownload->getHintedUser(), aDownload->getPFS());
					fire(QueueManagerListener::Removed(), q);

					userQueue.remove(q);
					fileQueue.remove(q);
				} else {
					userQueue.removeDownload(q, aDownload->getUser());
					fire(QueueManagerListener::StatusUpdated(), q);
				}
			}
		} else {
			QueueItem* q = fileQueue.find(aDownload->getPath());

			if(q) {
				if(aDownload->getType() == Transfer::TYPE_FULL_LIST) {
					if(aDownload->isSet(Download::FLAG_XML_BZ_LIST)) {
						q->setFlag(QueueItem::FLAG_XML_BZLIST);
					} else {
						q->unsetFlag(QueueItem::FLAG_XML_BZLIST);
					}
				}

				if(finished) {
					if(aDownload->getType() == Transfer::TYPE_TREE) {
						// Got a full tree, now add it to the HashManager
						dcassert(aDownload->getTreeValid());
						HashManager::getInstance()->addTree(aDownload->getTigerTree());

						userQueue.removeDownload(q, aDownload->getUser());
						fire(QueueManagerListener::StatusUpdated(), q);
					} else {
						// Now, let's see if this was a directory download filelist...
						if( (q->isSet(QueueItem::FLAG_DIRECTORY_DOWNLOAD) && directories.find(aDownload->getUser()) != directories.end()) ||
							(q->isSet(QueueItem::FLAG_MATCH_QUEUE)) )
						{
							fl_fname = q->getListName();
							fl_user = aDownload->getHintedUser();
							fl_flag = (q->isSet(QueueItem::FLAG_DIRECTORY_DOWNLOAD) ? QueueItem::FLAG_DIRECTORY_DOWNLOAD : 0)
								| (q->isSet(QueueItem::FLAG_MATCH_QUEUE) ? QueueItem::FLAG_MATCH_QUEUE : 0);
						}

						string dir;
						bool crcError = false;
						if(aDownload->getType() == Transfer::TYPE_FULL_LIST) {
							dir = q->getTempTarget();
							q->addSegment(Segment(0, q->getSize()));
						} else if(aDownload->getType() == Transfer::TYPE_FILE) {
							q->addSegment(aDownload->getSegment());

							if (q->isFinished() && SETTING(SFV_CHECK)) {
								crcError = checkSfv(q, aDownload);
							}
						}

						if(aDownload->getType() != Transfer::TYPE_FILE || q->isFinished()) {
							// Check if we need to move the file
							if( (aDownload->getType() == Transfer::TYPE_FILE) && !aDownload->getTempTarget().empty() && (Util::stricmp(aDownload->getPath().c_str(), aDownload->getTempTarget().c_str()) != 0 ))   {
								moveFile(aDownload->getTempTarget(), aDownload->getPath());
							}

							if (SETTING(LOG_FINISHED_DOWNLOADS) && aDownload->getType() == Transfer::TYPE_FILE) {
								logFinishedDownload(q, aDownload, crcError);
							}

							fire(QueueManagerListener::Finished(), q, dir, aDownload->getAverageSpeed());

							userQueue.remove(q);

							if(!SETTING(KEEP_FINISHED_FILES) || aDownload->getType() == Transfer::TYPE_FULL_LIST) {
								fire(QueueManagerListener::Removed(), q);
								fileQueue.remove(q);
							} else {
								fire(QueueManagerListener::StatusUpdated(), q);
							}
						} else {
							userQueue.removeDownload(q, aDownload->getUser());
							if(aDownload->getType() != Transfer::TYPE_FILE || (reportFinish && q->isWaiting())) {
								fire(QueueManagerListener::StatusUpdated(), q);
							}
						}
						setDirty();
					}
				} else {
					if(aDownload->getType() != Transfer::TYPE_TREE) {
						if(q->getDownloadedBytes() == 0) {
							q->setTempTarget(Util::emptyString);
						}
						if(q->isSet(QueueItem::FLAG_USER_LIST)) {
							// Blah...no use keeping an unfinished file list...
							File::deleteFile(q->getListName());
						}

						if(aDownload->getType() == Transfer::TYPE_FILE) {
							// mark partially downloaded chunk, but align it to block size
							int64_t downloaded = aDownload->getPos();
							downloaded -= downloaded % aDownload->getTigerTree().getBlockSize();

							if(downloaded > 0) {
								q->addSegment(Segment(aDownload->getStartPos(), downloaded));
								setDirty();
							}
						}
					}

					if(q->getPriority() != QueueItem::PAUSED) {
						q->getOnlineUsers(getConn);
					}

					userQueue.removeDownload(q, aDownload->getUser());
					fire(QueueManagerListener::StatusUpdated(), q);
				}
			} else if(aDownload->getType() != Transfer::TYPE_TREE) {
				if(!aDownload->getTempTarget().empty() && (aDownload->getType() == Transfer::TYPE_FULL_LIST || aDownload->getTempTarget() != aDownload->getPath())) {
					File::deleteFile(aDownload->getTempTarget());
				}
			}

			setDirty();
		}
		delete aDownload;
	}

	for(HintedUserList::iterator i = getConn.begin(); i != getConn.end(); ++i) {
		ConnectionManager::getInstance()->getDownloadConnection(*i);
	}

	if(!fl_fname.empty()) {
		processList(fl_fname, fl_user, fl_flag);
	}
}

void QueueManager::processList(const string& name, const HintedUser& user, int flags) {
	DirectoryListing dirList(user);
	try {
		dirList.loadFile(name);
	} catch(const Exception&) {
		LogManager::getInstance()->message(string(F_("Unable to open filelist: ") + Util::addBrackets(name)),LogManager::Sev::HIGH);
		return;
	}

	if(flags & QueueItem::FLAG_DIRECTORY_DOWNLOAD) {
		DirectoryItem::List dl;
		{
			Lock l(cs);
			auto dp = directories.equal_range(user); //| map_values;
			for(auto i = dp.first;i!= dp.second;++i) {dl.push_back((*i).second);}//TODO check...
			directories.erase(user);
		}

		for(DirectoryItem::Iter i = dl.begin(); i != dl.end(); ++i) {
			DirectoryItem* di = *i;
			dirList.download(di->getName(), di->getTarget(), false);
			delete di;
		}
	}
	if(flags & QueueItem::FLAG_MATCH_QUEUE) {
		size_t files = matchListing(dirList);
		LogManager::getInstance()->message(string(F_(Util::toString(ClientManager::getInstance()->getNicks(user))+": Matched "+Util::toString(files)+" file(s)")));
	}
}

void QueueManager::recheck(const string& aTarget) {
	rechecker.add(aTarget);
}

void QueueManager::remove(const string& aTarget) noexcept {
	UserList x;
	{
		Lock l(cs);

		QueueItem* q = fileQueue.find(aTarget);
		if(!q)
			return;

		if(q->isSet(QueueItem::FLAG_DIRECTORY_DOWNLOAD)) {
			dcassert(q->getSources().size() == 1);
			DirectoryItem::DirectoryPair dp = directories.equal_range(q->getSources()[0].getUser());
			for(DirectoryItem::DirectoryIter i = dp.first; i != dp.second; ++i) {
				delete i->second;
			}
			directories.erase(q->getSources()[0].getUser());
		}

		if(q->isRunning()) {
			for(DownloadList::iterator i = q->getDownloads().begin(); i != q->getDownloads().end(); ++i) {
				x.push_back((*i)->getUser());
			}
		} else if(!q->getTempTarget().empty() && q->getTempTarget() != q->getTarget()) {
			File::deleteFile(q->getTempTarget());
		}

		fire(QueueManagerListener::Removed(), q);

		if(!q->isFinished()) {
			userQueue.remove(q);
		}
		fileQueue.remove(q);

		setDirty();
	}

	for(UserList::iterator i = x.begin(); i != x.end(); ++i) {
		ConnectionManager::getInstance()->disconnect(*i, true);
	}
}

#define MAX_SIZE_WO_TREE 20*1024*1024

void QueueManager::removeSource(const string& aTarget, const UserPtr& aUser, int reason, bool removeConn /* = true */) noexcept {
	bool isRunning = false;
	bool removeCompletely = false;
	{
		Lock l(cs);
		QueueItem* q = fileQueue.find(aTarget);
		if(!q)
			return;

		if(!q->isSource(aUser))
			return;

		if(q->isSet(QueueItem::FLAG_USER_LIST)) {
			removeCompletely = true;
			goto endCheck;
		}

		if(reason == QueueItem::Source::FLAG_NO_TREE) {
			q->getSource(aUser)->setFlag(reason);
			if (q->getSize() < MAX_SIZE_WO_TREE) {
				return;
			}
		}

		if(q->isRunning() && userQueue.getRunning(aUser) == q) {
			isRunning = true;
			userQueue.removeDownload(q, aUser);
			fire(QueueManagerListener::StatusUpdated(), q);
		}

		if(!q->isFinished()) {
			userQueue.remove(q, aUser);
		}
		q->removeSource(aUser, reason);

		fire(QueueManagerListener::SourcesUpdated(), q);
		setDirty();
	}
endCheck:
	if(isRunning && removeConn) {
		ConnectionManager::getInstance()->disconnect(aUser, true);
	}
	if(removeCompletely) {
		remove(aTarget);
	}
}

pair<size_t, int64_t> QueueManager::getQueued(const UserPtr& aUser) const {
	Lock l(cs);
	return userQueue.getQueued(aUser);
}

void QueueManager::removeSource(const UserPtr& aUser, int reason) noexcept {
	// @todo remove from finished items
	bool isRunning = false;
	string removeRunning;
	{
		Lock l(cs);
		QueueItem* qi = NULL;
		while( (qi = userQueue.getNext(aUser, QueueItem::PAUSED)) != NULL) {
			if(qi->isSet(QueueItem::FLAG_USER_LIST)) {
				remove(qi->getTarget());
			} else {
				userQueue.remove(qi, aUser);
				qi->removeSource(aUser, reason);
				fire(QueueManagerListener::SourcesUpdated(), qi);
				setDirty();
			}
		}

		qi = userQueue.getRunning(aUser);
		if(qi) {
			if(qi->isSet(QueueItem::FLAG_USER_LIST)) {
				removeRunning = qi->getTarget();
			} else {
				userQueue.removeDownload(qi, aUser);
				userQueue.remove(qi, aUser);
				isRunning = true;
				qi->removeSource(aUser, reason);
				fire(QueueManagerListener::StatusUpdated(), qi);
				fire(QueueManagerListener::SourcesUpdated(), qi);
				setDirty();
			}
		}
	}

	if(isRunning) {
		ConnectionManager::getInstance()->disconnect(aUser, true);
	}
	if(!removeRunning.empty()) {
		remove(removeRunning);
	}
}

void QueueManager::setPriority(const string& aTarget, QueueItem::Priority p) noexcept {
	HintedUserList getConn;

	{
		Lock l(cs);

		QueueItem* q = fileQueue.find(aTarget);
		if( (q != NULL) && (q->getPriority() != p) && !q->isFinished() ) {
			if(q->getPriority() == QueueItem::PAUSED || p == QueueItem::HIGHEST) {
				// Problem, we have to request connections to all these users...
				q->getOnlineUsers(getConn);
			}
			userQueue.setPriority(q, p);
			setDirty();
			fire(QueueManagerListener::StatusUpdated(), q);
		}
	}

	for(HintedUserList::iterator i = getConn.begin(); i != getConn.end(); ++i) {
		ConnectionManager::getInstance()->getDownloadConnection(*i);
	}
}

void QueueManager::saveQueue(bool force) noexcept {
	if(!dirty && !force)
		return;

	std::vector<CID> cids;

	try {
		Lock l(cs);

		File ff(getQueueFile() + ".tmp", File::WRITE, File::CREATE | File::TRUNCATE);
		BufferedOutputStream<false> f(&ff);

		f.write(SimpleXML::utf8Header);
		f.write(LIT("<Downloads Version=\"" VERSIONSTRING "\">\r\n"));
		string tmp;
		string b32tmp;
		for(auto i = fileQueue.getQueue().begin(); i != fileQueue.getQueue().end(); ++i) {
			QueueItem* qi = i->second;
			if(!qi->isSet(QueueItem::FLAG_USER_LIST) && !qi->isSet(QueueItem::FLAG_TESTSUR)) {
				f.write(LIT("\t<Download Target=\""));
				f.write(SimpleXML::escape(qi->getTarget(), tmp, true));
				f.write(LIT("\" Size=\""));
				f.write(Util::toString(qi->getSize()));
				f.write(LIT("\" Priority=\""));
				f.write(Util::toString((int)qi->getPriority()));
				f.write(LIT("\" Added=\""));
				f.write(Util::toString(qi->getAdded()));
				b32tmp.clear();
				f.write(LIT("\" TTH=\""));
				f.write(qi->getTTH().toBase32(b32tmp));
				if(!qi->getDone().empty()) {
					f.write(LIT("\" TempTarget=\""));
					f.write(SimpleXML::escape(qi->getTempTarget(), tmp, true));
				}
				f.write(LIT("\">\r\n"));

				for(QueueItem::SegmentSet::const_iterator i = qi->getDone().begin(); i != qi->getDone().end(); ++i) {
					f.write(LIT("\t\t<Segment Start=\""));
					f.write(Util::toString(i->getStart()));
					f.write(LIT("\" Size=\""));
					f.write(Util::toString(i->getSize()));
					f.write(LIT("\"/>\r\n"));
				}

				for(QueueItem::SourceConstIter j = qi->sources.begin(); j != qi->sources.end(); ++j) {
					const CID& cid = j->getUser().user->getCID();
					const string& hint = j->getUser().hint;

					f.write(LIT("\t\t<Source CID=\""));
					f.write(cid.toBase32());
					if(!hint.empty()) {
						f.write(LIT("\" Hub=\""));
						f.write(hint);
					}
					f.write(LIT("\"/>\r\n"));

					cids.push_back(cid);
				}

				f.write(LIT("\t</Download>\r\n"));
			}
		}

		f.write("</Downloads>\r\n");
		f.flush();
		ff.close();
		File::deleteFile(getQueueFile());
		File::renameFile(getQueueFile() + ".tmp", getQueueFile());

		dirty = false;
	} catch(const FileException&) {
		// ...
	}
	// Put this here to avoid very many saves tries when disk is full...
	lastSave = GET_TICK();
}

class QueueLoader : public SimpleXMLReader::CallBack {
public:
	QueueLoader() : cur(NULL), inDownloads(false) { }
	virtual ~QueueLoader() { }
	virtual void startTag(const string& name, StringPairList& attribs, bool simple);
	virtual void endTag(const string& name);
private:
	string target;

	QueueItem* cur;
	bool inDownloads;
};

void QueueManager::loadQueue() noexcept {
	try {
		QueueLoader l;
		Util::migrate(getQueueFile());

		File f(getQueueFile(), File::READ, File::OPEN);
		SimpleXMLReader(&l).parse(f);
		dirty = false;
	} catch(const Exception&) {
		// ...
	}
}

int QueueManager::countOnlineSources(const string& aTarget) {
	Lock l(cs);

	QueueItem* qi = fileQueue.find(aTarget);
	if(!qi)
		return 0;
	int onlineSources = 0;
	for(QueueItem::SourceConstIter i = qi->getSources().begin(); i != qi->getSources().end(); ++i) {
		if(i->getUser().user->isOnline())
			onlineSources++;
	}
	return onlineSources;
}

static const string sDownloads = "Downloads";
static const string sDownload = "Download";
static const string sTempTarget = "TempTarget";
static const string sTarget = "Target";
static const string sSize = "Size";
static const string sDownloaded = "Downloaded";
static const string sPriority = "Priority";
static const string sSource = "Source";
static const string sNick = "Nick";
static const string sDirectory = "Directory";
static const string sAdded = "Added";
static const string sTTH = "TTH";
static const string sCID = "CID";
static const string sHubHint = "Hub";
static const string sSegment = "Segment";
static const string sStart = "Start";

void QueueLoader::startTag(const string& name, StringPairList& attribs, bool simple) {
	QueueManager* qm = QueueManager::getInstance();
	if(!inDownloads && name == sDownloads) {
		inDownloads = true;
	} else if(inDownloads) {
		if(cur == NULL && name == sDownload) {
			int64_t size = Util::toInt64(getAttrib(attribs, sSize, 1));
			if(size == 0)
				return;
			try {
				const string& tgt = getAttrib(attribs, sTarget, 0);
				// @todo do something better about existing files
				target = QueueManager::checkTarget(tgt,  /*checkExistence*/ false);
				if(target.empty())
					return;
			} catch(const Exception&) {
				return;
			}
			QueueItem::Priority p = (QueueItem::Priority)Util::toInt(getAttrib(attribs, sPriority, 3));
			time_t added = static_cast<time_t>(Util::toInt(getAttrib(attribs, sAdded, 4)));
			const string& tthRoot = getAttrib(attribs, sTTH, 5);
			if(tthRoot.empty())
				return;

			string tempTarget = getAttrib(attribs, sTempTarget, 5);
			int64_t downloaded = Util::toInt64(getAttrib(attribs, sDownloaded, 5));
			if (downloaded > size || downloaded < 0)
				downloaded = 0;

			if(added == 0)
				added = GET_TIME();

			QueueItem* qi = qm->fileQueue.find(target);

			if(qi == NULL) {
				qi = qm->fileQueue.add(target, size, 0, p, tempTarget, added, TTHValue(tthRoot));
				if(downloaded > 0) {
					qi->addSegment(Segment(0, downloaded));
				}
				qm->fire(QueueManagerListener::Added(), qi);
			}
			if(!simple)
				cur = qi;
		} else if(cur && name == sSegment) {
			int64_t start = Util::toInt64(getAttrib(attribs, sStart, 0));
			int64_t size = Util::toInt64(getAttrib(attribs, sSize, 1));

			if(size > 0 && start >= 0 && (start + size) <= cur->getSize()) {
				cur->addSegment(Segment(start, size));
			}
		} else if(cur && name == sSource) {
			const string& cid = getAttrib(attribs, sCID, 0);
			if(cid.length() != 39) {
				// Skip loading this source - sorry old users
				return;
			}
			UserPtr user = ClientManager::getInstance()->getUser(CID(cid));

			try {
				const string& hubHint = getAttrib(attribs, sHubHint, 1);
				HintedUser hintedUser(user, hubHint);
				if(qm->addSource(cur, hintedUser, 0) && user->isOnline())
					ConnectionManager::getInstance()->getDownloadConnection(hintedUser);
			} catch(const Exception&) {
				return;
			}
		}
	}
}

void QueueLoader::endTag(const string& name) {
	if(inDownloads) {
		if(name == sDownload) {
			cur = NULL;
		} else if(name == sDownloads) {
			inDownloads = false;
		}
	}
}

void QueueManager::noDeleteFileList(const string& path) {
	if(!SETTING(KEEP_LISTS)) {
		protectedFileLists.push_back(path);
	}
}

// SearchManagerListener
void QueueManager::on(SearchManagerListener::SR, const SearchResultPtr& sr) noexcept {
	bool added = false;
	bool wantConnection = false;

	{
		Lock l(cs);
		auto matches = fileQueue.find(sr->getTTH());

		for(auto i = matches.begin(); i != matches.end(); ++i) {
			QueueItem* qi = *i;

			// Size compare to avoid popular spoof
			if(qi->getSize() == sr->getSize() && !qi->isSource(sr->getUser()) && !qi->isBadSource(sr->getUser())) {
				try {
					if(!SETTING(AUTO_SEARCH_AUTO_MATCH))
						wantConnection = addSource(qi, HintedUser(sr->getUser(), sr->getHubURL()), 0);
					added = true;
				} catch(const Exception&) {
					// ...
				}
				break;
			}
		}
	}

	if(added && SETTING(AUTO_SEARCH_AUTO_MATCH)) {
		try {
			addList(HintedUser(sr->getUser(), sr->getHubURL()), QueueItem::FLAG_MATCH_QUEUE);
		} catch(const Exception&) {
			// ...
		}
	}
	if(added && sr->getUser()->isOnline() && wantConnection) {
		ConnectionManager::getInstance()->getDownloadConnection(HintedUser(sr->getUser(), sr->getHubURL()));
	}

}

// ClientManagerListener
void QueueManager::on(UsersManagerListener::UserConnected, const UserPtr& aUser) noexcept {
	bool hasDown = false;
	{
		Lock l(cs);
		for(int i = 0; i < QueueItem::LAST; ++i) {
			auto j = userQueue.getList(i).find(aUser);
			if(j != userQueue.getList(i).end()) {
				for(auto m = j->second.begin(); m != j->second.end(); ++m)
					fire(QueueManagerListener::StatusUpdated(), *m);
				if(i != QueueItem::PAUSED)
					hasDown = true;
			}
		}
	}

	if(hasDown) {
		// the user just came on, so there's only 1 possible hub, no need for a hint
		ConnectionManager::getInstance()->getDownloadConnection(HintedUser(aUser, string()));
	}
}

void QueueManager::on(UsersManagerListener::UserDisconnected, const UserPtr& aUser) noexcept {
	bool hasTestSURinQueue = false;
	Lock l(cs);
	for(int i = 0; i < QueueItem::LAST; ++i) {
		auto j = userQueue.getList(i).find(aUser);
		if(j != userQueue.getList(i).end()) {
			for(auto m = j->second.begin(); m != j->second.end(); ++m) {
				if((*m)->isSet(QueueItem::FLAG_TESTSUR))  hasTestSURinQueue = true;
				fire(QueueManagerListener::StatusUpdated(), *m);
		}
	}
	if(hasTestSURinQueue)
		removeTestSUR(HintedUser(aUser, string()));
  }
}

void QueueManager::on(TimerManagerListener::Second, uint64_t aTick) noexcept {
	if(dirty && ((lastSave + 10000) < aTick)) {
		saveQueue();
	}
}

bool QueueManager::checkSfv(QueueItem* qi, Download* d) {
	SFVReader sfv(qi->getTarget());

	if(sfv.hasCRC()) {
		bool crcMatch = false;
		try {
			crcMatch = (calcCrc32(qi->getTempTarget()) == sfv.getCRC());
		} catch(const FileException& ) {
			// Couldn't read the file to get the CRC(!!!)
		}

		if(!crcMatch) {
			/// @todo There is a slight chance that something happens with a file while it's being saved to disk
			/// maybe calculate tth along with crc and if tth is ok and crc is not flag the file as bad at once
			/// if tth mismatches (possible disk error) then repair / redownload the file

			File::deleteFile(qi->getTempTarget());
			qi->resetDownloaded();
			dcdebug("QueueManager: CRC32 mismatch for %s\n", qi->getTarget().c_str());
			LogManager::getInstance()->message(string(F_("CRC32 inconsistency (SFV-Check): ") + Util::addBrackets(qi->getTarget())),LogManager::Sev::LOW);

			setPriority(qi->getTarget(), QueueItem::PAUSED);

			QueueItem::SourceList sources = qi->getSources();
			for(QueueItem::SourceConstIter i = sources.begin(); i != sources.end(); ++i) {
				removeSource(qi->getTarget(), i->getUser(), QueueItem::Source::FLAG_CRC_FAILED, false);
			}

			fire(QueueManagerListener::CRCFailed(), d, _("CRC32 inconsistency (SFV-Check)"));
		} else {
			dcdebug("QueueManager: CRC32 match for %s\n", qi->getTarget().c_str());
			fire(QueueManagerListener::CRCChecked(), d);
		}
		return true;
	}
	return false;
}

uint32_t QueueManager::calcCrc32(const string& file) {
	CRC32Filter crc32;
	FileReader(true).read(file, [&](const void* x, size_t n) {
		return crc32(x, n), true;
	});
	return crc32.getValue();
}

void QueueManager::logFinishedDownload(QueueItem* qi, Download*, bool crcChecked)
{
	ParamMap params;
	params["target"] = qi->getTarget();
	params["fileSI"] = Util::toString(qi->getSize());
	params["fileSIshort"] = Util::formatBytes(qi->getSize());
	params["fileTR"] = qi->getTTH().toBase32();
	params["sfv"] = Util::toString(crcChecked ? 1 : 0);

	{
		auto lock = FinishedManager::getInstance()->lockLists();

		auto& map = FinishedManager::getInstance()->getMapByFile(false);
		auto it = map.find(qi->getTarget());
		if(it != map.end()) {
			auto entry = it->second;
			if (!entry->getUsers().empty()) {
				StringList nicks, cids, ips, hubNames, hubUrls, temp;
				string ip;
				for(auto i = entry->getUsers().begin(), iend = entry->getUsers().end(); i != iend; ++i) {

					nicks.push_back(Util::toString(ClientManager::getInstance()->getNicks(*i)));
					cids.push_back(i->user->getCID().toBase32());

					ip.clear();
					if (i->user->isOnline()) {
						OnlineUser* u = ClientManager::getInstance()->findOnlineUser(*i);
						if (u) {
							ip = u->getIdentity().getIp();
						}
					}
					if (ip.empty()) {
						ip = _("Offline");
					}
					ips.push_back(ip);

					temp = UsersManager::getInstance()->getHubNames(*i);
					if(temp.empty()) {
						temp.push_back(_("Offline"));
					}
					hubNames.push_back(Util::toString(temp));

					temp = UsersManager::getInstance()->getHubs(*i);
					if(temp.empty()) {
						temp.push_back(_("Offline"));
					}
					hubUrls.push_back(Util::toString(temp));
				}

				params["userNI"] = Util::toString(nicks);
				params["userCID"] = Util::toString(cids);
				params["userI4"] = Util::toString(ips);
				params["hubNI"] = Util::toString(hubNames);
				params["hubURL"] = Util::toString(hubUrls);
			}

			params["fileSIsession"] = Util::toString(entry->getTransferred());
			params["fileSIsessionshort"] = Util::formatBytes(entry->getTransferred());
			params["fileSIactual"] = Util::toString(entry->getActual());
			params["fileSIactualshort"] = Util::formatBytes(entry->getActual());

			params["speed"] = string(Util::formatBytes(entry->getAverageSpeed()+_("/s") ));
			params["time"] = Util::formatSeconds(entry->getMilliSeconds() / 1000);
		}
	}

	LOG(LogManager::FINISHED_DOWNLOAD, params);
}

//RSX++
string QueueManager::addFileListCheck(const HintedUser& aUser) {
	StringList nicks = ClientManager::getInstance()->getNicks(aUser);
	string nick = nicks.empty() ? Util::emptyString : Util::cleanPathChars(nicks[0]) + ".";
	string fname = nick + aUser.user->getCID().toBase32();

	if(aUser.user == ClientManager::getInstance()->getMe())
		return string();

	add(Util::getListPath() + fname, -1, TTHValue(), aUser, (Flags::MaskType)(QueueItem::FLAG_USER_LIST | QueueItem::FLAG_CHECK_FILE_LIST));
	return fname;
}
//END
void QueueManager::lockedOperation(const function<void (const QueueItem::StringMap&)>& currentQueue) {
	Lock l(cs);
	if(currentQueue) currentQueue(fileQueue.getQueue());
}
} // namespace dcpp
