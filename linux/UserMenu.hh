#pragma once
#include <dcpp/stdinc.h>
#include <dcpp/DCPlusPlus.h>
#include "entry.hh"
#include "settingsmanager.hh"
#include "wulformanager.hh"
#include "UserCommandMenu.hh"

using namespace std;
using namespace dcpp;

class UserMenu: public Entry
{
	public:
		enum {
			BROWSE,
			FULL,
			MATCH,	
			GRANT,
			REMQ,
			PM,
			FAVA,
			FAVR,
			IGNA,
			IGNR,
			CHECKFL,
			CHECKTS,
			ENBPRO,
			DSBPRO
		};
		
		UserMenu(GtkWidget *menu) : mainMenu(menu), w_uMenu(NULL) {}
		~UserMenu() {}
		void cleanMenu_gui(){ 
			nicks = Util::emptyString; 
			if(w_uMenu != NULL)
				w_uMenu->cleanMenu_gui();
			gtk_container_foreach(GTK_CONTAINER(mainMenu), (GtkCallback)gtk_widget_destroy, NULL);
		};
		GtkWidget* getContainer() {return mainMenu;}
		void setHub(std::string _hub)
		{hub=_hub;}
		void addNick(std::string _nick)
		{ nicks += ";" + _nick;}
		std::string getNicks()
		{return nicks;}
		std::string getHub() { return hub;}
		#define MAX_ENTRY 14
		bool buildMenu_gui(const std::string &cid)
		{
					
		string color = WGETS("menu-userlist-color");//@ Settings of UserList Menu  text color (1st item)?
		gchar *markup = g_markup_printf_escaped ("<span fgcolor=\"%s\" ><b>%s</b></span>",color.c_str(),nicks.c_str());
		GtkWidget *header = gtk_menu_item_new_with_label(markup);
		GtkWidget *label = gtk_bin_get_child(GTK_BIN(header));
		gtk_label_set_markup (GTK_LABEL (label), markup);
		g_free(markup);
		gtk_widget_show(header)	;
		gtk_menu_shell_append(GTK_MENU_SHELL(mainMenu),header);
		
		GtkWidget* m_menu = gtk_menu_item_new_with_label(_("User Commands"));
		GtkWidget* u_menu = gtk_menu_new();
		gtk_menu_item_set_submenu(GTK_MENU_ITEM(m_menu),u_menu);
		gtk_widget_show(m_menu);
		gtk_widget_show(u_menu);
		gtk_menu_shell_append(GTK_MENU_SHELL(mainMenu), m_menu);	
		w_uMenu = new UserCommandMenu( u_menu ,::UserCommand::CONTEXT_USER);
		addChild(w_uMenu);
		
		
			GtkWidget* item;
			static const gchar* names[MAX_ENTRY][2] =
			{
				{"Browse Filelist","browse"},	
				{"Open Full Filelist","full"},
				{"Match Queune","match"},
				{"Grant slot","grant"},
				{"Remove from Queune","quene"},
				{"Send Private message","pm"},
				{"Add to Favorite users","addf"},
				{"Remove to Favorite users","addr"},
				{"Add to Ignore users","ignf"},
				{"Remove to ignore users","ignr"},
				{"Check Filelist", "check"},
				{"Check TESTSUR","testsur"},
				{"Enable Protect","enbp"},
				{"Disable Protect","disp"}
			};

			for(int i = 0;i < MAX_ENTRY;++i)
			{
				item = gtk_menu_item_new_with_label((names[i][0]));
				g_object_set_data_full(G_OBJECT(item), "type", g_strdup(names[i][1]), g_free);
				g_object_set_data_full(G_OBJECT(item), "cid", g_strdup(cid.c_str()), g_free);
				gtk_widget_show (item);
				g_signal_connect(item, "activate", G_CALLBACK(onBrowseClicked_gui), (gpointer)this);
				gtk_menu_shell_append(GTK_MENU_SHELL(mainMenu),item);
			}
			
			return true;
		}
	private:
		GtkWidget *mainMenu;
		string hub;
		string nicks;
		UserCommandMenu* w_uMenu;
		static void onBrowseClicked_gui(GtkWidget *widget, gpointer data)
		{
			UserMenu* hub = (UserMenu*)data;
			std::string cid = (gchar*)g_object_get_data(G_OBJECT(widget), "cid");
			std::string type = (gchar*)g_object_get_data(G_OBJECT(widget), "type");
			typedef Func2<UserMenu, string, int> F2;
			F2 *func;
			func = new F2(hub, &UserMenu::getFileList_client, cid,  (type == "match") ? MATCH : (type == "browse") ? BROWSE : (type == "grant") ? GRANT : (type == "quene") ? REMQ : (type == "pm") ? PM : (type == "addf") ? FAVA : (type == "addr") ? FAVR : (type == "ignf") ? IGNA : (type == "ignr") ?IGNR : (type == "check") ? CHECKFL : (type == "testsur") ? CHECKTS : (type == "enbp") ? ENBPRO : (type == "disp") ?  DSBPRO : -1 );
			WulforManager::get()->dispatchClientFunc(func);
		}
		
		
void getFileList_client(string cid, int ti)
{
	std::string message;

	if (!cid.empty())
	{
		try
		{
			dcpp::UserPtr user = ClientManager::getInstance()->findUser(CID(cid));
			if (user)
			{
				const dcpp::HintedUser hintedUser(user, getHub());

				if ((ti == BROWSE) && (user == dcpp::ClientManager::getInstance()->getMe()))
				{
					// Don't download file list, open locally instead
					WulforManager::get()->getMainWindow()->openOwnList_client(true);
				}
				
				switch(ti) {
					case BROWSE:
						dcpp::QueueManager::getInstance()->addList(hintedUser,dcpp::QueueItem::FLAG_CLIENT_VIEW | dcpp::QueueItem::FLAG_PARTIAL_LIST);
						break;
					case FULL:
						dcpp::QueueManager::getInstance()->addList(hintedUser,dcpp::QueueItem::FLAG_CLIENT_VIEW);
						break;
					case MATCH:
						dcpp::QueueManager::getInstance()->addList(hintedUser, dcpp::QueueItem::FLAG_MATCH_QUEUE);
						break;
					case GRANT:
						UploadManager::getInstance()->reserveSlot(hintedUser);
						message = _("Slot granted to ") + WulforUtil::getNicks(user, getHub());
						break;
					case REMQ:
						QueueManager::getInstance()->removeSource(user, QueueItem::Source::FLAG_REMOVED);
						break;
					case FAVA:
						FavoriteManager::getInstance()->addFavoriteUser(user);
						break;
					case FAVR:
						FavoriteManager::getInstance()->removeFavoriteUser(user);
						break;
					case IGNA:
						FavoriteManager::getInstance()->addFavoriteUser(user);
						FavoriteManager::getInstance()->setIgnore(user,true);	
						break;
					case IGNR:
						FavoriteManager::getInstance()->addFavoriteUser(user);
						FavoriteManager::getInstance()->setIgnore(user,false);
						break;
					case CHECKFL:	
						ClientManager::getInstance()->addCheckToQueue(hintedUser, true);
						break;
					case CHECKTS:
						ClientManager::getInstance()->addCheckToQueue(hintedUser, false);
						break;
					case ENBPRO:
						if(!user->isSet(User::PROTECT))
							user->setFlag(User::PROTECT);
						break;	
					case DSBPRO:
						//todo UL
						if(!user->isSet(User::PROTECT))
							user->unsetFlag(User::PROTECT);
							break;
					case PM:
						WulforManager::get()->getMainWindow()->addPrivateMessage_gui(Msg::PRIVATE, cid, getHub(), Util::emptyString, true);
						break;
					default:break;
							
				}
									
			}	
			else
			{
				message = _("User not found");
			}
		}
		catch (const dcpp::Exception &e)
		{
			message = e.getError();
			dcpp::LogManager::getInstance()->message(message);
		}
	}

	if (!message.empty())
	{
		WulforManager::get()->getMainWindow()->addPublicStatusMessage_gui(getHub(),message,Msg::SYSTEM, Sound::NONE, Notify::NONE);
	}
}
		
};
