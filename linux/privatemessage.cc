/*
 * Copyright © 2004-2013 Jens Oknelid, paskharen@gmail.com
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

#include "privatemessage.hh"

#include <dcpp/ClientManager.h>
#include <dcpp/FavoriteManager.h>
#include <dcpp/StringTokenizer.h>
#include <dcpp/PluginManager.h>
#include "settingsmanager.hh"
#include "emoticonsdialog.hh"
#include "emoticons.hh"
#include "wulformanager.hh"
#include "WulforUtil.hh"
#include "search.hh"
#include "sound.hh"

using namespace std;
using namespace dcpp;

PrivateMessage::PrivateMessage(const string &cid, const string &hubUrl):
	BookEntry(Entry::PRIVATE_MESSAGE, _("PM: ") + WulforUtil::getNicks(cid, hubUrl), "privatemessage.glade", cid),//NOTE: core 0.760
	cid(cid),
	hubUrl(hubUrl),
	historyIndex(0),
	sentAwayMessage(FALSE),
	scrollToBottom(TRUE),
	offline(false)
{
	// Intialize the chat window
	if (SETTING(USE_OEM_MONOFONT))
	{
		PangoFontDescription *fontDesc = pango_font_description_new();
		pango_font_description_set_family(fontDesc, "Mono");
		gtk_widget_override_font(getWidget("text"), fontDesc);
		pango_font_description_free(fontDesc);
	}

	//..set Colors
	string strcolor = WGETS("background-color-chat");
	GdkRGBA color;
	gdk_rgba_parse(&color,strcolor.c_str());
	gtk_widget_override_background_color(getWidget("text"),GTK_STATE_FLAG_NORMAL,&color);
	gtk_widget_override_background_color(getWidget("text"),GTK_STATE_FLAG_PRELIGHT,&color);
	gtk_widget_override_background_color(getWidget("text"),GTK_STATE_FLAG_ACTIVE,&color);
	gtk_widget_override_background_color(getWidget("text"),GTK_STATE_FLAG_INSENSITIVE,&color);

	// the reference count on the buffer is not incremented and caller of this function won't own a new reference.
	messageBuffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(getWidget("text")));

	/* initial markers */
	GtkTextIter iter;
	gtk_text_buffer_get_end_iter(messageBuffer, &iter);

	mark = gtk_text_buffer_create_mark(messageBuffer, NULL, &iter, FALSE);
	start_mark = gtk_text_buffer_create_mark(messageBuffer, NULL, &iter, TRUE);
	end_mark = gtk_text_buffer_create_mark(messageBuffer, NULL, &iter, TRUE);
	tag_mark = gtk_text_buffer_create_mark(messageBuffer, NULL, &iter, FALSE);
	emot_mark = gtk_text_buffer_create_mark(messageBuffer, NULL, &iter, TRUE);

	handCursor = gdk_cursor_new(GDK_HAND2);

	GtkAdjustment *adjustment = gtk_scrolled_window_get_vadjustment(GTK_SCROLLED_WINDOW(getWidget("scroll")));

	// menu
	g_object_ref_sink(getWidget("magnetMenu"));
	g_object_ref_sink(getWidget("linkMenu"));
	g_object_ref_sink(getWidget("hubMenu"));
	g_object_ref_sink(getWidget("chatCommandsMenu"));

	userCommandMenu = new UserCommandMenu(BookEntry::createmenu(), ::UserCommand::CONTEXT_USER);
	addChild(userCommandMenu);

	// Emoticons dialog
	emotdialog = new EmoticonsDialog(getWidget("entry"), getWidget("emotButton"), getWidget("emotMenu"));
	if (!WGETB("emoticons-use"))
		gtk_widget_set_sensitive(getWidget("emotButton"), FALSE);
	useEmoticons = TRUE;

	// PM commands
	g_object_set_data_full(G_OBJECT(getWidget("awayCommandItem")), "command", g_strdup("/away"), g_free);
	g_signal_connect(getWidget("awayCommandItem"), "activate", G_CALLBACK(onCommandClicked_gui), (gpointer)this);

	g_object_set_data_full(G_OBJECT(getWidget("backCommandItem")), "command", g_strdup("/back"), g_free);
	g_signal_connect(getWidget("backCommandItem"), "activate", G_CALLBACK(onCommandClicked_gui), (gpointer)this);

	g_object_set_data_full(G_OBJECT(getWidget("clearCommandItem")), "command", g_strdup("/clear"), g_free);
	g_signal_connect(getWidget("clearCommandItem"), "activate", G_CALLBACK(onCommandClicked_gui), (gpointer)this);

	g_object_set_data_full(G_OBJECT(getWidget("closeCommandItem")), "command", g_strdup("/close"), g_free);
	g_signal_connect(getWidget("closeCommandItem"), "activate", G_CALLBACK(onCommandClicked_gui), (gpointer)this);

	g_object_set_data_full(G_OBJECT(getWidget("fuserCommandItem")), "command", g_strdup("/fuser"), g_free);
	g_signal_connect(getWidget("fuserCommandItem"), "activate", G_CALLBACK(onCommandClicked_gui), (gpointer)this);

	g_object_set_data_full(G_OBJECT(getWidget("rmfuCommandItem")), "command", g_strdup("/rmfu"), g_free);
	g_signal_connect(getWidget("rmfuCommandItem"), "activate", G_CALLBACK(onCommandClicked_gui), (gpointer)this);

	g_object_set_data_full(G_OBJECT(getWidget("helpCommandItem")), "command", g_strdup("/help"), g_free);
	g_signal_connect(getWidget("helpCommandItem"), "activate", G_CALLBACK(onCommandClicked_gui), (gpointer)this);

	g_object_set_data_full(G_OBJECT(getWidget("getlistCommandItem")), "command", g_strdup("/getlist"), g_free);
	g_signal_connect(getWidget("getlistCommandItem"), "activate", G_CALLBACK(onCommandClicked_gui), (gpointer)this);

	g_object_set_data_full(G_OBJECT(getWidget("grantCommandItem")), "command", g_strdup("/grant"), g_free);
	g_signal_connect(getWidget("grantCommandItem"), "activate", G_CALLBACK(onCommandClicked_gui), (gpointer)this);

	// chat commands button
	g_signal_connect(getWidget("chatCommandsButton"), "button-release-event", G_CALLBACK(onChatCommandButtonRelease_gui), (gpointer)this);

	// Connect the signals to their callback functions.
	g_signal_connect(getContainer(), "focus-in-event", G_CALLBACK(onFocusIn_gui), (gpointer)this);
	g_signal_connect(getWidget("entry"), "activate", G_CALLBACK(onSendMessage_gui), (gpointer)this);
	g_signal_connect(getWidget("entry"), "key-press-event", G_CALLBACK(onKeyPress_gui), (gpointer)this);
	g_signal_connect(getWidget("text"), "motion-notify-event", G_CALLBACK(onChatPointerMoved_gui), (gpointer)this);
	g_signal_connect(getWidget("text"), "visibility-notify-event", G_CALLBACK(onChatVisibilityChanged_gui), (gpointer)this);
	g_signal_connect(getWidget("text"), "draw", G_CALLBACK(expose), (gpointer)this);
	g_signal_connect(adjustment, "value_changed", G_CALLBACK(onChatScroll_gui), (gpointer)this);
	g_signal_connect(adjustment, "changed", G_CALLBACK(onChatResize_gui), (gpointer)this);
	g_signal_connect(getWidget("copyLinkItem"), "activate", G_CALLBACK(onCopyURIClicked_gui), (gpointer)this);
	g_signal_connect(getWidget("openLinkItem"), "activate", G_CALLBACK(onOpenLinkClicked_gui), (gpointer)this);
	g_signal_connect(getWidget("copyhubItem"), "activate", G_CALLBACK(onCopyURIClicked_gui), (gpointer)this);
	g_signal_connect(getWidget("openhubItem"), "activate", G_CALLBACK(onOpenHubClicked_gui), (gpointer)this);
	g_signal_connect(getWidget("copyMagnetItem"), "activate", G_CALLBACK(onCopyURIClicked_gui), (gpointer)this);
	g_signal_connect(getWidget("searchMagnetItem"), "activate", G_CALLBACK(onSearchMagnetClicked_gui), (gpointer)this);
	g_signal_connect(getWidget("magnetPropertiesItem"), "activate", G_CALLBACK(onMagnetPropertiesClicked_gui), (gpointer)this);
	g_signal_connect(getWidget("emotButton"), "button-release-event", G_CALLBACK(onEmotButtonRelease_gui), (gpointer)this);
	g_signal_connect(getWidget("downloadBrowseItem"), "activate", G_CALLBACK(onDownloadToClicked_gui), (gpointer)this);
	g_signal_connect(getWidget("downloadItem"), "activate", G_CALLBACK(onDownloadClicked_gui), (gpointer)this);

	g_signal_connect(getWidget("ripeitem"), "activate", G_CALLBACK(onRipeDbItem_gui),(gpointer)this);
	g_signal_connect(getWidget("copyipItem"), "activate", G_CALLBACK(onCopyIpItem_gui),(gpointer)this);

	gtk_widget_grab_focus(getWidget("entry"));
	history.push_back("");
	OnlineUser* user = ClientManager::getInstance()->findOnlineUser(CID(cid), hubUrl);
	isBot = user ? user->getIdentity().isBot() : FALSE;

	setLabel_gui(WulforUtil::getNicks(cid, hubUrl) + " [" + WulforUtil::getHubNames(cid, hubUrl) + "]");

	/* initial tags map */
	TagsMap[Tag::TAG_PRIVATE] = createTag_gui("TAG_PRIVATE", Tag::TAG_PRIVATE);
	TagsMap[Tag::TAG_MYOWN] = createTag_gui("TAG_MYOWN", Tag::TAG_MYOWN);
	TagsMap[Tag::TAG_SYSTEM] = createTag_gui("TAG_SYSTEM", Tag::TAG_SYSTEM);
	TagsMap[Tag::TAG_STATUS] = createTag_gui("TAG_STATUS", Tag::TAG_STATUS);
	TagsMap[Tag::TAG_TIMESTAMP] = createTag_gui("TAG_TIMESTAMP", Tag::TAG_TIMESTAMP);
	/*-*/
	TagsMap[Tag::TAG_MYNICK] = createTag_gui("TAG_MYNICK", Tag::TAG_MYNICK);
	TagsMap[Tag::TAG_NICK] = createTag_gui("TAG_NICK", Tag::TAG_NICK);
	TagsMap[Tag::TAG_OPERATOR] = createTag_gui("TAG_OPERATOR", Tag::TAG_OPERATOR);
	TagsMap[Tag::TAG_FAVORITE] = createTag_gui("TAG_FAVORITE", Tag::TAG_FAVORITE);
	TagsMap[Tag::TAG_URL] = createTag_gui("TAG_URL", Tag::TAG_URL);
	TagsMap[Tag::TAG_IPADR] = createTag_gui("TAG_IPADR", Tag::TAG_IPADR);

	// set default select tag (fix error show cursor in neutral space)
	selectedTag = TagsMap[Tag::TAG_PRIVATE];

	dcpp::ParamMap params;
	params["hubNI"] = WulforUtil::getHubNames(cid, hubUrl);//NOTE: core 0.762
	params["hubURL"] = hubUrl;
	params["userCID"] = cid;
	params["userNI"] = ClientManager::getInstance()->getNicks(CID(cid), hubUrl)[0];//NOTE: core 0.762
	params["myCID"] = ClientManager::getInstance()->getMe()->getCID().toBase32();

	readLog(LogManager::getInstance()->getPath(LogManager::PM, params)
		,(unsigned int)SETTING(PM_LAST_LOG_LINES));
}

PrivateMessage::~PrivateMessage()
{
	ClientManager::getInstance()->removeListener(this);

	if (handCursor)
	{
		g_object_unref(handCursor);
		handCursor = NULL;
	}

	g_object_unref(getWidget("magnetMenu"));
	g_object_unref(getWidget("linkMenu"));
	g_object_unref(getWidget("hubMenu"));
	g_object_unref(getWidget("chatCommandsMenu"));

	delete emotdialog;
}

void PrivateMessage::show()
{
	ClientManager::getInstance()->addListener(this);
}

void PrivateMessage::addMessage_gui(string message, Msg::TypeMsg typemsg)
{
	addLine_gui(typemsg, message);

	if (SETTING(LOG_PRIVATE_CHAT))
	{
		dcpp::ParamMap params;
		params["message"] = message;
		params["hubNI"] = WulforUtil::getHubNames(cid, hubUrl);//NOTE: core 0.762
		params["hubURL"] = hubUrl;
		params["userCID"] = cid;
		params["userNI"] = ClientManager::getInstance()->getNicks(CID(cid), hubUrl)[0];//NOTE: core 0.762
		params["myCID"] = ClientManager::getInstance()->getMe()->getCID().toBase32();
		LOG(LogManager::PM, params);
	}

	if (SETTING(BOLD_PM))
		setUrgent_gui();

	// Send an away message, but only the first time after setting away mode.
	if (!Util::getAway())
	{
		sentAwayMessage = FALSE;
	}
	else if (!sentAwayMessage && !(SETTING(NO_AWAYMSG_TO_BOTS) && isBot))
	{
		/*What away message to send*/
		auto what = [this](ParamMap& params) -> std::string {
				string defAway = Util::getAwayMessage(params);
				if(hubUrl.empty())
					return defAway;
			return FavoriteManager::getInstance()->getAwayMessage(hubUrl, params);
		};

		ParamMap params;
		params["message"] = message;
		params["hubNI"] = WulforUtil::getHubNames(cid, hubUrl);//NOTE: core 0.762
		params["hubURL"] = hubUrl;
		params["userCID"] = cid;
		params["userNI"] = ClientManager::getInstance()->getNicks(CID(cid), hubUrl)[0];//NOTE: core 0.762
		params["myCID"] = ClientManager::getInstance()->getMe()->getCID().toBase32();
		/**/

		sentAwayMessage = TRUE;
		typedef Func1<PrivateMessage, string> F1;
		F1 *func = new F1(this, &PrivateMessage::sendMessage_client, what(params) );
		WulforManager::get()->dispatchClientFunc(func);
	}

	if (WGETB("sound-pm"))
	{
		MainWindow *mw = WulforManager::get()->getMainWindow();
		GdkWindowState state = gdk_window_get_state(gtk_widget_get_window(mw->getContainer()));

		if ((state & GDK_WINDOW_STATE_ICONIFIED) || mw->currentPage_gui() != getContainer())
			Sound::get()->playSound(Sound::PRIVATE_MESSAGE);
		else if (WGETB("sound-pm-open")) Sound::get()->playSound(Sound::PRIVATE_MESSAGE);
	}
}

void PrivateMessage::addStatusMessage_gui(string message, Msg::TypeMsg typemsg)
{
	setStatus_gui(message);
	addLine_gui(typemsg, "*** " + message);
}

void PrivateMessage::preferences_gui()
{
	WulforSettingsManager *wsm = WulforSettingsManager::getInstance();
	string fore, back;
	int bold, italic;

	for (int i = Tag::TAG_FIRST; i < Tag::TAG_LAST; i++)
	{
		if(i == Tag::TAG_GENERAL)
            		continue;
        	if(i == Tag::TAG_CHEAT)
			continue;

		getSettingTag_gui(wsm, (Tag::TypeTag)i, fore, back, bold, italic);

		g_object_set(TagsMap[i],
			"foreground", fore.c_str(),
			"background", back.c_str(),
			"weight", bold ? TEXT_WEIGHT_BOLD : TEXT_WEIGHT_NORMAL,
			"style", italic ? TEXT_STYLE_ITALIC : TEXT_STYLE_NORMAL,
			NULL);
	}

	gtk_widget_queue_draw(getWidget("emotButton"));

	if (!WGETB("emoticons-use"))
	{
		if (gtk_widget_is_sensitive(getWidget("emotButton")))
			gtk_widget_set_sensitive(getWidget("emotButton"), FALSE);
	}
	else if (!gtk_widget_is_sensitive(getWidget("emotButton")))
	{
		gtk_widget_set_sensitive(getWidget("emotButton"), TRUE);
	}
	string strcolor = WGETS("background-color-chat");
	GdkRGBA color;
	gdk_rgba_parse(&color,strcolor.c_str());
	gtk_widget_override_background_color(getWidget("text"),GTK_STATE_FLAG_NORMAL,&color);
	gtk_widget_override_background_color(getWidget("text"),GTK_STATE_FLAG_PRELIGHT,&color);
	gtk_widget_override_background_color(getWidget("text"),GTK_STATE_FLAG_ACTIVE,&color);
	gtk_widget_override_background_color(getWidget("text"),GTK_STATE_FLAG_INSENSITIVE,&color);

	gtk_widget_queue_draw(getWidget("text"));
}

void PrivateMessage::setStatus_gui(string text)
{
	if (!text.empty())
	{
		text = "[" + Util::getShortTimeString() + "] " + text;
		gtk_statusbar_pop(GTK_STATUSBAR(getWidget("status")), 0);
		gtk_statusbar_push(GTK_STATUSBAR(getWidget("status")), 0, text.c_str());
	}
}

void PrivateMessage::addLine_gui(Msg::TypeMsg typemsg, const string &message)
{
	if (message.empty())
		return;

	GtkTextIter iter;
	string line = "";

	if (SETTING(TIME_STAMPS))
		line += "[" + Util::getShortTimeString() + "] ";

	line += message + "\n";

	gtk_text_buffer_get_end_iter(messageBuffer, &iter);
	gtk_text_buffer_insert(messageBuffer, &iter, line.c_str(), line.size());

	switch (typemsg)
	{
		case Msg::MYOWN:

			tagMsg = Tag::TAG_MYOWN;
			tagNick = Tag::TAG_MYNICK;
		break;

		case Msg::SYSTEM:

			tagMsg = Tag::TAG_SYSTEM;
			tagNick = Tag::TAG_NICK;
		break;

		case Msg::STATUS:

			tagMsg = Tag::TAG_STATUS;
			tagNick = Tag::TAG_NICK;
		break;

		case Msg::OPERATOR:

			tagMsg = Tag::TAG_PRIVATE;
			tagNick = Tag::TAG_OPERATOR;
		break;

		case Msg::FAVORITE:

			tagMsg = Tag::TAG_PRIVATE;
			tagNick = Tag::TAG_FAVORITE;
		break;

		case Msg::PRIVATE:

		default:

			tagMsg = Tag::TAG_PRIVATE;
			tagNick = Tag::TAG_NICK;
	}

	totalEmoticons = 0;

	applyTags_gui(line);

	gtk_text_buffer_get_end_iter(messageBuffer, &iter);

	// Limit size of chat text
	if (gtk_text_buffer_get_line_count(messageBuffer) > maxLines + 1)
	{
		GtkTextIter next;
		gtk_text_buffer_get_start_iter(messageBuffer, &iter);
		gtk_text_buffer_get_iter_at_line(messageBuffer, &next, 1);
		gtk_text_buffer_delete(messageBuffer, &iter, &next);
	}
}

void PrivateMessage::applyTags_gui(const string &line)
{
	GtkTextIter start_iter;
	gtk_text_buffer_get_end_iter(messageBuffer, &start_iter);

	string::size_type begin = 0;

	// apply timestamp tag
	if (SETTING(TIME_STAMPS))
	{
		string ts = Util::getShortTimeString();
		gtk_text_iter_backward_chars(&start_iter, g_utf8_strlen(line.c_str(), -1) - g_utf8_strlen(ts.c_str(), -1) - 2);

		GtkTextIter ts_start_iter, ts_end_iter;
		ts_end_iter = start_iter;

		gtk_text_buffer_get_end_iter(messageBuffer, &ts_start_iter);
		gtk_text_iter_backward_chars(&ts_start_iter, g_utf8_strlen(line.c_str(), -1));

		gtk_text_buffer_apply_tag(messageBuffer, TagsMap[Tag::TAG_TIMESTAMP], &ts_start_iter, &ts_end_iter);

		begin = ts.size() + 2 + 1;
	}
	else
		gtk_text_iter_backward_chars(&start_iter, g_utf8_strlen(line.c_str(), -1));

	dcassert(begin < line.size());

	// apply nick tag
	if (line[begin] == '<')
	{
		string::size_type end = line.find_first_of('>', begin);

		if (end != string::npos)
		{
			GtkTextIter nick_start_iter, nick_end_iter;

			gtk_text_buffer_get_end_iter(messageBuffer, &nick_start_iter);
			gtk_text_buffer_get_end_iter(messageBuffer, &nick_end_iter);

			gtk_text_iter_backward_chars(&nick_start_iter, g_utf8_strlen(line.c_str() + begin, -1));
			gtk_text_iter_backward_chars(&nick_end_iter, g_utf8_strlen(line.c_str() + end, -1) - 1);

			dcassert(tagNick >= Tag::TAG_MYNICK && tagNick < Tag::TAG_IPADR);
			gtk_text_buffer_apply_tag(messageBuffer, TagsMap[tagNick], &nick_start_iter, &nick_end_iter);

			start_iter = nick_end_iter;
		}
	}

	// apply tags: link, hub-url, magnet
	GtkTextIter tag_start_iter, tag_end_iter;

	gtk_text_buffer_move_mark(messageBuffer, start_mark, &start_iter);
	gtk_text_buffer_move_mark(messageBuffer, end_mark, &start_iter);

	string tagName;
	bool start = FALSE;
	bool isIp = false;
	for(;;)
	{
		do {
			gunichar ch = gtk_text_iter_get_char(&start_iter);

			if (!g_unichar_isspace(ch))
				break;

		} while (gtk_text_iter_forward_char(&start_iter));

		if(!start)
		{
			gtk_text_buffer_move_mark(messageBuffer, start_mark, &start_iter);
			gtk_text_buffer_move_mark(messageBuffer, end_mark, &start_iter);

			start = TRUE;
		}

		tag_start_iter = start_iter;

		for(;gtk_text_iter_forward_char(&start_iter);)
		{
			gunichar ch = gtk_text_iter_get_char(&start_iter);

			if (g_unichar_isspace(ch))
				break;
		}

		tag_end_iter = start_iter;

		GCallback callback = NULL;
		gchar *temp = gtk_text_iter_get_text(&tag_start_iter, &tag_end_iter);
		string country_text;
		bool isCountryFlag = FALSE;

		if(WGETB("use-highlighting"))
		{
			GtkTextTag *tag = gtk_text_tag_table_lookup(gtk_text_buffer_get_tag_table(messageBuffer), temp);
			bool isTab = false;
			if(WulforUtil::isHighlightingWorld(messageBuffer,tag,string(temp),isTab,(gpointer)NULL))
			{
				gtk_text_buffer_apply_tag(messageBuffer, tag, &tag_start_iter, &tag_end_iter);
				if(isTab)
				{
					typedef Func0<PrivateMessage> F0;
					F0 *func = new F0(this, &PrivateMessage::setUrgent_gui);
					WulforManager::get()->dispatchGuiFunc(func);
				}
			}
		}

		if (!C_EMPTY(temp))
		{
			tagName = temp;
			bool notlink = FALSE;

			if(g_ascii_strncasecmp(tagName.c_str(), "[ccc]", 5) == 0)
			{
				string::size_type i = tagName.rfind("[/ccc]");
				if (i != string::npos)
				{
					country_text = tagName.substr(5, i - 5);
					if(country_text.length() == 2 )
					{
							notlink = isCountryFlag = TRUE;
					}
                		}
            	}

			if(!notlink)
			{
				if (WulforUtil::isLink(tagName))
					callback = G_CALLBACK(onLinkTagEvent_gui);
				else if (WulforUtil::isHubURL(tagName))
					callback = G_CALLBACK(onHubTagEvent_gui);
				else if (WulforUtil::isMagnet(tagName))
					callback = G_CALLBACK(onMagnetTagEvent_gui);
			}

			if(WulforUtil::HitIP(tagName,ip))
			{
				callback = G_CALLBACK(onIpTagEvent_gui);
//				tagStyle = Tag::TAG_IPADR;
				isIp = true;
				userCommandMenu->cleanMenu_gui();
				userCommandMenu->addIp(ip);
				userCommandMenu->addHub(cid);
				userCommandMenu->buildMenu_gui();
				gtk_widget_show_all(userCommandMenu->getContainer());
			}

		}

		g_free(temp);

		if(isCountryFlag)
		{
            gtk_text_buffer_move_mark(messageBuffer, tag_mark, &tag_end_iter);

            if(country_text.length() == 2)
            {
                GdkPixbuf *buffer = WulforUtil::LoadCountryPixbuf(country_text);
                gtk_text_buffer_delete(messageBuffer, &tag_start_iter, &tag_end_iter);
                GtkTextChildAnchor *anchor = gtk_text_buffer_create_child_anchor(messageBuffer, &tag_start_iter);
                GtkWidget *event_box = gtk_event_box_new();
                // Creating a visible window may cause artifacts that are visible to the user.
                gtk_event_box_set_visible_window(GTK_EVENT_BOX(event_box), FALSE);
                GtkWidget *image = gtk_image_new_from_pixbuf(buffer);
                gtk_container_add(GTK_CONTAINER(event_box),image);
                gtk_text_view_add_child_at_anchor(GTK_TEXT_VIEW(getWidget("text")), event_box, anchor);
                g_signal_connect(G_OBJECT(image), "draw", G_CALLBACK(expose), NULL);

                gtk_widget_show_all(event_box);
                gtk_widget_set_tooltip_text(event_box, country_text.c_str());
            }

			applyEmoticons_gui();

			gtk_text_buffer_get_iter_at_mark(messageBuffer, &start_iter, tag_mark);

			if (gtk_text_iter_is_end(&start_iter))
				return;

			start = FALSE;

			continue;
		}

		if (callback)
		{
			gtk_text_buffer_move_mark(messageBuffer, tag_mark, &tag_end_iter);

			// check for the tags in our buffer
			GtkTextTag *tag = gtk_text_tag_table_lookup(gtk_text_buffer_get_tag_table(messageBuffer), tagName.c_str());

			if (!tag)
			{
				if(isIp)
					tag = gtk_text_buffer_create_tag(messageBuffer, tagName.c_str(), NULL);
				else tag = gtk_text_buffer_create_tag(messageBuffer, tagName.c_str(), "underline", PANGO_UNDERLINE_SINGLE, NULL);
				g_signal_connect(tag, "event", callback, (gpointer)this);
			}

			// apply tags
			if (callback == G_CALLBACK(onMagnetTagEvent_gui) && WGETB("use-magnet-split"))
			{
				string line;

				if (WulforUtil::splitMagnet(tagName, line))
				{
					gtk_text_buffer_delete(messageBuffer, &tag_start_iter, &tag_end_iter);

					gtk_text_buffer_insert_with_tags(messageBuffer, &tag_start_iter,
						line.c_str(), line.size(), tag, TagsMap[Tag::TAG_URL], NULL);
				}
			}
			else
			{
				gtk_text_buffer_apply_tag(messageBuffer, tag, &tag_start_iter, &tag_end_iter);
				gtk_text_buffer_apply_tag(messageBuffer, TagsMap[Tag::TAG_URL], &tag_start_iter, &tag_end_iter);
			}

			applyEmoticons_gui();

			gtk_text_buffer_get_iter_at_mark(messageBuffer, &start_iter, tag_mark);

			if (gtk_text_iter_is_end(&start_iter))
				return;

			start = FALSE;
		}
		else
		{
			if (gtk_text_iter_is_end(&start_iter))
			{
				if (!gtk_text_iter_equal(&tag_start_iter, &tag_end_iter))
					gtk_text_buffer_move_mark(messageBuffer, end_mark, &tag_end_iter);

				applyEmoticons_gui();

				break;
			}

			gtk_text_buffer_move_mark(messageBuffer, end_mark, &tag_end_iter);
		}
	}
}

gboolean PrivateMessage::expose(GtkWidget *widget, cairo_t *event, gpointer data)
{
	GTK_WIDGET_CLASS(GTK_WIDGET_GET_CLASS(widget))->draw(widget, event);
	return true;
}

void PrivateMessage::applyEmoticons_gui()
{
	GtkTextIter start_iter, end_iter;
	gtk_text_buffer_get_iter_at_mark(messageBuffer, &start_iter, start_mark);
	gtk_text_buffer_get_iter_at_mark(messageBuffer, &end_iter, end_mark);

	if(gtk_text_iter_equal(&start_iter, &end_iter))
		return;

	dcassert(tagMsg >= Tag::TAG_PRIVATE && tagMsg < Tag::TAG_TIMESTAMP);
	gtk_text_buffer_apply_tag(messageBuffer, TagsMap[tagMsg], &start_iter, &end_iter);

	/* emoticons */
	if (tagMsg == Tag::TAG_SYSTEM || tagMsg == Tag::TAG_STATUS)
	{
		return;
	}
	else if (!emotdialog->getEmot()->useEmoticons_gui())
	{
		if (WGETB("emoticons-use"))
			setStatus_gui(_(" *** Emoticons not loaded"));
		return;
	}
	else if (!useEmoticons)
	{
		setStatus_gui(_(" *** Emoticons mode off"));
		return;
	}
	else if (totalEmoticons >= EMOTICONS_MAX)
	{
		setStatus_gui(_(" *** Emoticons limit"));
		return;
	}

	bool search;
	gint searchEmoticons = 0;

	GtkTextIter tmp_end_iter,
		match_start,
		match_end,
		p_start,
		p_end;

	Emot::Iter p_it;
	gint set_start, new_start;
	Emot::List &list = emotdialog->getEmot()->getPack_gui();

	/* set start mark */
	gtk_text_buffer_move_mark(messageBuffer, emot_mark, &start_iter);

	for (;;)
	{
		/* get start and end iter positions at marks */
		gtk_text_buffer_get_iter_at_mark(messageBuffer, &start_iter, emot_mark);
		gtk_text_buffer_get_iter_at_mark(messageBuffer, &end_iter, end_mark);

		search = FALSE;
		set_start = gtk_text_iter_get_offset(&end_iter);

		for (Emot::Iter it = list.begin(); it != list.end(); ++it)
		{
			GList *names = (*it)->getNames();

			for (GList *p = names; p != NULL; p = p->next)
			{
				if (gtk_text_iter_forward_search(&start_iter,
					(gchar *)p->data,
					GTK_TEXT_SEARCH_VISIBLE_ONLY,
					&match_start,
					&match_end,
					&end_iter))
				{
					if (!search)
					{
						search = TRUE;
						end_iter = match_start;

						/* set new limit search */
						gtk_text_buffer_get_iter_at_mark(messageBuffer, &tmp_end_iter, end_mark);
						for (int i = 1; !gtk_text_iter_equal(&end_iter, &tmp_end_iter) && i <= Emot::SIZE_NAME;
							gtk_text_iter_forward_chars(&end_iter, 1), i++);

					}

					new_start = gtk_text_iter_get_offset(&match_start);

					if (new_start < set_start)
					{
						set_start = new_start;

						p_start = match_start;
						p_end = match_end;

						p_it = it;

						if (gtk_text_iter_equal(&start_iter, &match_start))
						{
							it = list.end() - 1;
							break;
						}
					}
				}
			}
		}

		if (search)
		{
			if (totalEmoticons >= EMOTICONS_MAX)
			{
				setStatus_gui(_(" *** Emoticons limit"));
				return;
			}

			/* delete text-emoticon and insert pixbuf-emoticon */
			gtk_text_buffer_delete(messageBuffer, &p_start, &p_end);
			gtk_text_buffer_insert_pixbuf(messageBuffer, &p_start, (*p_it)->getPixbuf());

			searchEmoticons++;
			totalEmoticons++;

			/* set emoticon mark to start */
			gtk_text_buffer_move_mark(messageBuffer, emot_mark, &p_start);

			/* check full emoticons */
			gtk_text_buffer_get_iter_at_mark(messageBuffer, &start_iter, start_mark);
			gtk_text_buffer_get_iter_at_mark(messageBuffer, &end_iter, end_mark);

			if (gtk_text_iter_get_offset(&end_iter) - gtk_text_iter_get_offset(&start_iter) == searchEmoticons - 1)
				return;
		}
		else
			return;
	}
}

void PrivateMessage::getSettingTag_gui(WulforSettingsManager *wsm, Tag::TypeTag type, string &fore, string &back, int &bold, int &italic)
{
	switch (type)
	{
		case Tag::TAG_MYOWN:

			fore = wsm->getString("text-myown-fore-color");
			back = wsm->getString("text-myown-back-color");
			bold = wsm->getInt("text-myown-bold");
			italic = wsm->getInt("text-myown-italic");
		break;

		case Tag::TAG_SYSTEM:

			fore = wsm->getString("text-system-fore-color");
			back = wsm->getString("text-system-back-color");
			bold = wsm->getInt("text-system-bold");
			italic = wsm->getInt("text-system-italic");
		break;

		case Tag::TAG_STATUS:

			fore = wsm->getString("text-status-fore-color");
			back = wsm->getString("text-status-back-color");
			bold = wsm->getInt("text-status-bold");
			italic = wsm->getInt("text-status-italic");
		break;

		case Tag::TAG_TIMESTAMP:

			fore = wsm->getString("text-timestamp-fore-color");
			back = wsm->getString("text-timestamp-back-color");
			bold = wsm->getInt("text-timestamp-bold");
			italic = wsm->getInt("text-timestamp-italic");
		break;

		case Tag::TAG_MYNICK:

			fore = wsm->getString("text-mynick-fore-color");
			back = wsm->getString("text-mynick-back-color");
			bold = wsm->getInt("text-mynick-bold");
			italic = wsm->getInt("text-mynick-italic");
		break;

		case Tag::TAG_OPERATOR:

			fore = wsm->getString("text-op-fore-color");
			back = wsm->getString("text-op-back-color");
			bold = wsm->getInt("text-op-bold");
			italic = wsm->getInt("text-op-italic");
		break;

		case Tag::TAG_FAVORITE:

			fore = wsm->getString("text-fav-fore-color");
			back = wsm->getString("text-fav-back-color");
			bold = wsm->getInt("text-fav-bold");
			italic = wsm->getInt("text-fav-italic");
		break;
		case Tag::TAG_IPADR:
			fore = wsm->getString("text-ip-fore-color");
			back = wsm->getString("text-ip-back-color");
			bold = wsm->getInt("text-ip-bold");
			italic = wsm->getInt("text-ip-italic");
		break;
		case Tag::TAG_URL:

			fore = wsm->getString("text-url-fore-color");
			back = wsm->getString("text-url-back-color");
			bold = wsm->getInt("text-url-bold");
			italic = wsm->getInt("text-url-italic");
		break;

		case Tag::TAG_NICK:

			fore = wsm->getString("text-private-fore-color");
			back = wsm->getString("text-private-back-color");
			italic = wsm->getInt("text-private-italic");

			if (wsm->getBool("text-bold-autors"))
				bold = 1;
			else
				bold = 0;
		break;

		case Tag::TAG_PRIVATE:

		default:

			fore = wsm->getString("text-private-fore-color");
			back = wsm->getString("text-private-back-color");
			bold = wsm->getInt("text-private-bold");
			italic = wsm->getInt("text-private-italic");
	}
}

GtkTextTag* PrivateMessage::createTag_gui(const string &tagname, Tag::TypeTag type)
{
	WulforSettingsManager *wsm = WulforSettingsManager::getInstance();
	GtkTextTag *tag = gtk_text_tag_table_lookup(gtk_text_buffer_get_tag_table(messageBuffer), tagname.c_str());

	if (!tag)
	{
		string fore, back;
		int bold, italic;

		getSettingTag_gui(wsm, type, fore, back, bold, italic);

		tag = gtk_text_buffer_create_tag(messageBuffer, tagname.c_str(),
			"foreground", fore.c_str(),
			"background", back.c_str(),
			"weight", bold ? TEXT_WEIGHT_BOLD : TEXT_WEIGHT_NORMAL,
			"style", italic ? TEXT_STYLE_ITALIC : TEXT_STYLE_NORMAL,
			NULL);
	}

	return tag;
}

void PrivateMessage::updateCursor(GtkWidget *widget)
{
	gint x, y, buf_x, buf_y;
	GtkTextIter iter;
	GSList *tagList;
	GtkTextTag *newTag = NULL;

	GdkDeviceManager *device_manager;
	GdkDevice *pointer;
//@NOTE: GTK3
	device_manager = gdk_display_get_device_manager (gdk_window_get_display (gtk_widget_get_window(widget)));
	pointer = gdk_device_manager_get_client_pointer (device_manager);
	gdk_window_get_device_position (gtk_widget_get_window(widget), pointer, &x, &y, NULL);

	// Check for tags under the cursor, and change mouse cursor appropriately
	gtk_text_view_window_to_buffer_coords(GTK_TEXT_VIEW(widget), GTK_TEXT_WINDOW_WIDGET, x, y, &buf_x, &buf_y);
	gtk_text_view_get_iter_at_location(GTK_TEXT_VIEW(widget), &iter, buf_x, buf_y);
	tagList = gtk_text_iter_get_tags(&iter);

	if (tagList != NULL)
	{
		newTag = GTK_TEXT_TAG(tagList->data);

		if (newTag == TagsMap[Tag::TAG_URL])
		{
			GSList *nextList = g_slist_next(tagList);

			if (nextList != NULL)
				newTag = GTK_TEXT_TAG(nextList->data);
			else
				newTag = NULL;
		}

		g_slist_free(tagList);
	}


	if (newTag != selectedTag)
	{
		// Cursor is in transition.
		if (newTag != NULL)
		{
			// Cursor is entering a tag.
			gchar *tmp;
			g_object_get(G_OBJECT(newTag),"name",&tmp,NULL);
			selectedTagStr = string(tmp);

			if (find(TagsMap, TagsMap + Tag::TAG_URL, newTag) == TagsMap + Tag::TAG_URL)
			{
				// Cursor was in neutral space.
				gdk_window_set_cursor(gtk_text_view_get_window(GTK_TEXT_VIEW(widget), GTK_TEXT_WINDOW_TEXT), handCursor);
			}
			else
				gdk_window_set_cursor(gtk_text_view_get_window(GTK_TEXT_VIEW(widget), GTK_TEXT_WINDOW_TEXT), NULL);
		}
		else
		{
			// Cursor is entering neutral space.
			gdk_window_set_cursor(gtk_text_view_get_window(GTK_TEXT_VIEW(widget), GTK_TEXT_WINDOW_TEXT), NULL);
		}

		selectedTag = newTag;
	}
}

gboolean PrivateMessage::onFocusIn_gui(GtkWidget *widget, GdkEventFocus *event, gpointer data)
{
	PrivateMessage *pm = (PrivateMessage *)data;

	gtk_widget_grab_focus(pm->getWidget("entry"));

	// fix select text
	gtk_editable_set_position(GTK_EDITABLE(pm->getWidget("entry")), -1);

	return TRUE;
}

void PrivateMessage::onSendMessage_gui(GtkEntry *entry, gpointer data)
{
	string text = gtk_entry_get_text(entry);
	if (text.empty())
		return;

	PrivateMessage *pm = (PrivateMessage *)data;
	gtk_entry_set_text(entry, "");

	// Store line in chat history
	pm->history.pop_back();
	pm->history.push_back(text);
	pm->history.push_back("");
	pm->historyIndex = pm->history.size() - 1;
	if (pm->history.size() > maxHistory + 1)
		pm->history.erase(pm->history.begin());

	// Process special commands
	if (text[0] == '/')
	{
		string command = text, param, params;
		string::size_type separator = text.find_first_of(' ');
		if (separator != string::npos && text.size() > separator + 1)
		{
			command = text.substr(1, separator - 1);
			params = text.substr(separator + 1);
		}
		bool isThirdPerson = false;
		string message = Util::emptyString, status = Util::emptyString;
		if(PluginManager::getInstance()->onChatCommandPM(HintedUser(make_shared<User>(User(CID(pm->cid))),pm->hubUrl),command,false )) {
			// Plugins, chat commands
		  return;
	    }

		if(WulforUtil::checkCommand(command, param, message, status, isThirdPerson))
		{
			if(!message.empty())
				pm->addMessage_gui(message, Msg::MYOWN);

			if(!status.empty())
				pm->addStatusMessage_gui(status, Msg::STATUS);
		}
		else if (command == "clear")
		{
			GtkTextIter startIter, endIter;
			gtk_text_buffer_get_start_iter(pm->messageBuffer, &startIter);
			gtk_text_buffer_get_end_iter(pm->messageBuffer, &endIter);
			gtk_text_buffer_delete(pm->messageBuffer, &startIter, &endIter);
		}
		else if (command == "close")
		{
			WulforManager::get()->getMainWindow()->removeBookEntry_gui(pm);
		}
		else if (command == "fuser" || command == "fu")
		{
			typedef Func0<PrivateMessage> F0;
			F0 *func = new F0(pm, &PrivateMessage::addFavoriteUser_client);
			WulforManager::get()->dispatchClientFunc(func);
		}
		else if (command == "removefu" || command == "rmfu")
		{
			typedef Func0<PrivateMessage> F0;
			F0 *func = new F0(pm, &PrivateMessage::removeFavoriteUser_client);
			WulforManager::get()->dispatchClientFunc(func);
		}
		else if (command == "getlist")
		{
			typedef Func0<PrivateMessage> F0;
			F0 *func = new F0(pm, &PrivateMessage::getFileList_client);
			WulforManager::get()->dispatchClientFunc(func);
		}
		else if (command == "grant")
		{
			typedef Func0<PrivateMessage> F0;
			F0 *func = new F0(pm, &PrivateMessage::grantSlot_client);
			WulforManager::get()->dispatchClientFunc(func);
		}
		else if (command == "emoticons" || command == "emot")
		{
			if (pm->useEmoticons)
			{
				pm->useEmoticons = FALSE;
				pm->addStatusMessage_gui(_("Emoticons mode off"), Msg::SYSTEM);
			}
			else
			{
				pm->useEmoticons = TRUE;
				pm->addStatusMessage_gui(_("Emoticons mode on"), Msg::SYSTEM);
			}
		}
		else if (command == "help")
		{
			pm->addLine_gui(Msg::SYSTEM, string(_("*** Available commands:")) + "\n\n" +
			"/away <message>\t\t - " + _("Away mode message on/off") + "\n" +
			"/back\t\t\t\t - " + _("Away mode off") + "\n" +
			"/clear\t\t\t\t - " + _("Clear PM") + "\n" +
			"/close\t\t\t\t - " + _("Close PM") + "\n" +
			"/fuser, /fu\t\t\t\t - " + _("Add user to favorites list") + "\n" +
			"/removefu, /rmfu\t\t - " + _("Remove user favorite") + "\n" +
			"/getlist\t\t\t\t - " + _("Get file list") + "\n" +
			"/grant\t\t\t\t - " + _("Grant extra slot") + "\n" +
			"/emoticons, /emot\t\t - " + _("Emoticons on/off") + "\n" +
			"/help\t\t\t\t - " + _("Show help") + "\n" +
			WulforUtil::commands+"\n") ;
		}
		else
		{
			pm->addStatusMessage_gui(_("Unknown command ") + text + _(": type /help for a list of available commands"), Msg::SYSTEM);
		}
	}
	else
	{
		typedef Func1<PrivateMessage, string> F1;
		F1 *func = new F1(pm, &PrivateMessage::sendMessage_client, text);
		WulforManager::get()->dispatchClientFunc(func);
	}
}

gboolean PrivateMessage::onKeyPress_gui(GtkWidget *widget, GdkEventKey *event, gpointer data)
{
	PrivateMessage *pm = (PrivateMessage *)data;
	string text;
	size_t index;

	if ( ( WGETB("key-hub-with-ctrl") &&
		((event->keyval == GDK_KEY_Up)) && (event->state & GDK_CONTROL_MASK ) )
		|| (!WGETB("key-hub-with-ctrl") && (event->keyval == GDK_KEY_Up )) )
	{
		index = pm->historyIndex - 1;
		if (index < pm->history.size())
		{
			text = pm->history[index];
			pm->historyIndex = index;
			gtk_entry_set_text(GTK_ENTRY(widget), text.c_str());
		}
		return TRUE;
	}
	else if ((
		WGETB("key-hub-with-ctrl") && ((event->keyval == GDK_KEY_Down ) && (event->state & GDK_CONTROL_MASK ) ))
		|| ( !WGETB("key-hub-with-ctrl") && (event->keyval == GDK_KEY_Down)))
	{
		index = pm->historyIndex + 1;
		if (index < pm->history.size())
		{
			text = pm->history[index];
			pm->historyIndex = index;
			gtk_entry_set_text(GTK_ENTRY(widget), text.c_str());
		}
		return TRUE;
	}

	return FALSE;
}

gboolean PrivateMessage::onLinkTagEvent_gui(GtkTextTag *tag, GObject *textView, GdkEvent *event, GtkTextIter *iter, gpointer data)
{
	PrivateMessage *pm = (PrivateMessage *)data;

	if (event->type == GDK_BUTTON_PRESS)
	{
		gchar *tmp;
		g_object_get(G_OBJECT(tag),"name",&tmp,NULL);
		pm->selectedTagStr = string(tmp);

		switch (event->button.button)
		{
			case 1:
				onOpenLinkClicked_gui(NULL, data);
				break;
			case 3:
				// Pop-up link context menu
				gtk_widget_show_all(pm->getWidget("linkMenu"));
				gtk_menu_popup(GTK_MENU(pm->getWidget("linkMenu")), NULL, NULL, NULL, NULL, 0, gtk_get_current_event_time());
				break;
		}
		return TRUE;
	}
	return FALSE;
}

gboolean PrivateMessage::onHubTagEvent_gui(GtkTextTag *tag, GObject *textView, GdkEvent *event, GtkTextIter *iter, gpointer data)
{
	PrivateMessage *pm = (PrivateMessage *)data;

	if (event->type == GDK_BUTTON_PRESS)
	{
		gchar *tmp;
		g_object_get(G_OBJECT(tag),"name",&tmp,NULL);
		pm->selectedTagStr = string(tmp);

		switch (event->button.button)
		{
			case 1:
				onOpenHubClicked_gui(NULL, data);
				break;
			case 3:
				// Popup hub context menu
				gtk_widget_show_all(pm->getWidget("hubMenu"));
				gtk_menu_popup(GTK_MENU(pm->getWidget("hubMenu")), NULL, NULL, NULL, NULL, 0, gtk_get_current_event_time());
				break;
		}
		return TRUE;
	}
	return FALSE;
}

gboolean PrivateMessage::onMagnetTagEvent_gui(GtkTextTag *tag, GObject *textView, GdkEvent *event, GtkTextIter *iter, gpointer data)
{
	PrivateMessage *pm = (PrivateMessage *)data;

	if (event->type == GDK_BUTTON_PRESS)
	{
		gchar *tmp;
		g_object_get(G_OBJECT(tag),"name",&tmp,NULL);
		pm->selectedTagStr = string(tmp);

		switch (event->button.button)
		{
			case 1:
				// Search for magnet
				WulforManager::get()->getMainWindow()->actionMagnet_gui(pm->selectedTagStr);
				break;
			case 3:
				// Popup magnet context menu
				gtk_widget_show_all(pm->getWidget("magnetMenu"));
				gtk_menu_popup(GTK_MENU(pm->getWidget("magnetMenu")), NULL, NULL, NULL, NULL, 0, gtk_get_current_event_time());
				break;
		}
		return TRUE;
	}
	return FALSE;
}

gboolean PrivateMessage::onIpTagEvent_gui(GtkTextTag *tag, GObject *textView, GdkEvent *event , GtkTextIter *iter, gpointer data)
{
	PrivateMessage *pm = (PrivateMessage *)data;
	gchar *tmp;
	g_object_get(G_OBJECT(tag),"name",&tmp,NULL);
	pm->ip = std::string(tmp);

	if(event->type == GDK_BUTTON_PRESS)
	{
		if(event->button.button == 3)
		{
			gtk_widget_show_all(pm->getWidget("ipMenu"));
			gtk_menu_popup(GTK_MENU(pm->getWidget("ipMenu")), NULL, NULL, NULL, NULL, 0, gtk_get_current_event_time());
			return TRUE;
		}
	}
	return FALSE;
}

gboolean PrivateMessage::onChatPointerMoved_gui(GtkWidget* widget, GdkEventMotion* event, gpointer data)
{
	PrivateMessage *pm = (PrivateMessage *)data;

	pm->updateCursor(widget);

	return FALSE;
}

gboolean PrivateMessage::onChatVisibilityChanged_gui(GtkWidget* widget, GdkEventVisibility* event, gpointer data)
{
	PrivateMessage *pm = (PrivateMessage *)data;

	pm->updateCursor(widget);

	return FALSE;
}

gboolean PrivateMessage::onEmotButtonRelease_gui(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
	PrivateMessage *pm = (PrivateMessage *)data;

	switch (event->button)
	{
		case 1: //show emoticons dialog

			pm->emotdialog->showEmotDialog_gui();
		break;

		case 3: //show emoticons menu

			pm->emotdialog->buildEmotMenu_gui();

			GtkWidget *check_item = NULL;
			GtkWidget *emot_menu = pm->getWidget("emotMenu");

			check_item = gtk_separator_menu_item_new();
			gtk_menu_shell_append(GTK_MENU_SHELL(emot_menu), check_item);
			gtk_widget_show(check_item);

			check_item = gtk_check_menu_item_new_with_label(_("Use Emoticons"));
			gtk_menu_shell_append(GTK_MENU_SHELL(emot_menu), check_item);

			if (pm->useEmoticons)
				gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(check_item), TRUE);

			g_signal_connect(check_item, "activate", G_CALLBACK(onUseEmoticons_gui), data);

			gtk_widget_show_all(emot_menu);
			gtk_menu_popup(GTK_MENU(emot_menu), NULL, NULL, NULL, NULL, 0, gtk_get_current_event_time());
		break;
	}

	return FALSE;
}

void PrivateMessage::onChatScroll_gui(GtkAdjustment *adjustment, gpointer data)
{
	PrivateMessage *pm = (PrivateMessage *)data;
	gdouble value = gtk_adjustment_get_value(adjustment);
    pm->scrollToBottom = value >= ( gtk_adjustment_get_upper(adjustment) - gtk_adjustment_get_page_size (adjustment));
}

void PrivateMessage::onChatResize_gui(GtkAdjustment *adjustment, gpointer data)
{
	PrivateMessage *pm = (PrivateMessage *)data;
	gdouble value = gtk_adjustment_get_value(adjustment);

    if (pm->scrollToBottom && value < (gtk_adjustment_get_upper(adjustment) - gtk_adjustment_get_page_size (adjustment)))
	{
		GtkTextIter iter;

		gtk_text_buffer_get_end_iter(pm->messageBuffer, &iter);
		gtk_text_buffer_move_mark(pm->messageBuffer, pm->mark, &iter);
		gtk_text_view_scroll_to_mark(GTK_TEXT_VIEW(pm->getWidget("text")), pm->mark, 0, FALSE, 0, 0);
	}
}

void PrivateMessage::onCopyURIClicked_gui(GtkMenuItem *item, gpointer data)
{
	PrivateMessage *pm = (PrivateMessage *)data;

	gtk_clipboard_set_text(gtk_clipboard_get(GDK_SELECTION_CLIPBOARD), pm->selectedTagStr.c_str(), pm->selectedTagStr.length());
}

void PrivateMessage::onOpenLinkClicked_gui(GtkMenuItem *item, gpointer data)
{
	PrivateMessage *pm = (PrivateMessage *)data;
	string error = Util::emptyString;
	WulforUtil::openURI(pm->selectedTagStr, error);

	if(!error.empty())
		pm->setStatus_gui(error);
}

void PrivateMessage::onOpenHubClicked_gui(GtkMenuItem *item, gpointer data)
{
	PrivateMessage *pm = (PrivateMessage *)data;

	WulforManager::get()->getMainWindow()->showHub_gui(pm->selectedTagStr);
}

void PrivateMessage::onSearchMagnetClicked_gui(GtkMenuItem *item, gpointer data)
{
	PrivateMessage *pm = (PrivateMessage *)data;

	WulforManager::get()->getMainWindow()->addSearch_gui(pm->selectedTagStr);
}

void PrivateMessage::onDownloadClicked_gui(GtkMenuItem *item, gpointer data)
{
	PrivateMessage *pm = (PrivateMessage *)data;
	WulforManager::get()->getMainWindow()->fileToDownload_gui(pm->selectedTagStr, SETTING(DOWNLOAD_DIRECTORY));
}

void PrivateMessage::onDownloadToClicked_gui(GtkMenuItem *item, gpointer data)
{
	PrivateMessage *pm = (PrivateMessage *)data;

	GtkWidget *dialog = WulforManager::get()->getMainWindow()->getChooserDialog_gui();
	gtk_window_set_title(GTK_WINDOW(dialog), _("Choose a directory"));
	gtk_file_chooser_set_action(GTK_FILE_CHOOSER(dialog), GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER);
	gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(dialog), Text::fromUtf8(WGETS("magnet-choose-dir")).c_str());
	gint response = gtk_dialog_run(GTK_DIALOG(dialog));

	// if the dialog gets programmatically destroyed.
	if (response == GTK_RESPONSE_NONE)
		return;

	if (response == GTK_RESPONSE_OK)
	{
		gchar *temp = gtk_file_chooser_get_current_folder(GTK_FILE_CHOOSER(dialog));

		if (temp)
		{
			string path = Text::toUtf8(temp) + G_DIR_SEPARATOR_S;
			g_free(temp);

			WulforManager::get()->getMainWindow()->fileToDownload_gui(pm->selectedTagStr, path);
		}
	}
	gtk_widget_hide(dialog);
}

void PrivateMessage::onMagnetPropertiesClicked_gui(GtkMenuItem *item, gpointer data)
{
	PrivateMessage *pm = (PrivateMessage *)data;

	WulforManager::get()->getMainWindow()->propertiesMagnetDialog_gui(pm->selectedTagStr);
}

void PrivateMessage::onCommandClicked_gui(GtkWidget *widget, gpointer data)
{
	PrivateMessage *pm = (PrivateMessage *)data;

	string command = (gchar*)g_object_get_data(G_OBJECT(widget), "command");

	gint pos = 0;
	GtkWidget *entry = pm->getWidget("entry");
	if (!gtk_widget_is_focus(entry))
		gtk_widget_grab_focus(entry);
	gtk_editable_delete_text(GTK_EDITABLE(entry), pos, -1);
	gtk_editable_insert_text(GTK_EDITABLE(entry), command.c_str(), -1, &pos);
	gtk_editable_set_position(GTK_EDITABLE(entry), pos);
}

gboolean PrivateMessage::onChatCommandButtonRelease_gui(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
	PrivateMessage *pm = (PrivateMessage *)data;

	if (event->button == 1)
	{
		gtk_menu_popup(GTK_MENU(pm->getWidget("chatCommandsMenu")), NULL, NULL, NULL, NULL, 0, gtk_get_current_event_time());
	}

	return FALSE;
}

void PrivateMessage::onUseEmoticons_gui(GtkWidget *widget, gpointer data)
{
	PrivateMessage *pm = (PrivateMessage *)data;

	pm->useEmoticons = !pm->useEmoticons;
}

void PrivateMessage::updateOnlineStatus_gui(bool online)
{
	setIcon_gui(online ? WGETS("icon-pm-online") : WGETS("icon-pm-offline"));
}

void PrivateMessage::sendMessage_client(string message)
{
	UserPtr user = ClientManager::getInstance()->findUser(CID(cid));
	if (user && user->isOnline())
	{
		// NOTE: WTF does the 3rd param (bool thirdPerson) do? A: Used for /me stuff
		ClientManager::getInstance()->privateMessage(HintedUser(user, hubUrl), message, false);//NOTE: core 0.762
	}
	else
	{
		typedef Func2<PrivateMessage, string, Msg::TypeMsg> F2;
		F2 *func = new F2(this, &PrivateMessage::addStatusMessage_gui, _("User went offline"), Msg::STATUS);
		WulforManager::get()->dispatchGuiFunc(func);
	}
}

void PrivateMessage::addFavoriteUser_client()
{
	UserPtr user = ClientManager::getInstance()->findUser(CID(cid));

	if (user && FavoriteManager::getInstance()->isFavoriteUser(user))
	{
		typedef Func2<PrivateMessage, string, Msg::TypeMsg> F2;
		F2 *func = new F2(this, &PrivateMessage::addStatusMessage_gui, WulforUtil::getNicks(user, hubUrl) + _(" is favorite user"),
			Msg::STATUS);//NOTE: core 0.762
		WulforManager::get()->dispatchGuiFunc(func);
	}
	else
	{
		FavoriteManager::getInstance()->addFavoriteUser(user);
	}
}

void PrivateMessage::removeFavoriteUser_client()
{
	UserPtr user = ClientManager::getInstance()->findUser(CID(cid));

	if (user && FavoriteManager::getInstance()->isFavoriteUser(user))
	{
		FavoriteManager::getInstance()->removeFavoriteUser(user);
	}
	else
	{
		typedef Func2<PrivateMessage, string, Msg::TypeMsg> F2;
		F2 *func = new F2(this, &PrivateMessage::addStatusMessage_gui, WulforUtil::getNicks(user, hubUrl) + _(" is not favorite user"),
			Msg::STATUS);//NOTE: core 0.762
		WulforManager::get()->dispatchGuiFunc(func);
	}
}

void PrivateMessage::getFileList_client()
{
	try
	{
		UserPtr user = ClientManager::getInstance()->findUser(CID(cid));
		if (user)
			QueueManager::getInstance()->addList(HintedUser(user, hubUrl), QueueItem::FLAG_CLIENT_VIEW);//NOTE: core 0.762
	}
	catch (const Exception& e)
	{
		typedef Func2<PrivateMessage, string, Msg::TypeMsg> F2;
		F2 *func = new F2(this, &PrivateMessage::addStatusMessage_gui, e.getError(), Msg::STATUS);
		WulforManager::get()->dispatchGuiFunc(func);
	}
}

void PrivateMessage::grantSlot_client()
{
	UserPtr user = ClientManager::getInstance()->findUser(CID(cid));
	if (user)
	{
		UploadManager::getInstance()->reserveSlot(HintedUser(user, hubUrl));//NOTE: core 0.762
	}
	else
	{
		typedef Func2<PrivateMessage, string, Msg::TypeMsg> F2;
		F2 *func = new F2(this, &PrivateMessage::addStatusMessage_gui, _("Slot granted"), Msg::STATUS);
		WulforManager::get()->dispatchGuiFunc(func);
	}
}

void PrivateMessage::on(ClientManagerListener::UserConnected, const UserPtr& aUser) throw()
{
	if (aUser->getCID() == CID(cid))
	{
		typedef Func1<PrivateMessage, bool> F1;
		F1 *func = new F1(this, &PrivateMessage::updateOnlineStatus_gui, aUser->isOnline());
		WulforManager::get()->dispatchGuiFunc(func);
	}
}

void PrivateMessage::on(ClientManagerListener::UserDisconnected, const UserPtr& aUser) throw()
{
	if (aUser->getCID() == CID(cid))
	{
		typedef Func1<PrivateMessage, bool> F1;
		F1 *func = new F1(this, &PrivateMessage::updateOnlineStatus_gui, aUser->isOnline());
		WulforManager::get()->dispatchGuiFunc(func);
		offline = true;
	}
}

void PrivateMessage::readLog(const string& logPath, const unsigned setting)
{
	if(setting == 0)
		return;
	if(logPath.empty())
		return;
	//..
	StringList lines;
	try {
			const int MAX_SIZE = 32 * 1024;

			File f(logPath, File::READ, File::OPEN);
			if(f.getSize() > MAX_SIZE) {
				f.setEndPos(-MAX_SIZE + 1);
			}
			lines = StringTokenizer<string>(f.read(MAX_SIZE), "\r\n").getTokens();
		} catch(const FileException&) { }

		if(lines.empty())
			return;

		// the last line in the log file is an empty line; remove it
		lines.pop_back();

		const size_t linesCount = lines.size();
		for(size_t i = ((linesCount > setting) ? (linesCount - setting) : 0); i < linesCount; ++i) {
			addLine_gui(Msg::PRIVATE, lines[i]);
		}
}

//custom popup menu
GtkWidget *PrivateMessage::createmenu()
{
	string nicks = WulforUtil::getNicks(this->cid, this->hubUrl);
	GtkWidget *item = getFItem();
	gtk_menu_item_set_label(GTK_MENU_ITEM(item), nicks.c_str());

	userCommandMenu->cleanMenu_gui();
	userCommandMenu->addUser(cid);
	userCommandMenu->addHub(hubUrl);
	userCommandMenu->buildMenu_gui();
	GtkWidget *menu = userCommandMenu->getContainer();

	GtkWidget *copyHubUrl = gtk_menu_item_new_with_label(_("Copy CID"));
	GtkWidget *close = gtk_menu_item_new_with_label(_("Close"));
	GtkWidget *addFav = gtk_menu_item_new_with_label(_("Add to Favorite Users"));
	GtkWidget *copyNicks = gtk_menu_item_new_with_label(_("Copy Nick(s)"));

	gtk_menu_shell_append(GTK_MENU_SHELL(menu),close);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu),copyHubUrl);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu),addFav);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu),copyNicks);
	gtk_widget_show(close);
	gtk_widget_show(copyHubUrl);
	gtk_widget_show(addFav);
	gtk_widget_show(copyNicks);
	gtk_widget_show_all(userCommandMenu->getContainer());

	g_signal_connect_swapped(copyHubUrl, "activate", G_CALLBACK(onCopyCID), (gpointer)this);
	g_signal_connect_swapped(close, "activate", G_CALLBACK(onCloseItem), (gpointer)this);
	g_signal_connect_swapped(addFav, "activate", G_CALLBACK(onAddFavItem), (gpointer)this);
	g_signal_connect_swapped(copyNicks, "activate", G_CALLBACK(onCopyNicks), (gpointer)this);
    return menu;
}

void PrivateMessage::onCloseItem(gpointer data)
{
    PrivateMessage *entry = (PrivateMessage *)data;
    WulforManager::get()->getMainWindow()->removeBookEntry_gui(dynamic_cast<BookEntry*>(entry));
}

void PrivateMessage::onCopyCID(gpointer data)
{
    PrivateMessage *pm = (PrivateMessage *)data;
    gtk_clipboard_set_text(gtk_clipboard_get(GDK_SELECTION_CLIPBOARD), pm->cid.c_str(), pm->cid.length());
}

void PrivateMessage::onCopyNicks(gpointer data)
{
	PrivateMessage *pm = (PrivateMessage *)data;
	string nicks = WulforUtil::getNicks(pm->cid, pm->hubUrl);
	gtk_clipboard_set_text(gtk_clipboard_get(GDK_SELECTION_CLIPBOARD), nicks.c_str(), nicks.length());
}

void PrivateMessage::onAddFavItem(gpointer data)
{
	PrivateMessage *pm = (PrivateMessage *)data;
	pm->addFavoriteUser_client();
}
//EnD

void PrivateMessage::onCopyIpItem_gui(GtkWidget *wid, gpointer data)
{
	PrivateMessage *pm = (PrivateMessage *)data;
	gtk_clipboard_set_text(gtk_clipboard_get(GDK_SELECTION_CLIPBOARD), pm->ip.c_str(), pm->ip.length());
}

void PrivateMessage::onRipeDbItem_gui(GtkWidget *wid, gpointer data)
{
	PrivateMessage *pm = (PrivateMessage *)data;
	string error;
	dcpp::ParamMap params;
	params["IP"] = pm->ip;
	string result = dcpp::Util::formatParams(SETTING(RIPE_DB),params);
	WulforUtil::openURI(result,error);
	pm->setStatus_gui(error);
}
