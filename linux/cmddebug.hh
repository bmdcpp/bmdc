/*
 * Copyright © 2010-2013 Mank <freedcpp at seznam dot cz>
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

#ifndef _BMDC_CMDDEBUG_HH
#define _BMDC_CMDDEBUG_HH

#include <dcpp/stdinc.h>
#include <dcpp/DCPlusPlus.h>
#include <dcpp/ClientManager.h>
#include <dcpp/Util.h>
#include <dcpp/DebugManager.h>

#include "bookentry.hh"
#include "treeview.hh"
#include "wulformanager.hh"

class cmddebug:
    public BookEntry,
    private dcpp::ClientManagerListener,
	private dcpp::DebugManagerListener,
    public dcpp::Thread
{
    public:
        cmddebug();
        virtual ~cmddebug();
        virtual void show();
        //Gui
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
	std::string getDirection(unsigned int dir)
	{
		std::string tmp = dcpp::Util::emptyString;
		switch(dir){
		case dcpp::DebugManager::INCOMING: tmp+="\t\tIncoming: "; break;
		case dcpp::DebugManager::OUTGOING: tmp+="\t\tOutcoming: "; break;
		default:break;
		}	
		return tmp;
	}
	std::string getType(unsigned int type,unsigned int dir)
	{	std::string tmp = dcpp::Util::emptyString;
		switch(type){
			case dcpp::DebugManager::TYPE_HUB: tmp += "Hub:\t"+getDirection(dir); break;
			case dcpp::DebugManager::TYPE_CLIENT: tmp += "Client:\t"+getDirection(dir); break;
			default:break;
		}
		return tmp;
	}		
		
	virtual void on(dcpp::DebugManagerListener::DebugCommand, const std::string& m, uint8_t type, uint8_t dir, const std::string& ip) noexcept {
			gchar *fUrl = NULL;
			GtkTreeIter piter;
			GtkTreeModel *model = NULL;
			if( gtk_combo_box_get_active_iter( GTK_COMBO_BOX(getWidget("comboboxadr")), &piter ) ) {
				model = gtk_combo_box_get_model( GTK_COMBO_BOX(getWidget("comboboxadr")) );
				gtk_tree_model_get( model, &piter, 0, &fUrl, -1 );
			}
			if((gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(getWidget("by_ip_button"))) == TRUE) || (!(fUrl == NULL) && (ip == fUrl) ))
				addCmd("\t"+getType(type,dir)+"\tIP (Address):\t"+ip+"\t:\t"+m);
			else
				addCmd("\t"+getType(type,dir)+"\tIP (Address):\t"+ip+"\t:\t"+m);
	}
	virtual void on(dcpp::DebugManagerListener::DebugDetection, const std::string& m) noexcept {
		addCmd(m);
	}

};

#else
class cmddebug;
#endif
