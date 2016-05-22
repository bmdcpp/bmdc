/*
 * Copyright © 2004-2012 Jens Oknelid, paskharen@gmail.com
 * Copyright © 2010-2016 BMDC
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

#ifndef _BMDC_HUB_HH
#define _BMDC_HUB_HH

#include <dcpp/stdinc.h>
#include <dcpp/DCPlusPlus.h>
#include <dcpp/Client.h>
#include <dcpp/FavoriteManager.h>
#include <dcpp/QueueManager.h>
#include <dcpp/TimerManager.h>
#include <dcpp/Flags.h>
#include "bookentry.hh"
#include "treeview.hh"
#include "sound.hh"
#include "notify.hh"
#include "message.hh"
#include "WulforUtil.hh"
#include <queue>
#include <map>

class UserCommandMenu;
class WulforSettingsManager;
class EmoticonsDialog;

class Hub:
	public BookEntry,
	public dcpp::ClientListener,
	public dcpp::FavoriteManagerListener,
	public dcpp::QueueManagerListener,
	public dcpp::TimerManagerListener
{
	private:
		using dcpp::ClientListener::on;
		using dcpp::FavoriteManagerListener::on;
		using dcpp::QueueManagerListener::on;	
		using dcpp::TimerManagerListener::on;
		mutable dcpp::CriticalSection cs;
	public:
		Hub(const std::string &address, const std::string &encoding);
		virtual ~Hub();
		virtual void show();

		// Client functions
		void reconnect_client();

		// GUI functions
		void preferences_gui();

		GtkWidget *createmenu() override;

		bool findNick_gui_p(std::string &word)
		{
			GtkTreeIter iter;
			return findNick_gui(word,&iter);
		}

	private:
		dcpp::FavoriteHubEntry* getFavoriteHubEntry();
		typedef std::unordered_map<std::string, std::string> UserMap;
		typedef std::unordered_map<GtkWidget*, std::string> ImageList;
		typedef std::pair<std::string, GtkWidget*> ImageLoad;

		// GUI functions
		void setStatus_gui(std::string statusBar, std::string text);
		bool findUser_gui(const std::string &cid, GtkTreeIter *iter);
		bool findNick_gui(const std::string &nick, GtkTreeIter *iter);
		void updateUser_gui(dcpp::StringMap params);
		void removeUser_gui(std::string cid);
		void removeTag_gui(const std::string &nick);
		void clearNickList_gui();
		void popupNickMenu_gui();
		void getPassword_gui();
		void addMessage_gui(std::string cid, std::string message, Msg::TypeMsg typemsg);
		void applyTags_gui(const std::string cid, const std::string line);

		void applyEmoticons_gui();
		void updateCursor_gui(GtkWidget *widget);
		void getSettingTag_gui(WulforSettingsManager *wsm, Tag::TypeTag type, std::string &fore, std::string &back, bool &bold, bool &italic);
		GtkTextTag* createTag_gui(const std::string &tagname, Tag::TypeTag type);
public:
		void addStatusMessage_gui(std::string message, Msg::TypeMsg typemsg, Sound::TypeSound sound);
		void addStatusMessage_gui(std::string message, Msg::TypeMsg typemsg, Sound::TypeSound sound, Notify::TypeNotify notify);
		void nickToChat_gui(const std::string &nick);
private:		
		void addFavoriteUser_gui(dcpp::StringMap params);
		void removeFavoriteUser_gui(dcpp::StringMap params);
		//BMDC++
		void addProtected_gui(dcpp::StringMap params);

		void addPrivateMessage_gui(Msg::TypeMsg typemsg, std::string nick, std::string cid, std::string url, std::string message, bool useSetting);
		//[BBCodes
		void loadImage_gui(std::string target, std::string tth);
		void openImage_gui(std::string target);
		void insertBBcodeEntry_gui(std::string ch);
		//BMDC++
		void set_Header_tooltip_gui();
		void columnHeader(int num, std::string name);

		// GUI callbacks
		static gboolean onFocusIn_gui(GtkWidget *widget, GdkEventFocus *event, gpointer data);
		static gboolean onNickListButtonPress_gui(GtkWidget *widget, GdkEventButton *event, gpointer data);
		static gboolean onNickListButtonRelease_gui(GtkWidget *widget, GdkEventButton *event, gpointer data);
		static gboolean onNickListKeyRelease_gui(GtkWidget *widget, GdkEventKey *event, gpointer data);
		static gboolean onEntryKeyPress_gui(GtkWidget *widget, GdkEventKey *event, gpointer data);
		static gboolean onNickTagEvent_gui(GtkTextTag *tag, GObject *textView, GdkEvent *event, GtkTextIter *iter, gpointer data);
		static gboolean onLinkTagEvent_gui(GtkTextTag *tag, GObject *textView, GdkEvent *event, GtkTextIter *iter, gpointer data);
		static gboolean onHubTagEvent_gui(GtkTextTag *tag, GObject *textView, GdkEvent *event, GtkTextIter *iter, gpointer data);
		static gboolean onMagnetTagEvent_gui(GtkTextTag *tag, GObject *textView, GdkEvent *event, GtkTextIter *iter, gpointer data);
		static gboolean onChatPointerMoved_gui(GtkWidget *widget, GdkEventMotion *event, gpointer data);
		static gboolean onChatVisibilityChanged_gui(GtkWidget *widget, GdkEventVisibility *event, gpointer data);
		static gboolean onEmotButtonRelease_gui(GtkWidget *widget, GdkEventButton *event, gpointer data);
		static void onChatScroll_gui(GtkAdjustment *adjustment, gpointer data);
		static void onChatResize_gui(GtkAdjustment *adjustment, gpointer data);
		static void onSendMessage_gui(GtkEntry *entry, gpointer data);
		static void onNickToChat_gui(GtkMenuItem *item, gpointer data);
		static void onBrowseItemClicked_gui(GtkMenuItem *item, gpointer data);
		static void onMatchItemClicked_gui(GtkMenuItem *item, gpointer data);
		static void onMsgItemClicked_gui(GtkMenuItem *item, gpointer data);
		static void onGrantItemClicked_gui(GtkMenuItem *item, gpointer data);
		static void onRemoveUserItemClicked_gui(GtkMenuItem *item, gpointer data);
		static void onCopyURIClicked_gui(GtkMenuItem *item, gpointer data);
		static void onOpenLinkClicked_gui(GtkMenuItem *item, gpointer data);
		static void onOpenHubClicked_gui(GtkMenuItem *item, gpointer data);
		static void onSearchMagnetClicked_gui(GtkMenuItem *item, gpointer data);
		static void onMagnetPropertiesClicked_gui(GtkMenuItem *item, gpointer data);
		static void onUserListToggled_gui(GtkWidget *widget, gpointer data);
		static void onAddFavoriteUserClicked_gui(GtkMenuItem *item, gpointer data);
		static void onRemoveFavoriteUserClicked_gui(GtkMenuItem *item, gpointer data);
		static void onPasswordDialog(GtkWidget *dialog, gint response, gpointer data);
		static void onDownloadToClicked_gui(GtkMenuItem *item, gpointer data);
		static void onDownloadClicked_gui(GtkMenuItem *item, gpointer data);
		static void onCommandClicked_gui(GtkWidget *widget, gpointer data);
		static gboolean onChatCommandButtonRelease_gui(GtkWidget *widget, GdkEventButton *event, gpointer data);
		static void onUseEmoticons_gui(GtkWidget *widget, gpointer data);
		static void onImageDestroy_gui(GtkWidget *widget, gpointer data);
		static void onDownloadImageClicked_gui(GtkMenuItem *item, gpointer data);
		static void onRemoveImageClicked_gui(GtkMenuItem *item, gpointer data);
		static void onOpenImageClicked_gui(GtkMenuItem *item, gpointer data);
		static gboolean onImageEvent_gui(GtkWidget *widget, GdkEventButton *event, gpointer data);
		static void onItalicButtonClicked_gui(GtkWidget *widget, gpointer data);
		static void onBoldButtonClicked_gui(GtkWidget *widget, gpointer data);
		static void onUnderlineButtonClicked_gui(GtkWidget *widget, gpointer data);
		//[BMDC++
		static void onPartialFileListOpen_gui(GtkMenuItem *item, gpointer data);
		static void onCloseItem(gpointer data);
		static void onCopyHubUrl(gpointer data);
		static void onAddFavItem(gpointer data);
		static void onRemoveFavHub(gpointer data);
		static void onSetTabText(gpointer data);
		static void onReconnectItemTab(gpointer data);
		static void onAddIgnoreUserItemClicked_gui(GtkMenuItem *item, gpointer data);
		static void onRemoveIgnoreUserItemClicked_gui(GtkMenuItem *item, gpointer data);
		static void onShowReportClicked_gui(GtkMenuItem *item, gpointer data);
		static void selection_changed_userlist_gui(GtkTreeSelection *selection, GtkWidget *tree_view);
		static gboolean onUserListTooltip_gui(GtkWidget *widget, gint x, gint y, gboolean keyboard_tip, GtkTooltip *_tooltip, gpointer data);
		static gboolean onIpTagEvent_gui(GtkTextTag *tag, GObject *textView, GdkEvent *event , GtkTextIter *iter, gpointer data);
		static void onCopyIpItem_gui(GtkWidget *wid, gpointer data);
		static void onRipeDbItem_gui(GtkWidget *wid, gpointer data);
		static void onTestSURItemClicked_gui(GtkMenuItem *item, gpointer data);
		static void onCheckFLItemClicked_gui(GtkMenuItem *item , gpointer data);
		static void onUnProtectUserClicked_gui(GtkMenuItem *item , gpointer data);
		static void onProtectUserClicked_gui(GtkMenuItem *item , gpointer data);
		static void onRefreshUserListClicked_gui(GtkWidget *wid, gpointer data);

		static void on_setImage_tab(GtkButton *widget, gpointer data);
		static void onToglleButtonIcon(GtkToggleButton *button, gpointer data);
		//[colorize userlist
		static void makeColor(GtkTreeViewColumn *column, GtkCellRenderer *cell, GtkTreeModel *model, GtkTreeIter *iter,gpointer data);
		static gint sort_iter_compare_func_nick(GtkTreeModel *model, GtkTreeIter  *a,GtkTreeIter  *b,  gpointer  data);

		// Client functions
		void addFavoriteUser_client(const std::string& cid);
		void removeFavoriteUser_client(const std::string& cid);
		void connectClient_client(std::string address, std::string encoding);
		void disconnect_client(bool shutdownHub = false);
		void setPassword_client(std::string password);
		void sendMessage_client(std::string message, bool thirdPerson);
		void getFileList_client(std::string cid, bool match,bool partial);
		void grantSlot_client(std::string cid);
		void removeUserFromQueue_client(std::string cid);
		void redirect_client(std::string address, bool follow);
		void addAsFavorite_client();
		void removeAsFavorite_client();

		void getParams_client(dcpp::StringMap &user, dcpp::Identity &id);
		void download_client(std::string target, int64_t size, std::string tth, std::string cid);
		std::string realFile_client(std::string tth);
		void openImage_client(std::string tth);
		//BMDC++
		std::string formatAdditionalInfo(const std::string& aIp, bool sIp, bool sCC);
		std::string getIcons(const dcpp::Identity& id);

		void getPartialFileList_client(std::string cid);

		void SetTabText(gpointer data);

		void setColorRow(const std::string cell);
		void setColorsRows();
		void clickAction(gpointer data);

		// Favorite callbacks
		virtual void on(dcpp::FavoriteManagerListener::UserAdded, const dcpp::FavoriteUser &user) noexcept;
		virtual void on(dcpp::FavoriteManagerListener::UserRemoved, const dcpp::FavoriteUser &user) noexcept;
		virtual void on(dcpp::FavoriteManagerListener::FavoriteIAdded, const std::string &nick, dcpp::FavoriteUser* &user) noexcept;
		virtual void on(dcpp::FavoriteManagerListener::FavoriteIRemoved, const std::string &nick, dcpp::FavoriteUser* &user) noexcept;
		virtual void on(dcpp::FavoriteManagerListener::StatusChanged, const dcpp::FavoriteUser& fu) noexcept;
		// Client callbacks
		virtual void on(dcpp::ClientListener::Connecting, dcpp::Client *) noexcept;
		virtual void on(dcpp::ClientListener::Connected, dcpp::Client *) noexcept;
		virtual void on(dcpp::ClientListener::UserUpdated, dcpp::Client *, const dcpp::OnlineUser &user) noexcept;
		virtual void on(dcpp::ClientListener::UsersUpdated, dcpp::Client *, const dcpp::OnlineUserList &list) noexcept;
		virtual void on(dcpp::ClientListener::UserRemoved, dcpp::Client *, const dcpp::OnlineUser &user) noexcept;
		virtual void on(dcpp::ClientListener::Redirect, dcpp::Client *, const std::string &address) noexcept;
		virtual void on(dcpp::ClientListener::Failed, dcpp::Client *, const std::string &reason) noexcept;
		virtual void on(dcpp::ClientListener::GetPassword, dcpp::Client *) noexcept;
		virtual void on(dcpp::ClientListener::HubUpdated, dcpp::Client *) noexcept;
		virtual void on(dcpp::ClientListener::Message, dcpp::Client*, const dcpp::ChatMessage& message) noexcept;;
		virtual void on(dcpp::ClientListener::StatusMessage, dcpp::Client *, const std::string &message, int flag) noexcept;;
		virtual void on(dcpp::ClientListener::NickTaken, dcpp::Client *) noexcept;
		virtual void on(dcpp::ClientListener::SearchFlood, dcpp::Client *, const std::string &message) noexcept;
		virtual void on(dcpp::ClientListener::CheatMessage, dcpp::Client *, const std::string &msg) noexcept;
		virtual void on(dcpp::ClientListener::HubTopic, dcpp::Client *, const std::string &top) noexcept;
		virtual void on(dcpp::ClientListener::ClientLine, dcpp::Client* , const std::string &mess, int type) noexcept;
		virtual void on(dcpp::QueueManagerListener::Finished, dcpp::QueueItem *item, const std::string& dir, int64_t avSpeed) noexcept;
		
		typedef enum 
		{
			ONE_HOUR = 1,
			TWO_HOUR ,
			THREE_HOUR,
			FOUR_HOUR ,
			FIVE_HOUR ,
			SIX_HOUR ,
			SEVEN_HOUR ,
			EIGHT_HOUR,
			NINE_HOUR,
			TEN_HOUR
		} TempTime;
		
		typedef std::multimap<uint64_t,std::string> TempMap;
		TempMap listTempsNicks;
		TempMap listTempsIps;
		TempMap listTempsCids;
		uint64_t lastTickCid;
		uint64_t lastTickNick;
		uint64_t lastTickIp;
		static void onClickMenuItemTime(GtkMenuItem* item,gpointer data);
		// TimerManagerListener
		virtual void on(dcpp::TimerManagerListener::Minute, uint64_t aTick) noexcept
		{
			dcpp::Lock l(cs);
			dcdebug("[HUB] TimerManager %lud",aTick);
			if(aTick > lastTickCid)
			{
				lastTickCid = aTick + (60*60*1000);//hour
				for(auto i = listTempsCids.begin();i!= listTempsCids.end();++i)
				{
					if( ( (*i).first == 60*60*1000) ||
					( (*i).first == 60*60*1000*2) ||
					( (*i).first == 60*60*1000*3) ||
					 ( (*i).first == 60*60*1000*4) ||
					 ( (*i).first == 60*60*1000*5) ||
					 ( (*i).first == 60*60*1000*6) ||
					 ( (*i).first == 60*60*1000*7) ||
					 ( (*i).first == 60*60*1000*8) ||
					 ( (*i).first == 60*60*1000*9) ||
					 ( (*i).first == 60*60*1000*10))
					{
						listTempsCids.erase(i);
					
					}
				}
			}	
			if(aTick > lastTickNick)
			{
				lastTickNick = aTick + (60*60*1000);
				for(auto i = listTempsNicks.begin();i!= listTempsNicks.end();++i)
				{
					if( ( (*i).first == 60*60*1000) ||
					( (*i).first == 60*60*1000*2) ||
					( (*i).first == 60*60*1000*3) ||
					 ( (*i).first == 60*60*1000*4) ||
					 ( (*i).first == 60*60*1000*5) ||
					 ( (*i).first == 60*60*1000*6) ||
					 ( (*i).first == 60*60*1000*7) ||
					 ( (*i).first == 60*60*1000*8) ||
					 ( (*i).first == 60*60*1000*9) ||
					 ( (*i).first == 60*60*1000*10))
					{
						listTempsNicks.erase(i);
					
					}
				}
			}	
			
			for(auto i = listTempsIps.begin();i!= listTempsIps.end();++i)
			{
				if(aTick > lastTickIp)
				{
					lastTickIp = aTick + ((*i).first*(60*60*1000));
					listTempsIps.erase(i);
					continue;
				}
			}	
		}

		
		UserMap userMap;
		UnMapIter userIters;
		UserMap userFavoriteMap;
		ImageList imageList;
		ImageLoad imageLoad;
		dcpp::StringPair imageMagnet;
		GtkTextTag *TagsMap[Tag::TAG_LAST];
		dcpp::Client *client;
		TreeView nickView;
		GtkListStore *nickStore;
		GtkTreeSelection *nickSelection;
		GtkTextBuffer *chatBuffer;
		GtkTextMark *chatMark, *start_mark, *end_mark, *tag_mark, *emot_mark;
		std::vector<std::string> history;
		GdkCursor *handCursor;
		GtkTextTag *selectedTag;
		UserCommandMenu *userCommandMenu, *userCommandMenu1, *userCommandMenu2;
		EmoticonsDialog *emotdialog;
		GtkTextTag *BoldTag, *UnderlineTag, *ItalicTag;
		std::queue<std::string> statustext;
		static const std::string tagPrefix;
		std::string completionKey;
		std::string sort; //sort order for TreeView
		std::string address;
		std::string encoding;
		std::string selectedTagStr;
		GtkWidget *tab_image;
		GtkWidget *tab_button;
		GtkWidget *m_menu;
		Tag::TypeTag tagMsg;
		unsigned int historyIndex;
		static const int maxLines = 1000;
		static const int maxHistory = 30;
		int64_t totalShared;
		int ImgLimit;
		gint totalEmoticons;
		gint oldType;
		bool scrollToBottom;
		bool PasswordDialog;
		bool WaitingPassword;
		bool useEmoticons;
		bool notCreated;
		bool isFavBool; 

};
#else
class Hub;
#endif
