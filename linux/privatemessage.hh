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

#ifndef _BMDC_PRIVATE_MESSAGE_HH
#define _BMDC_PRIVATE_MESSAGE_HH

#include "../dcpp/stdinc.h"
#include "../dcpp/DCPlusPlus.h"
#include "../dcpp/ClientManagerListener.h"
#include "bookentry.hh"
#include "message.hh"
#include "UserCommandMenu.hh"
#include <vector>
#include "../dcpp/Flags.h"
#include "../dcpp/UserManager.h"

class WulforSettingsManager;
class EmoticonsDialog;

class PrivateMessage:
	public BookEntry,
	public dcpp::UsersManagerListener,
	public dcpp::Flags
{
	public:
		PrivateMessage(const std::string &cid, const std::string &hubUrl);
		virtual ~PrivateMessage();
		virtual void show();
		GMenu *createmenu() override;

		// GUI functions
		void addMessage_gui(std::string message, Msg::TypeMsg typemsg);
		void addStatusMessage_gui(std::string message, Msg::TypeMsg typemsg);
		void preferences_gui();
		void sendMessage_p(std::string message) { sendMessage_client(message); }
		bool getIsOffline() const { return isSet(OFFLINE); }

	private:
		using dcpp::UsersManagerListener::on;
		//@ Status of PM's user
		typedef enum
		{
			NORMAL = 0,
			OFFLINE = 1,
			BOT = 2,
		} UserStatus;
	
		// GUI functions
		void setStatus_gui(std::string text);
		void addLine_gui(Msg::TypeMsg typemsg, const std::string &line);
		void applyTags_gui(const std::string &line);
		void applyEmoticons_gui();
		void getSettingTag_gui(WulforSettingsManager *wsm, Tag::TypeTag type, std::string &fore, std::string &back, bool &bold, bool &italic);
		GtkTextTag* createTag_gui(const std::string &tagname, Tag::TypeTag type);
		void updateCursor(GtkWidget *widget);
		void updateOnlineStatus_gui(bool online);
		void readLog(const std::string& logPath, const unsigned setting);

		static void on_right_btn_pressed (GtkGestureClick *gesture, int       n_press,
                                   double             x,
                                   double             y,
                                   gpointer         *data);

		static void on_right_btn_released (GtkGestureClick *gesture,int       n_press,
                                    double           x,
                                    double           y,
                                    gpointer *widget);
		static gboolean key_pressed_gui ( GtkEventControllerKey* self,	guint keyval,
  							guint keycode,	GdkModifierType state,	gpointer user_data	);
		static void key_released_gui (  GtkEventControllerKey* self,  guint keyval,
  							guint keycode,  GdkModifierType state,  gpointer user_data  );

		// GUI callbacks
		/*
		static gboolean onLinkTagEvent_gui(GtkTextTag *tag, GObject *textView, GdkEvent *event, GtkTextIter *iter, gpointer data);
		static gboolean onHubTagEvent_gui(GtkTextTag *tag, GObject *textView, GdkEvent *event, GtkTextIter *iter, gpointer data);
		static gboolean onMagnetTagEvent_gui(GtkTextTag *tag, GObject *textView, GdkEvent *event, GtkTextIter *iter, gpointer data);
		static gboolean onIpTagEvent_gui(GtkTextTag *tag, GObject *textView, GdkEvent *event , GtkTextIter *iter, gpointer data);//BMDC
		static gboolean onEmotButtonRelease_gui(GtkWidget *widget, GdkEventButton *event, gpointer data);*/
		static void onSendMessage_gui(GtkEntry *entry, gpointer data);
		/*
		static void onCopyURIClicked_gui(GtkWidget* widget, GVariant* var,  gpointer data);
		static void onOpenLinkClicked_gui(GtkWidget* widget , GVariant* var , gpointer data);
		static void onOpenHubClicked_gui(GtkWidget* widget , GVariant* var ,  gpointer data);
		static void onSearchMagnetClicked_gui(GtkWidget* widget , GVariant* var ,  gpointer data);
		static void onMagnetPropertiesClicked_gui(GtkWidget* widget , GVariant* var ,  gpointer data);
		static void onDownloadToClicked_gui(GtkWidget* widget , GVariant* var ,  gpointer data);
		static void onDownloadClicked_gui(GtkWidget* widget , GVariant* var ,  gpointer data);
		static void onUseEmoticons_gui(GtkWidget* widget , GVariant* var ,  gpointer data);
*/
		static void onCopyIpItem_gui(GtkWidget* widget , GVariant* var , gpointer data);
/*		static void onRipeDbItem_gui(GtkWidget* widget , GVariant* var ,  gpointer data);
*/
		static void onCopyCID(GtkWidget* wid , GVariant* var , gpointer data);
		static void onAddFavItem(GtkWidget* wid , GVariant* var , gpointer data);
		static void onDeleteFavItem(GtkWidget* wid , GVariant* var , gpointer data){};
		static void onCopyNicks(GtkWidget* wid , GVariant* var , gpointer data);
//		void setImageButton(const std::string country);

		// Client functions
		void sendMessage_client(std::string message);
		void addFavoriteUser_client();
		void removeFavoriteUser_client();
		void getFileList_client();
		void grantSlot_client();

		// client callback
		virtual void on(dcpp::UsersManagerListener::UserConnected, const dcpp::UserPtr& aUser) noexcept;
		virtual void on(dcpp::UsersManagerListener::UserDisconnected, const dcpp::UserPtr& aUser) noexcept;

		GtkTextBuffer *messageBuffer;
		GtkTextMark *mark, *start_mark, *end_mark, *tag_mark, *emot_mark;
		std::string cid;
		std::string hubUrl;
		std::vector<std::string> history;
		int historyIndex;
		bool sentAwayMessage;
		static const int maxLines = 500; ///@todo: make these preferences
		GdkCursor* handCursor;
		std::string selectedTagStr;
		GtkTextTag* selectedTag;
		bool scrollToBottom;
		GtkTextTag *TagsMap[Tag::TAG_LAST];
		Tag::TypeTag tagMsg, tagNick;
		bool useEmoticons;
		gint totalEmoticons;
		EmoticonsDialog *emotdialog;
		UserCommandMenu *userCommandMenu;
		bool notCreated;
		static const GActionEntry pm_entries[];

};

#else
class PrivateMessage;
#endif
