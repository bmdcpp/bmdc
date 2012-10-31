/*
 * Copyright © 2010-2012 Mank <freedcpp at seznam dot cz>
 * Copyright © 2010-2011 Eugene Petrov <dhamp@ya.ru>
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * In addition, as a special exception, compiling, linking, and/or
 * using OpenSSL with this program is allowed.
 */

#ifndef CMDDEBUG_HH
#define CMDDEBUG_HH

#include <dcpp/stdinc.h>
#include <dcpp/DCPlusPlus.h>
#include <dcpp/ClientManager.h>
#include <dcpp/Util.h>
#include <dcpp/PluginManager.h>

#include "bookentry.hh"
#include "treeview.hh"
#include "wulformanager.hh"

class cmddebug:
    public BookEntry,
    private dcpp::ClientManagerListener,
    public dcpp::Thread
{
    public:
        cmddebug();
        virtual ~cmddebug();
        virtual void show();
        //GUI FCE
        void add_gui(time_t t, std::string file);

    private:
		typedef std::unordered_map<std::string, int> Iters;
		//GUI
		static void onClearButton(GtkWidget *widget, gpointer data);

		// Client functions
		void ini_client();
		bool stop;

		dcpp::CriticalSection cs;
		dcpp::Semaphore s;
		std::deque<std::string> cmdList;

		int run() {
			setThreadPriority(dcpp::Thread::LOW);
			std::string x;
			stop = false;

			while(true) {
				s.wait();

				if(stop)
					break;
				{
					dcpp::Lock l(cs);

					if(cmdList.empty()) continue;

					x = cmdList.front();
					cmdList.pop_front();
				}
			typedef Func2<cmddebug,time_t,std::string> F2;
			//time_t tt = time(NULL);
			F2 *func = new F2(this, &cmddebug::add_gui, time(NULL), x);
			WulforManager::get()->dispatchGuiFunc(func);
			}

			stop = false;
			return 0;
	}

	void addCmd(const std::string& cmd) {
          dcpp::Lock l(cs);
          cmdList.push_back(cmd);
		s.signal();
	}

	//ClientManager*/
	void on(dcpp::ClientManagerListener::ClientConnected, dcpp::Client* c) noexcept;
	void on(dcpp::ClientManagerListener::ClientDisconnected, dcpp::Client* c) noexcept;

	static void onScroll_gui(GtkAdjustment *adjustment, gpointer data);
	static void onResize_gui(GtkAdjustment *adjustment, gpointer data);
	void UpdateCombo(dcpp::Client* c, bool add);

	GtkTextBuffer *buffer;
	static const int maxLines = 1000;
	GtkTextIter iter;
	bool scrollToBottom;
	GtkTextMark *cmdMark;
	GtkListStore *store;
	Iters iters;
	dcpp::HookSubscriber *hubIn, *hubOut, *clientIn, *clientOut;

	/* HubData */
	static Bool onHubDataIn(HubDataPtr iHub, const char* message, dcptr_t pCommon);
	static Bool DCAPI netHubInEvent(dcptr_t pObject, dcptr_t pData, dcptr_t pCommon, Bool* /*bBreak*/) { return onHubDataIn((HubDataPtr)pObject, (char*)pData, pCommon); }

	static Bool onHubDataOut(HubDataPtr oHub, const char* message, dcptr_t pCommon);
	static Bool DCAPI netHubOutEvent(dcptr_t pObject, dcptr_t pData, dcptr_t pCommon, Bool* /*bBreak*/) { return onHubDataOut((HubDataPtr)pObject, (char*)pData, pCommon); }

	/* ClientData */
	static Bool onConnDataIn(ConnectionDataPtr iConn, const char* message, dcptr_t pCommon);
	static Bool DCAPI netConnInEvent(dcptr_t pObject, dcptr_t pData, dcptr_t pCommon, Bool* /*bBreak*/) { return onConnDataIn((ConnectionDataPtr)pObject, (char*)pData, pCommon); }

	static Bool onConnDataOut(ConnectionDataPtr oConn, const char* message, dcptr_t pCommon);
	static Bool DCAPI netConnOutEvent(dcptr_t pObject, dcptr_t pData, dcptr_t pCommon, Bool* /*bBreak*/) { return onConnDataOut((ConnectionDataPtr)pObject, (char*)pData, pCommon); }

};

#else
class cmddebug;
#endif
