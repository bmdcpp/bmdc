/*
 * Copyright © 2010-2017 BMDC++
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

#include "../dcpp/stdinc.h"
#include "../dcpp/DCPlusPlus.h"
#include "../dcpp/ClientManager.h"
#include "../dcpp/Util.h"
#include "../dcpp/DebugManager.h"

#include "bookentry.hh"
#include "treeview.hh"
#include "wulformanager.hh"

class cmddebug:
    public BookEntry,
    private dcpp::ClientManagerListener,
	private dcpp::DebugManagerListener,
    public dcpp::Thread
{
	private:
		using dcpp::ClientManagerListener::on;
		using dcpp::DebugManagerListener::on;
    public:
        cmddebug();
        virtual ~cmddebug();
        virtual void show();
   		void preferences_gui();

        //Gui
        void add_gui(std::string str);

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
				typedef Func1<cmddebug,std::string> F1;
				F1 *func = new F1(this, &cmddebug::add_gui, x);
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

	// ClientManager
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
	Iters iters;

	std::string getType(unsigned int type,unsigned int dir)
	{	std::string tmp = dcpp::Util::emptyString;
		bool hub = false;
		switch(type){
			case dcpp::DebugManager::TYPE_HUB: 
			{
						tmp += "Hub:\t"; 
						hub = true;
				break;
			}
			case dcpp::DebugManager::TYPE_CLIENT: 
			{
						tmp += "Client:\t"; 
				break;
			}
			default:break;
		}
		
		switch(dir){
			case dcpp::DebugManager::INCOMING:{
					if(gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(getWidget(hub ? "hub_in_button" : "client_in_button"))) == TRUE)
							tmp+="\t\tIncoming: "; break;
				}			
			case dcpp::DebugManager::OUTGOING: {
					if(gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(getWidget(hub ? "hub_out_button": "client_out_button"))) == TRUE)
							tmp+="\t\tOutcoming: "; break;
			}				
			default:break;
		}
		return tmp;	
	}		
		
	virtual void on(dcpp::DebugManagerListener::DebugCommand, const std::string& m, uint8_t type, uint8_t dir, const std::string& ip) noexcept {
			gchar *fUrl = NULL;
			fUrl = gtk_combo_box_text_get_active_text (GTK_COMBO_BOX_TEXT(getWidget("comboboxadr")));

			if( (fUrl != NULL) && ( gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(getWidget("by_ip_button"))) == TRUE))  {
				if(!strcmp(fUrl, ip.c_str())) {
					addCmd("\t"+getType(type,dir)+"\tIP (Address): \t"+ip+" \t:\t "+m);
					return;
				}	
			}
			else
				addCmd("\t"+getType(type,dir)+"\tIP (Address): \t"+ip+" \t:\t "+m);
	}
	virtual void on(dcpp::DebugManagerListener::DebugDetection, const std::string& m) noexcept {
				addCmd(m);
	}

};

#else
class cmddebug;
#endif
