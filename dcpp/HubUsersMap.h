/*
 * Copyright (C) 2007-2013 adrian_007, adrian-007 on o2 point pl
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

#ifndef HUB_USERS_MAP_H
#define HUB_USERS_MAP_H

#include "QueueManager.h"
#include "Client.h"
#include "Thread.h"
#include "SettingsManager.h"
#include "format.h"
#include "Exception.h"

namespace dcpp {

template<bool isADC, typename BaseMap>
class HubUsersMap : public BaseMap {
public:
	HubUsersMap() : clientEngine(NULL) { };
	~HubUsersMap() noexcept {
		stopCheck();
		stopMyINFOCheck();
	}

	void startMyINFOCheck(Client* c) {
		if(!myInfoEngine.isRunning()) {
			myInfoEngine.startCheck(c);
		}
	}

	string startChecking(Client* c, const string& param) noexcept {
		//if(!c->isOp())
		//	return _("You are not an Operator on this hub");
		if(clientEngine != NULL) {
			stopCheck();
			return _("Checking stopped");
		}

		bool cc = false;
		bool cf = false;

		if(param.empty()) {
			cc = c->getCheckClients();
			cf = c->getCheckFilelists();
		} else {
			if(Util::stricmp(param.c_str(), "clients") == 0 || Util::stricmp(param.c_str(), "c") == 0)
				cc = true;
			else if(Util::stricmp(param.c_str(), "filelists") == 0 || Util::stricmp(param.c_str(), "fl") == 0 || Util::stricmp(param.c_str(), "f") == 0)
				cf = true;
			else if(Util::stricmp(param.c_str(), "all") == 0)
				cc = cf = true;
			else
				return _("Incorrect parameters!");
		}
		if(!cc && !cf)
			return _("There is nothing to check - select check type (Clients/FileLists) in Hub Properties");

		if(clientEngine == NULL) {
			clientEngine = new ThreadedCheck(this, c);
			clientEngine->setCheckClients(cc);
			clientEngine->setCheckFilelists(cf);
			if(c->getCheckAtConnect())
				clientEngine->setCheckAtConnect(true);
			clientEngine->startCheck();

			if(cc && !cf)
				return _("Checking started (Clients)");
			else if(!cc && cf)
				return _("Checking started (FileLists)");
			else if(cc && cf)
				return _("Checking started (Clients & FileLists)");
		}
		return Util::emptyString;
	}

	void stopMyINFOCheck() {
		if(myInfoEngine.isRunning()) {
			myInfoEngine.stop = true;
		}
	}

	void stopCheck() noexcept {
		if(clientEngine != NULL) {
            		delete clientEngine;
			clientEngine = NULL;
		}
	}

	bool isDetectorRunning() const {//is this need ?
		return (clientEngine != NULL && clientEngine->isChecking());
	}

private:
	//myinfo check engine
	class ThreadedMyINFOCheck : public Thread {
	public:
		ThreadedMyINFOCheck() : client(NULL), stop(true) { };

		bool isRunning() const { return !stop; }

		void startCheck(Client* _c) {
			if(_c && stop) {
				client = _c;
				start();
			}
		}

		CriticalSection cs;
		int run() {
			stop = false;
			setThreadPriority(Thread::HIGH);
			if(client && client->isConnected()) {
				client->setCheckAtConnect(true);

				OnlineUserList ul;
				client->getUserList(ul);

				Lock l(cs);
				for(OnlineUserList::const_iterator i = ul.begin(); i != ul.end(); ++i) {
					if(stop) {
						break;
					}
					{
						OnlineUser& ou = *(*i);
						if(ou.isCheckable()) {
							string report = ou.getIdentity().myInfoDetect(*(&(ou)));
							if(!report.empty()) {
								ou.getClient().cheatMessage(report);
								ou.getClient().updated(ou);
							}
						}
					}
					sleep(0.0001);
				}
			}
			stop = true;
			return 0;
		}
		Client* client;
		bool stop;
	}myInfoEngine;

	//clients check engine
	class ThreadedCheck : public Thread {
	public:
		ThreadedCheck(HubUsersMap* _u, Client* _c) : client(_c), users(_u),
			keepChecking(false), canCheckFilelist(false), inThread(false), 
			checkAtConnect(false) , checkClients(true) ,checkFilelists(true)  { };

		~ThreadedCheck() {
			keepChecking = inThread = false;

			StringList items;
			OnlineUserList ul;
			client->getUserList(ul);
			for(auto i = ul.begin(); i != ul.end(); ++i) {
				const Identity& id = (*i)->getIdentity();
				if(id.isClientQueued()) {
					string path = Util::getPath(Util::PATH_USER_CONFIG) + "TestSURs//" + id.getTestSURQueued();
					items.push_back(path);
				}
				if(id.isFileListQueued()) {
					string path = Util::getPath(Util::PATH_USER_CONFIG) + id.getFileListQueued();
					items.push_back(path);
				}
			}

			for(auto j = items.begin(); j != items.end(); ++j) {
				try {
					QueueManager::getInstance()->remove(*j);
				} catch(...) {
					//
				}
			}
			join();
			client = NULL;
		}

		bool isChecking() const {
			return inThread && keepChecking;
		}

		inline void startCheck() {
			if(!client || !users) {
				return;
			}
			if(!inThread) {
				start();
			}
		}

		CriticalSection cs;

		enum Actions {
			ADD_CLIENT_CHECK = 0x01,
			REMOVE_CLIENT_CHECK = 0x02,
			ADD_FILELIST_CHECK = 0x04,
			CONTINUE = 0x800,
			BREAK = 0x20
		};

		int preformUserCheck(OnlineUser& ou, const uint8_t clientItems, const uint8_t filelistItems) {
			if(!ou.isCheckable((uint16_t)SETTING(CHECK_DELAY)))
				return CONTINUE;
			if(isADC) {
				if((ou.getUser()->isSet(User::NO_ADC_1_0_PROTOCOL_BIT) || ou.getUser()->isSet(User::NO_ADCS_0_10_PROTOCOL_BIT)) &&
					!(ou.getIdentity().isClientChecked() || ou.getIdentity().isFileListChecked())) {
					//nasty...
					ou.getIdentity().setTestSURChecked(Util::toString(GET_TIME()));
					ou.getIdentity().setFileListChecked(Util::toString(GET_TIME()));
					string report = ou.setCheat("No ADC 1.0/0.10 support", true, false, false);
					client->updated(ou);
					client->cheatMessage(report);
					return BREAK;
				}
			}
			Identity& i = ou.getIdentity();
			if(i.isHidden())
				return CONTINUE;
			if(getCheckClients() && clientItems < SETTING(MAX_TESTSURS)) {
				if(ou.shouldCheckClient()) {
					if(!ou.getChecked(false, false)) {
						return ADD_CLIENT_CHECK | BREAK;
					}
				} else if(i.isClientQueued() && i.isClientChecked()) {
					return REMOVE_CLIENT_CHECK | BREAK;
				}
			}
			if(getCheckFilelists() && filelistItems < SETTING(MAX_FILELISTS)) {
				if(canCheckFilelist) {
					if(ou.shouldCheckFileList()) {
						if(!ou.getChecked(true, false)) {
							return ADD_FILELIST_CHECK | BREAK;
						}
					}
				}
			}
			return CONTINUE;
		}

		int run() {
			inThread = true;
			keepChecking = true;
			setThreadPriority(Thread::LOW);

			if(checkAtConnect) {
				if(checkClients && !checkFilelists)
					client->cheatMessage("*** Checking started (Clients)");
				else if(!checkClients && checkFilelists)
					client->cheatMessage("*** Checking started (FileLists)");
				else if(checkClients && checkFilelists)
					client->cheatMessage("*** Checking started (Clients & FileLists)");

				sleep((SETTING(CHECK_DELAY) + 1000)/1000);
				checkAtConnect = false;
				client->setCheckAtConnect(false);
			}

			if(!client->isConnected()) {
				keepChecking = false;
			}

			canCheckFilelist = !checkClients || !SETTING(CHECK_ALL_CLIENTS_BEFORE_FILELISTS);
			const uint64_t sleepTime = static_cast<uint64_t>(SETTING(SLEEP_TIME))/1000;


			while(keepChecking) {
				dcassert(client != NULL);
				if(client->isConnected()) {
					uint8_t t = 0;
					uint8_t f = 0;
					Lock l(cs);
					QueueManager::getInstance()->lockedOperation([&f,&t](const QueueItem::StringMap& queue) {
					for(auto i = queue.cbegin(); i != queue.cend(); ++i) {
							if(i->second->isSet(QueueItem::FLAG_TESTSUR)) {
								t++;
							} else if(i->second->isSet(QueueItem::FLAG_CHECK_FILE_LIST)) {
								f++;
							}
					}});

					OnlineUser* ou = NULL;
					int action = 0;
					{
						Lock l(client->cs);
						for(typename BaseMap::const_iterator i = users->begin(); i != users->end(); ++i) {
							if(!client->isConnected() || !inThread) break;

							action = preformUserCheck((*(i->second)), t, f);
							if(action & CONTINUE) {
								continue;
							}
							if(action & BREAK) {
								ou = i->second;
								break;
							}
						}
					}

					if(ou != NULL) {
						Lock l(cs);
						if(action & ADD_CLIENT_CHECK) {
							try {
								string fname = QueueManager::getInstance()->addClientCheck( HintedUser(ou->getUser(), client->getHubUrl()) );
								if(!fname.empty())
									ou->getIdentity().setTestSURQueued(fname);
							} catch(...) {
								//
							}
						} else if(action & ADD_FILELIST_CHECK) {
							string fname;
							try {
								fname = QueueManager::getInstance()->addFileListCheck( HintedUser(ou->getUser(), client->getHubUrl()) );
								if(!fname.empty())
									ou->getIdentity().setFileListQueued(fname);
							} catch(...) {

							}
						} else if(action & REMOVE_CLIENT_CHECK) {
							string path = Util::getPath(Util::PATH_USER_CONFIG) + "TestSURs//" + ou->getIdentity().getTestSURQueued();
							if(!Util::fileExists(path)) {
								ou->getIdentity().setTestSURQueued(Util::emptyString);
								ou->getIdentity().setTestSURChecked("1");
							}
							try {
								QueueManager::getInstance()->remove(path);
							} catch(...){  }
						}
					}

					if(!canCheckFilelist) {
						canCheckFilelist = !(action & BREAK);
					}
					sleep(sleepTime);
				}
			}
			inThread = false;
			return 0;
		}
		Client* client;
		HubUsersMap* users;
		GETSET(bool, keepChecking, KeepChecking);
		bool canCheckFilelist;
		bool inThread;
		GETSET(bool, checkAtConnect, CheckAtConnect);
		GETSET(bool, checkClients, CheckClients);
		GETSET(bool, checkFilelists, CheckFilelists);
		
	}*clientEngine;
};

} // namespace dcpp
#endif // HUB_USERS_MAP
/* Partialy edited by Mank*/
/**
 * @file
 * $Id: HubUsersMap.h 141 2011-08-10 00:06:13Z adrian_007 $
 */
