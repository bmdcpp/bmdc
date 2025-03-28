/*
* Copyright © 2004-2012 Jens Oknelid, paskharen@gmail.com
* Copyright © 2011-2025 BMDC
* This file is part of BMDC.
* BMDC is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 2 of the License, or (at your option) any later version.
*
* BMDC is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License along with Xfce-nameday-plugin. If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef _BMDC_HUB_HH
#define _BMDC_HUB_HH

#include "../dcpp/stdinc.h"
#include "../dcpp/DCPlusPlus.h"
#include "../dcpp/Client.h"
#include "../dcpp/FavoriteManager.h"
#include "../dcpp/QueueManager.h"
#include "../dcpp/TimerManager.h"
#include "../dcpp/Flags.h"
#include "bookentry.hh"
#include "treeview.hh"
#include "sound.hh"
#include "notify.hh"
#include "message.hh"
#include "GuiUtil.hh"
#include <queue>
#include <map>
#include "ignoremenu.hh"
#include "IgnoreTempManager.hh"

class UserCommandMenu;
class WulforSettingsManager;
class EmoticonsDialog;

class Hub:
	public BookEntry,
	public dcpp::ClientListener,
	public dcpp::FavoriteManagerListener,
	public dcpp::QueueManagerListener
{
	private:
		using dcpp::ClientListener::on;
		using dcpp::FavoriteManagerListener::on;
		using dcpp::QueueManagerListener::on;
		typedef enum
		{
			BOT = 0,OPERATOR,FAVORITE,IGNORED,PROTECTED,PASIVE,NORMAL
		} UserType;
	public:
		Hub(const std::string &address, const std::string &encoding);
		virtual ~Hub();
		virtual void show();

		// Client functions
		void reconnect_client();

		// GUI functions
		void preferences_gui();

		GMenu *createmenu() override;

		bool findNick_gui_p(std::string &word)
		{
			GtkTreeIter iter;
			return findNick_gui(word,&iter);
		}

	private:
		dcpp::FavoriteHubEntry* getFavoriteHubEntry();
		typedef std::unordered_map<std::string, std::string> UserMap;

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
		void addMessage_gui(std::string cid, std::string message, Msg::TypeMsg typemsg, std::string sCountry = "");
		void applyTags_gui(const std::string cid, const std::string line,std::string sCountry = "");

		void applyEmoticons_gui();
		void updateCursor_gui(GtkWidget *widget);
		void getSettingTag_gui(WulforSettingsManager *wsm, Tag::TypeTag type, std::string &fore, std::string &back, bool &bold, bool &italic);
		GtkTextTag* createTag_gui(const std::string &tagname, Tag::TypeTag type);
public:
		void addStatusMessage_gui(std::string message, Msg::TypeMsg typemsg, Sound::TypeSound sound);
		void addStatusMessage_gui(std::string message, Msg::TypeMsg typemsg, Sound::TypeSound sound, Notify::TypeNotify notify);
private:
		void nickToChat_gui(const std::string &nick);
		void addFavoriteUser_gui(dcpp::StringMap params);
		void removeFavoriteUser_gui(dcpp::StringMap params);
		void addProtected_gui(dcpp::StringMap params);

		void addPrivateMessage_gui(Msg::TypeMsg typemsg, std::string nick, std::string cid, std::string url, std::string message, bool useSetting);

//[BBCodes
		void loadImage_gui(std::string target, std::string tth);
		void openImage_gui(std::string target);
//
		void set_Header_tooltip_gui();
		void columnHeader(int num, std::string name);

		// GUI callbacks
		static void onSizeWindowState_gui(GtkWidget* widget,GdkRectangle *allocation,gpointer data);
//		static gboolean onNickTagEvent_gui(GtkTextTag *tag, GObject *textView, GdkEvent *event, GtkTextIter *iter, gpointer data);
//		static gboolean onLinkTagEvent_gui(GtkTextTag *tag, GObject *textView, GdkEvent *event, GtkTextIter *iter, gpointer data);
//		static gboolean onHubTagEvent_gui(GtkTextTag *tag, GObject *textView, GdkEvent *event, GtkTextIter *iter, gpointer data);
//		static gboolean onMagnetTagEvent_gui(GtkTextTag *tag, GObject *textView, GdkEvent *event, GtkTextIter *iter, gpointer data);
//		static gboolean onEmotButtonRelease_gui(GtkWidget *widget, GdkEventButton *event, gpointer data);
		static void onChatScroll_gui(GtkAdjustment *adjustment, gpointer data);
		static void onChatResize_gui(GtkAdjustment *adjustment, gpointer data);
		static void onSendMessage_gui(GtkEntry *entry, gpointer data);

		static void onNickToChat_gui(GtkWidget *item,GVariant*, gpointer data);
		static void onBrowseItemClicked_gui(GtkWidget *item,GVariant*, gpointer data);
		static void onMatchItemClicked_gui(GtkWidget *item,GVariant*, gpointer data);
		static void onMsgItemClicked_gui(GtkWidget *item,GVariant*, gpointer data);
		static void onGrantItemClicked_gui(GtkWidget *item,GVariant*, gpointer data);
		static void onRemoveUserItemClicked_gui(GtkWidget *item,GVariant* v, gpointer data);
//		static void onCopyURIClicked_gui(GtkWidget *item,GVariant* vg, gpointer data);
//		static void onOpenLinkClicked_gui(GtkWidget *item, GVariant* v, gpointer data);
//		static void onOpenHubClicked_gui(GtkWidget*,GVariant* v, gpointer data);
//		static void onSearchMagnetClicked_gui(GtkWidget*,GVariant* v, gpointer data);
//		static void onMagnetPropertiesClicked_gui(GtkWidget*,GVariant* v, gpointer data);
		static void onUserListToggled_gui(GtkWidget *widget, gpointer data);
		static void onAddFavoriteUserClicked_gui(GtkWidget*item,GVariant* v, gpointer data);
		static void onRemoveFavoriteUserClicked_gui(GtkWidget *item,GVariant* v, gpointer data);
		static void onPasswordDialog(GtkWidget *dialog, gint response, gpointer data);
//		static void onDownloadToClicked_gui(GtkWidget*,GVariant* v, gpointer data);
//		static void onDownloadClicked_gui(GtkWidget*,GVariant* v, gpointer data);
		static void onCommandClicked_gui(GtkWidget *widget, gpointer data);
		static void onUseEmoticons_gui(GtkWidget *widget, gpointer data);
		static void onCloseItem(GtkWidget* ,GVariant*, gpointer data);
		static void onCopyHubUrl(GtkWidget* ,GVariant*, gpointer data);
		static void onAddFavItem(GtkWidget* ,GVariant*, gpointer data);
		static void onRemoveFavHub(GtkWidget* ,GVariant*, gpointer data);
		static void onSetTabText(GtkWidget* ,GVariant*, gpointer data);
		static void onShareView(GtkWidget* ,GVariant*, gpointer data);
		static void onReconnectItemTab(GtkWidget* ,GVariant*, gpointer data);
		static void onRefreshShare(GtkWidget* ,GVariant*, gpointer data);
		static void onAddIgnoreUserItemClicked_gui(GtkWidget*,GVariant* v, gpointer data);
		static void onRemoveIgnoreUserItemClicked_gui(GtkWidget*,GVariant* v, gpointer data);
		static void onShowReportClicked_gui(GtkWidget*,GVariant* v, gpointer data);
		static void selection_changed_userlist_gui(GtkTreeSelection *selection, GtkWidget *tree_view);
		static gboolean onUserListTooltip_gui(GtkWidget *widget, gint x, gint y, gboolean keyboard_tip, GtkTooltip *_tooltip, gpointer data);
		static gboolean onIpTagEvent_gui(GtkTextTag *tag, GObject *textView, GdkEvent *event , GtkTextIter *iter, gpointer data);
		static void onCopyIpItem_gui(GtkWidget *wid, gpointer data);
		static void onRipeDbItem_gui(GtkWidget *wid, gpointer data);
		static void onTestSURItemClicked_gui(GtkWidget*,GVariant* v, gpointer data);
		static void onCheckFLItemClicked_gui(GtkWidget*,GVariant* v, gpointer data);
		static void onUnProtectUserClicked_gui(GtkWidget*,GVariant* v, gpointer data);
		static void onProtectUserClicked_gui(GtkWidget*,GVariant* v, gpointer data);
		static void onRefreshUserListClicked_gui(GtkWidget *wid, gpointer data);
		static void on_setImage_tab(GtkButton *widget, gpointer data);
		static void onToglleButtonIcon(GtkToggleButton *button, gpointer data);
		//[colorize userlist
		static void makeColor(GtkTreeViewColumn *column, GtkCellRenderer *cell, GtkTreeModel *model, GtkTreeIter *iter,gpointer data);
		static gint sort_iter_compare_func_nick(GtkTreeModel *model, GtkTreeIter  *a,GtkTreeIter  *b,  gpointer  data);

		static gboolean key_pressed_gui ( GtkEventControllerKey* self,	guint keyval,
  							guint keycode,	GdkModifierType state,	gpointer user_data	);
		static void key_released_gui (  GtkEventControllerKey* self,  guint keyval,
  							guint keycode,  GdkModifierType state,  gpointer user_data  );
		static void on_right_btn_pressed (GtkGestureClick *gesture, int       n_press,
                                   double             x,
                                   double             y,
                                   gpointer         *data);

		static void on_right_btn_released (GtkGestureClick *gesture,int       n_press,
                                    double           x,
                                    double           y,
                                    GtkWidget       *widget);

		static void onResponseSetText(GtkDialog *dialog,
                    int        response,
                    gpointer   data);
		static void Hub::onResponseSetTextIcon(GtkDialog *dialog,
                    int        response,
                    gpointer   data);

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
		std::string realFile_client(std::string tth);
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

		UserMap userMap;
		UnMapIter userIters;
		UserMap userFavoriteMap;
		IgnoreMenu* ignoreMenu;
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
		GtkWidget *TabEntry;
		Tag::TypeTag tagMsg;
		unsigned int historyIndex;
		static const int maxLines = 500;
		int64_t totalShared;
		int width;
		gint totalEmoticons;
		gint oldType;
		bool scrollToBottom;
		bool PasswordDialog;
		bool WaitingPassword;
		bool useEmoticons;
		bool notCreated;
		bool isFavBool;
		static const GActionEntry hub_entries[];

};
#else
class Hub;
#endif
