/*
 * Copyright © 2004-2017 Jens Oknelid, paskharen@gmail.com
 * Copyright © 2011-2025 BMDC
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

#include "../dcpp/ClientManager.h"
#include "../dcpp/FavoriteManager.h"
#include "../dcpp/StringTokenizer.h"
#include "../dcpp/GeoManager.h"
#include "settingsmanager.hh"
#include "emoticonsdialog.hh"
#include "emoticons.hh"
#include "wulformanager.hh"
#include "GuiUtil.hh"
#include "search.hh"
#include "sound.hh"

using namespace std;
using namespace dcpp;

const GActionEntry PrivateMessage::pm_entries[] = {
		{ "add-fav-user", onAddFavItem, NULL, NULL, NULL },
		{ "rem-fav-user", onDeleteFavItem, NULL, NULL, NULL },
		{ "copy-cid", onCopyCID, NULL, NULL, NULL }
};

PrivateMessage::PrivateMessage(const string &_cid, const string &_hubUrl):
	BookEntry(Entry::PRIVATE_MESSAGE, WulforUtil::getNicks(_cid, _hubUrl), "privatemessage", _cid),
	dcpp::Flags(NORMAL),
	cid(_cid), hubUrl(_hubUrl),
	historyIndex(0), sentAwayMessage(false),
	scrollToBottom(true), notCreated(true)
{
	GSimpleActionGroup *group;
	group = g_simple_action_group_new ();
	g_action_map_add_action_entries (G_ACTION_MAP (group), pm_entries, G_N_ELEMENTS (pm_entries), (gpointer)this);
	gtk_widget_insert_action_group(getContainer(), "pm" ,G_ACTION_GROUP(group));

	setName(cid);
	//Set Colors
	gtk_widget_set_name(getWidget("text"), "pm");
	WulforUtil::setTextDeufaults(getWidget("text"), SETTING(BACKGROUND_PM_COLOR), SETTING(BACKGROUND_PM_IMAGE), true);
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

	GtkAdjustment *adjustment = gtk_scrolled_window_get_vadjustment(GTK_SCROLLED_WINDOW(getWidget("scroll")));
	// menu
//	userCommandMenu = new UserCommandMenu(gtk_menu_new(), ::UserCommand::CONTEXT_USER);
	// Emoticons dialog
	emotdialog = new EmoticonsDialog(getWidget("entry"), getWidget("emotButton"), getWidget("emotMenu"));

// Connect the signals to their callback functions.
/*	
	g_signal_connect(getWidget("copyLinkItem"), "activate", G_CALLBACK(onCopyURIClicked_gui), (gpointer)this);
	g_signal_connect(getWidget("openLinkItem"), "activate", G_CALLBACK(onOpenLinkClicked_gui), (gpointer)this);
	g_signal_connect(getWidget("copyhubItem"), "activate", G_CALLBACK(onCopyURIClicked_gui), (gpointer)this);
	g_signal_connect(getWidget("openhubItem"), "activate", G_CALLBACK(onOpenHubClicked_gui), (gpointer)this);
	g_signal_connect(getWidget("copyMagnetItem"), "activate", G_CALLBACK(onCopyURIClicked_gui), (gpointer)this);
	g_signal_connect(getWidget("searchMagnetItem"), "activate", G_CALLBACK(onSearchMagnetClicked_gui), (gpointer)this);
	g_signal_connect(getWidget("magnetPropertiesItem"), "activate", G_CALLBACK(onMagnetPropertiesClicked_gui), (gpointer)this);
	g_signal_connect(getWidget("emotButton"), "button-release-event", G_CALLBACK(onEmotButtonRelease_gui), (gpointer)this);

	*/
	GtkGesture *gesture;
  gesture = gtk_gesture_click_new ();
  gtk_gesture_single_set_button (GTK_GESTURE_SINGLE (gesture), 3);
  g_signal_connect (gesture, "pressed",
                    G_CALLBACK (on_right_btn_pressed), (gpointer)this);
  g_signal_connect (gesture, "released",
                    G_CALLBACK (on_right_btn_released), (gpointer)this);
  gtk_widget_add_controller (GTK_WIDGET(getContainer()), GTK_EVENT_CONTROLLER (gesture));

  // keys stuff
  GtkEventController* keys = gtk_event_controller_key_new ();

  g_signal_connect (keys, "key-pressed",
                    G_CALLBACK (key_pressed_gui), (gpointer)this);
  g_signal_connect (keys, "key-released",
                    G_CALLBACK (key_released_gui), (gpointer)this);
  gtk_widget_add_controller (GTK_WIDGET(getWidget("entry")), GTK_EVENT_CONTROLLER (keys));

	history.push_back("");
	
	const OnlineUser* user = ClientManager::getInstance()->findOnlineUser(CID(cid), hubUrl);
	
	if(user) {
		if(user->getIdentity().isBot())
			setFlag(BOT);
		
		//string country = GeoManager::getInstance()->getCountryAbbrevation(user->getIdentity().getIp());
		//setImageButton(country);
	}
		
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
	params["hubNI"] = WulforUtil::getHubNames(cid, hubUrl);
	params["hubURL"] = hubUrl;
	params["userCID"] = cid;
	params["userNI"] = ClientManager::getInstance()->getNicks(CID(cid), hubUrl)[0];
	params["myCID"] = ClientManager::getInstance()->getMe()->getCID().toBase32();

	readLog(LogManager::getInstance()->getPath(LogManager::PM, params)
		,(unsigned int)SETTING(PM_LAST_LOG_LINES));
}


void PrivateMessage::on_right_btn_pressed (GtkGestureClick* /*gesture*/,
                                   int                /*n_press*/,
                                   double             x,
                                   double             y,
                                   gpointer         *data)
{
	PrivateMessage *PM = (PrivateMessage*)data;

	GMenu *menu = g_menu_new ();
	GMenuItem *menu_item_add = g_menu_item_new ("Add Favorite User", "pm.add-fav-user");
	g_menu_append_item (menu, menu_item_add);
	g_object_unref (menu_item_add);

	GMenuItem* menu_item_edit = g_menu_item_new ("Delete Favorite User", "pm.rem-fav-user");
	g_menu_append_item (menu, menu_item_edit);
	g_object_unref (menu_item_edit);

	GtkWidget *pop = gtk_popover_menu_new_from_model(G_MENU_MODEL(menu));
	gtk_widget_set_parent(pop, PM->getContainer());
	gtk_popover_set_pointing_to(GTK_POPOVER(pop), &(const GdkRectangle){x,y,1,1});
	gtk_popover_popup (GTK_POPOVER(pop));

}

void PrivateMessage::on_right_btn_released (GtkGestureClick *gesture,
                                    int             /* n_press*/,
                                    double          /* x*/,
                                    double           /*y*/,
                                    gpointer*       /*widget*/)
{
  gtk_gesture_set_state (GTK_GESTURE (gesture),
                         GTK_EVENT_SEQUENCE_CLAIMED);
}


PrivateMessage::~PrivateMessage()
{
	UsersManager::getInstance()->removeListener(this);
}

void PrivateMessage::show()
{
	UsersManager::getInstance()->addListener(this);
}

void PrivateMessage::addMessage_gui(string message, Msg::TypeMsg typemsg)
{
	addLine_gui(typemsg, message);

	if (SETTING(LOG_PRIVATE_CHAT))
	{
		dcpp::ParamMap params;
		params["message"] = message;
		params["hubNI"] = WulforUtil::getHubNames(cid, hubUrl);
		params["hubURL"] = hubUrl;
		params["userCID"] = cid;
		params["userNI"] = ClientManager::getInstance()->getNicks(CID(cid), hubUrl)[0];
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
	else if (!sentAwayMessage && !(SETTING(NO_AWAYMSG_TO_BOTS) && isSet(BOT)))
	{
		/* What away message to send */
		auto what = [this](ParamMap& params) -> std::string {
			if(hubUrl.empty())
					return Util::getAwayMessage(params);
			return FavoriteManager::getInstance()->getAwayMessage(hubUrl, params);
		};

		ParamMap params;
		params["message"] = message;
		params["hubNI"] = WulforUtil::getHubNames(cid, hubUrl);
		params["hubURL"] = hubUrl;
		params["userCID"] = cid;
		params["userNI"] = ClientManager::getInstance()->getNicks(CID(cid), hubUrl)[0];//NOTE: core 0.762
		params["myCID"] = ClientManager::getInstance()->getMe()->getCID().toBase32();

		sentAwayMessage = TRUE;
		typedef Func1<PrivateMessage, string> F1;
		F1 *func = new F1(this, &PrivateMessage::sendMessage_client, what(params) );
		WulforManager::get()->dispatchClientFunc(func);
	}

	if (WGETB("sound-pm"))
	{
		/*MainWindow *mw = WulforManager::get()->getMainWindow();
		GdkWindowState state = gdk_window_get_state(gtk_widget_get_window(mw->getContainer()));

		if ((state & GDK_WINDOW_STATE_ICONIFIED) || mw->currentPage_gui() != getContainer())
			Sound::get()->playSound(Sound::PRIVATE_MESSAGE);
		else if (WGETB("sound-pm-open")) Sound::get()->playSound(Sound::PRIVATE_MESSAGE);*/
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
	bool bold = false, italic = false;

	for (int i = Tag::TAG_FIRST; i < Tag::TAG_LAST; i++)
	{
		if(i == Tag::TAG_GENERAL) //@mainchat Tag
			continue;
		if(i == Tag::TAG_CHEAT) //@Cheating on mainchat Tag
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

	if (!SETTING(USE_EMOTS))
	{
		if (gtk_widget_is_sensitive(getWidget("emotButton")))
			gtk_widget_set_sensitive(getWidget("emotButton"), FALSE);
	}
	else if (!gtk_widget_is_sensitive(getWidget("emotButton")))
	{
		gtk_widget_set_sensitive(getWidget("emotButton"), TRUE);
	}
	WulforUtil::setTextDeufaults(getWidget("text"), SETTING(BACKGROUND_CHAT_COLOR));
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

	line += string(g_filename_to_utf8((message + "\n").c_str(),-1,NULL,NULL,NULL));

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
	bool start = false;
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

			start = true;
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
		bool isCountryFlag = false;

		if(WGETB("use-highlighting"))
		{
			GtkTextTag *tag = gtk_text_tag_table_lookup(gtk_text_buffer_get_tag_table(messageBuffer), temp);
			bool isTab = false;
			
			if(WulforUtil::isHighlightingWord(messageBuffer,tag,string(temp),isTab,(gpointer)NULL))
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
			g_free(temp);
			bool notlink = false;

			if(g_ascii_strncasecmp(tagName.c_str(), "[ccc]", 5) == 0)
			{
				string::size_type i = tagName.rfind("[/ccc]");
				if (i != string::npos)
				{
					country_text = tagName.substr(5, i - 5);
					if(country_text.length() == 2 )
					{
							notlink = isCountryFlag = true;
					}
            	}
            }

			if(!notlink)
			{
				/*if (WulforUtil::isLink(tagName))
					callback = G_CALLBACK(onLinkTagEvent_gui);
				else if (WulforUtil::isHubURL(tagName))
					callback = G_CALLBACK(onHubTagEvent_gui);
				else if (WulforUtil::isMagnet(tagName))
					callback = G_CALLBACK(onMagnetTagEvent_gui);*/
			}

			if(WulforUtil::HitIP(tagName))
			{
				/*callback = G_CALLBACK(onIpTagEvent_gui);
				isIp = true;
				userCommandMenu->cleanMenu_gui();
				userCommandMenu->addIp(tagName);
				userCommandMenu->addHub(cid);
				userCommandMenu->buildMenu_gui();*/
			}

		}

		if(isCountryFlag)
		{
			gtk_text_buffer_move_mark(messageBuffer, tag_mark, &tag_end_iter);

			if(country_text.length() == 2)
			{
				GdkPixbuf *buffer = WulforUtil::LoadCountryPixbuf(country_text);
				gtk_text_buffer_delete(messageBuffer, &tag_start_iter, &tag_end_iter);
				GtkTextChildAnchor *anchor = gtk_text_buffer_create_child_anchor(messageBuffer, &tag_start_iter);
				gtk_text_buffer_insert_paintable(messageBuffer, &tag_start_iter, WulforUtil::convertPixBuf(buffer));
			}

			applyEmoticons_gui();

			gtk_text_buffer_get_iter_at_mark(messageBuffer, &start_iter, tag_mark);

			if (gtk_text_iter_is_end(&start_iter))
				return;

			start = false;

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
			if (/*callback == G_CALLBACK(onMagnetTagEvent_gui) && WGETB("use-magnet-split")*/false)
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

			start = false;
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
		if (SETTING(USE_EMOTS))
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

	bool search = false;
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

		search = false;
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
						search = true;
						end_iter = match_start;

						/* set new limit search */
						gtk_text_buffer_get_iter_at_mark(messageBuffer, &tmp_end_iter, end_mark);
						for (int i = 1; !gtk_text_iter_equal(&end_iter, &tmp_end_iter) && i <= SIZE_NAME;
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
			if(*p_it)
				gtk_text_buffer_insert_paintable(messageBuffer, &p_start, WulforUtil::convertPixBuf((*p_it)->getPixbuf()));

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

void PrivateMessage::getSettingTag_gui(WulforSettingsManager *wsm, Tag::TypeTag type, string &fore, string &back, bool &bold, bool &italic)
{
	switch (type)
	{
		case Tag::TAG_MYOWN:

			fore = wsm->getString("text-myown-fore-color");
			back = wsm->getString("text-myown-back-color");
			bold = (bool)wsm->getInt("text-myown-bold");
			italic = (bool)wsm->getInt("text-myown-italic");
		break;

		case Tag::TAG_SYSTEM:

			fore = wsm->getString("text-system-fore-color");
			back = wsm->getString("text-system-back-color");
			bold = (bool)wsm->getInt("text-system-bold");
			italic = (bool)wsm->getInt("text-system-italic");
		break;

		case Tag::TAG_STATUS:

			fore = wsm->getString("text-status-fore-color");
			back = wsm->getString("text-status-back-color");
			bold = (bool)wsm->getInt("text-status-bold");
			italic = (bool)wsm->getInt("text-status-italic");
		break;

		case Tag::TAG_TIMESTAMP:

			fore = wsm->getString("text-timestamp-fore-color");
			back = wsm->getString("text-timestamp-back-color");
			bold = (bool)wsm->getInt("text-timestamp-bold");
			italic = (bool)wsm->getInt("text-timestamp-italic");
		break;

		case Tag::TAG_MYNICK:

			fore = wsm->getString("text-mynick-fore-color");
			back = wsm->getString("text-mynick-back-color");
			bold = (bool)wsm->getInt("text-mynick-bold");
			italic = (bool)wsm->getInt("text-mynick-italic");
		break;

		case Tag::TAG_OPERATOR:

			fore = wsm->getString("text-op-fore-color");
			back = wsm->getString("text-op-back-color");
			bold = (bool)wsm->getInt("text-op-bold");
			italic = (bool)wsm->getInt("text-op-italic");
		break;

		case Tag::TAG_FAVORITE:

			fore = wsm->getString("text-fav-fore-color");
			back = wsm->getString("text-fav-back-color");
			bold = (bool)wsm->getInt("text-fav-bold");
			italic = (bool)wsm->getInt("text-fav-italic");
		break;
		case Tag::TAG_IPADR:
			fore = wsm->getString("text-ip-fore-color");
			back = wsm->getString("text-ip-back-color");
			bold = (bool)wsm->getInt("text-ip-bold");
			italic = (bool)wsm->getInt("text-ip-italic");
		break;
		case Tag::TAG_URL:

			fore = wsm->getString("text-url-fore-color");
			back = wsm->getString("text-url-back-color");
			bold = (bool)wsm->getInt("text-url-bold");
			italic = (bool)wsm->getInt("text-url-italic");
		break;

		case Tag::TAG_NICK:

			fore = wsm->getString("text-private-fore-color");
			back = wsm->getString("text-private-back-color");
			italic = (bool)wsm->getInt("text-private-italic");
			bold = wsm->getBool("text-bold-autors");

		break;

		case Tag::TAG_PRIVATE:

		default:

			fore = wsm->getString("text-private-fore-color");
			back = wsm->getString("text-private-back-color");
			bold = (bool)wsm->getInt("text-private-bold");
			italic = (bool)wsm->getInt("text-private-italic");
	}
}

GtkTextTag* PrivateMessage::createTag_gui(const string &tagname, Tag::TypeTag type)
{
	WulforSettingsManager *wsm = WulforSettingsManager::getInstance();
	GtkTextTag *tag = gtk_text_tag_table_lookup(gtk_text_buffer_get_tag_table(messageBuffer), tagname.c_str());

	if (!tag)
	{
		string fore, back;
		bool bold = false, italic = false;

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
	GSList *tagList = NULL;
	GtkTextTag *newTag = NULL;
	GdkDevice *dev = NULL;

	GdkDisplay* win = gtk_widget_get_display(widget);
	GdkSeat* seat = gdk_display_get_default_seat(win);
	dev = gdk_seat_get_pointer(seat);

	// Check for tags under the cursor, and change mouse cursor appropriately
	gtk_text_view_window_to_buffer_coords(GTK_TEXT_VIEW(widget), GTK_TEXT_WINDOW_WIDGET, x, y, &buf_x, &buf_y);
	gtk_text_view_get_iter_at_location(GTK_TEXT_VIEW(widget), &iter, buf_x, buf_y);
	tagList = gtk_text_iter_get_tags(&iter);

	if (tagList != NULL)
	{
		newTag = GTK_TEXT_TAG(tagList->data);

		if (find(TagsMap + Tag::TAG_PRIVATE, TagsMap + Tag::TAG_LAST, newTag) != TagsMap + Tag::TAG_LAST)
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
			gchar *tmp = NULL;
			g_object_get(G_OBJECT(newTag),"name",&tmp,NULL);
			selectedTagStr = string(tmp);
			g_free(tmp);

			if (find(TagsMap, TagsMap + Tag::TAG_URL, newTag) == TagsMap + Tag::TAG_URL)
			{
				// Cursor was in neutral space.
				//gdk_window_set_cursor(gtk_text_view_get_window(GTK_TEXT_VIEW(widget), GTK_TEXT_WINDOW_TEXT), handCursor);
			}
			else;
				//gdk_window_set_cursor(gtk_text_view_get_window(GTK_TEXT_VIEW(widget), GTK_TEXT_WINDOW_TEXT), NULL);
		}
		else
		{
			// Cursor is entering neutral space.
			//gdk_window_set_cursor(gtk_text_view_get_window(GTK_TEXT_VIEW(widget), GTK_TEXT_WINDOW_TEXT), NULL);
		}

		selectedTag = newTag;
	}
}

void PrivateMessage::onSendMessage_gui(GtkEntry *entry, gpointer data)
{
	string text = gtk_editable_get_text(GTK_EDITABLE(entry));
	if (text.empty())
		return;

	PrivateMessage *pm = (PrivateMessage *)data;
	gtk_editable_set_text(GTK_EDITABLE(entry), "");

	// Store line in chat history
	pm->history.pop_back();
	//pm->history.push_back(text);
	pm->history.push_back("");
	pm->historyIndex = pm->history.size() - 1;
	if (pm->history.size() > ((size_t)(WGETI("pm-max-history") + 1)))
		pm->history.erase(pm->history.begin());

	// Process special commands
	if (text[0] == '/')
	{
		string command = text, params = string(), param;
		string::size_type separator = text.find_first_of(' ');
		if (separator != string::npos && text.size() > separator + 1)
		{
			command = text.substr(1, separator - 1);
			params = text.substr(separator + 1);
		}
		bool isThirdPerson = false;
		string message = string(), status = string();
        
        if(WulforUtil::checkCommand(text, param, message, status, isThirdPerson))
		{
			if(!message.empty())
				pm->sendMessage_client(message);

			if(!status.empty())
				pm->addStatusMessage_gui(status, Msg::STATUS);
			return;	
		}
		if (command == "clear")
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
			pm->addLine_gui(Msg::SYSTEM, string(_("*** Available commands:")) + "\n" +
			"\r\n/away <message>\r\n\t - " + _("Away mode message on/off") + 
			"\r\n/back \r\n\t - " + _("Away mode off") + 
			"\r\n/clear \r\n\t - " + _("Clear PM") + 
			"\r\n/close \r\n\t - " + _("Close PM") + 
			"\r\n/fuser, /fu \r\n\t - " + _("Add user to favorites list") + 
			"\r\n/removefu, /rmfu\r\n\t - " + _("Remove user favorite") + 
			"\r\n/getlist \r\n\t - " + _("Get file list") + 
			"\r\n/grant \r\n\t - " + _("Grant extra slot") +
			"\r\n/emoticons, /emot \r\n\t - " + _("Emoticons on/off") +
			"\r\n/help \r\n\t - " + _("Show help") +
			WulforUtil::commands+" \n") ;
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
/*
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
*//*
gboolean PrivateMessage::onLinkTagEvent_gui(GtkTextTag *tag, GObject*, GdkEvent *event, GtkTextIter*, gpointer data)
{
	if (event->type == GDK_BUTTON_PRESS)
	{
		PrivateMessage *pm = (PrivateMessage *)data;
		gchar *tmp = NULL;
		g_object_get(G_OBJECT(tag),"name",&tmp,NULL);
		pm->selectedTagStr = string(tmp);
		g_free(tmp);

		switch (event->button.button)
		{
			case 1:
				onOpenLinkClicked_gui(NULL, data);
				break;
			case 3:
				// Pop-up link context menu
				gtk_widget_show_all(pm->getWidget("linkMenu"));
				gtk_menu_popup_at_pointer(GTK_MENU(pm->getWidget("linkMenu")),NULL);
				break;
		}
		return TRUE;
	}
	return FALSE;
}

gboolean PrivateMessage::onHubTagEvent_gui(GtkTextTag *tag, GObject*, GdkEvent *event, GtkTextIter*, gpointer data)
{
	if (event->type == GDK_BUTTON_PRESS)
	{
		PrivateMessage *pm = (PrivateMessage *)data;
		pm->selectedTagStr = WulforUtil::getTagName(tag);

		switch (event->button.button)
		{
			case 1:
				onOpenHubClicked_gui(NULL, data);
				break;
			case 3:
				// Popup hub context menu
				gtk_widget_show_all(pm->getWidget("hubMenu"));
				gtk_menu_popup_at_pointer(GTK_MENU(pm->getWidget("hubMenu")),NULL);
				break;
		}
		return TRUE;
	}
	return FALSE;
}

gboolean PrivateMessage::onMagnetTagEvent_gui(GtkTextTag *tag, GObject*, GdkEvent *event, GtkTextIter*, gpointer data)
{
	if (event->type == GDK_BUTTON_PRESS)
	{
		PrivateMessage *pm = (PrivateMessage *)data;
		pm->selectedTagStr = WulforUtil::getTagName(tag);
		
		switch (event->button.button)
		{
			case 1:
				// Search for magnet
				WulforManager::get()->getMainWindow()->actionMagnet_gui(pm->selectedTagStr);
				break;
			case 3:
				// Popup magnet context menu
				gtk_widget_show_all(pm->getWidget("magnetMenu"));
				gtk_menu_popup_at_pointer(GTK_MENU(pm->getWidget("magnetMenu")),NULL);
				break;
		}
		return TRUE;
	}
	return FALSE;
}

gboolean PrivateMessage::onIpTagEvent_gui(GtkTextTag *tag, GObject*, GdkEvent *event , GtkTextIter*, gpointer data)
{
	PrivateMessage *pm = (PrivateMessage *)data;

	if(event->type == GDK_BUTTON_PRESS)
	{
		if(event->button.button == 3)
		{
            string tmp = WulforUtil::getTagName(tag);    
			g_signal_connect(pm->getWidget("ripeitem"), "activate", G_CALLBACK(onRipeDbItem_gui),(gpointer)pm);
			g_signal_connect(pm->getWidget("copyipItem"), "activate", G_CALLBACK(onCopyIpItem_gui),(gpointer)pm);
			g_object_set_data_full(G_OBJECT(pm->getWidget("ripeitem")),"ip_addr",g_strdup(tmp.c_str()),g_free);
			g_object_set_data_full(G_OBJECT(pm->getWidget("copyipItem")),"ip_addr",g_strdup(tmp.c_str()),g_free);
			
			gtk_widget_show_all(pm->getWidget("ipMenu"));
			gtk_menu_popup_at_pointer(GTK_MENU(pm->getWidget("ipMenu")),NULL);
			return TRUE;
		}
	}
	return FALSE;
}*/
/*
gboolean PrivateMessage::onEmotButtonRelease_gui(GtkWidget* wid, GdkEventButton *event, gpointer data)
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
			gtk_menu_popup_at_widget(GTK_MENU(emot_menu),wid,GDK_GRAVITY_SOUTH_WEST,GDK_GRAVITY_NORTH_WEST,NULL);
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

void PrivateMessage::onCopyURIClicked_gui(GtkMenuItem*, gpointer data)
{
	PrivateMessage *pm = (PrivateMessage *)data;

	gtk_clipboard_set_text(gtk_clipboard_get(GDK_SELECTION_CLIPBOARD), pm->selectedTagStr.c_str(), pm->selectedTagStr.length());
}

void PrivateMessage::onOpenLinkClicked_gui(GtkMenuItem*, gpointer data)
{
	PrivateMessage *pm = (PrivateMessage *)data;
	string error = dcpp::Util::emptyString;
	WulforUtil::openURI(pm->selectedTagStr, error);

	if(!error.empty())
		pm->setStatus_gui(error);
}

void PrivateMessage::onOpenHubClicked_gui(GtkMenuItem*, gpointer data)
{
	PrivateMessage *pm = (PrivateMessage *)data;

	WulforManager::get()->getMainWindow()->showHub_gui(pm->selectedTagStr);
}

void PrivateMessage::onSearchMagnetClicked_gui(GtkMenuItem*, gpointer data)
{
	PrivateMessage *pm = (PrivateMessage *)data;

	WulforManager::get()->getMainWindow()->addSearch_gui(pm->selectedTagStr);
}

void PrivateMessage::onDownloadClicked_gui(GtkMenuItem*, gpointer data)
{
	PrivateMessage *pm = (PrivateMessage *)data;
	WulforManager::get()->getMainWindow()->fileToDownload_gui(pm->selectedTagStr, SETTING(DOWNLOAD_DIRECTORY));
}

void PrivateMessage::onDownloadToClicked_gui(GtkMenuItem*, gpointer data)
{
	GtkWidget *dialog = WulforManager::get()->getMainWindow()->getChooserDialog_gui();
	gtk_window_set_title(GTK_WINDOW(dialog), _("Choose a directory"));
	gtk_file_chooser_set_action(GTK_FILE_CHOOSER(dialog), GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER);
	gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(dialog), Text::fromUtf8(WGETS("magnet-choose-dir")).c_str());
	gint response = gtk_dialog_run(GTK_DIALOG(dialog));

	if (response == GTK_RESPONSE_OK)
	{
		gchar *temp = gtk_file_chooser_get_current_folder(GTK_FILE_CHOOSER(dialog));

		if (temp)
		{
			PrivateMessage *pm = (PrivateMessage *)data;
			string path = Text::toUtf8(temp) + G_DIR_SEPARATOR_S;
			g_free(temp);

			WulforManager::get()->getMainWindow()->fileToDownload_gui(pm->selectedTagStr, path);
		}
	}
	gtk_widget_hide(dialog);
}

void PrivateMessage::onMagnetPropertiesClicked_gui(GtkMenuItem*, gpointer data)
{
	PrivateMessage *pm = (PrivateMessage *)data;

	WulforManager::get()->getMainWindow()->propertiesMagnetDialog_gui(pm->selectedTagStr);
}
*//*
void PrivateMessage::onUseEmoticons_gui(GtkWidget*, gpointer data)
{
	PrivateMessage *pm = (PrivateMessage *)data;

	pm->useEmoticons = !pm->useEmoticons;
}
*/
void PrivateMessage::updateOnlineStatus_gui(bool online)
{
	setIcon_gui(online ? WGETS("icon-pm-online") : WGETS("icon-pm-offline"));
}

void PrivateMessage::sendMessage_client(string message)
{
	UserPtr user = ClientManager::getInstance()->findUser(CID(cid),hubUrl);
	if (user && !isSet(OFFLINE))
	{
		// NOTE: WTF does the 3rd param (bool thirdPerson) do? A: Used for /me stuff
		ClientManager::getInstance()->privateMessage(HintedUser(user, hubUrl), message, false);
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
	UserPtr user = ClientManager::getInstance()->findUser(CID(cid),hubUrl);

	if (user && FavoriteManager::getInstance()->isFavoriteUser(user))
	{
		typedef Func2<PrivateMessage, string, Msg::TypeMsg> F2;
		F2 *func = new F2(this, &PrivateMessage::addStatusMessage_gui, WulforUtil::getNicks(user, hubUrl) + _(" is favorite user"),
			Msg::STATUS);
		WulforManager::get()->dispatchGuiFunc(func);
	}
	else
	{
		FavoriteManager::getInstance()->addFavoriteUser(user);
	}
}

void PrivateMessage::removeFavoriteUser_client()
{
	UserPtr user = ClientManager::getInstance()->findUser(CID(cid),hubUrl);

	if (user && FavoriteManager::getInstance()->isFavoriteUser(user))
	{
		FavoriteManager::getInstance()->removeFavoriteUser(user);
	}
	else
	{
		typedef Func2<PrivateMessage, string, Msg::TypeMsg> F2;
		F2 *func = new F2(this, &PrivateMessage::addStatusMessage_gui, WulforUtil::getNicks(user, hubUrl) + _(" is not favorite user"),
			Msg::STATUS);
		WulforManager::get()->dispatchGuiFunc(func);
	}
}

void PrivateMessage::getFileList_client()
{
	try
	{
		UserPtr user = ClientManager::getInstance()->findUser(CID(cid),hubUrl);
		if (user)
			QueueManager::getInstance()->addList(HintedUser(user, hubUrl), QueueItem::FLAG_CLIENT_VIEW);
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
	UserPtr user = ClientManager::getInstance()->findUser(CID(cid),hubUrl);
	if (user)
	{
		UploadManager::getInstance()->reserveSlot(HintedUser(user, hubUrl));
	}
	else
	{
		typedef Func2<PrivateMessage, string, Msg::TypeMsg> F2;
		F2 *func = new F2(this, &PrivateMessage::addStatusMessage_gui, _("Slot granted"), Msg::STATUS);
		WulforManager::get()->dispatchGuiFunc(func);
	}
}

void PrivateMessage::on(UsersManagerListener::UserConnected, const UserPtr& aUser) throw()
{
	if (aUser->getCID() == CID(cid))
	{
		typedef Func1<PrivateMessage, bool> F1;
		F1 *func = new F1(this, &PrivateMessage::updateOnlineStatus_gui, aUser->isOnline());
		WulforManager::get()->dispatchGuiFunc(func);
		
		if(aUser->isOnline() == true)
			setFlag(NORMAL);
	}
}

void PrivateMessage::on(UsersManagerListener::UserDisconnected, const UserPtr& aUser) throw()
{
	if (aUser->getCID() == CID(cid))
	{
		typedef Func1<PrivateMessage, bool> F1;
		F1 *func = new F1(this, &PrivateMessage::updateOnlineStatus_gui, aUser->isOnline());
		WulforManager::get()->dispatchGuiFunc(func);
		
		if(aUser->isOnline() == false)
			setFlag(OFFLINE);
	}
}

void PrivateMessage::readLog(const string& logPath, const unsigned setting)
{
	if(setting == 0) // Do not show log
		return;
	if(logPath.empty())
		return;

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
GMenu *PrivateMessage::createmenu()
{
	string nicks = WulforUtil::getNicks(this->cid, this->hubUrl);
	GMenu *menu = BookEntry::createmenu();
	
	GMenuItem* label = g_menu_item_new(nicks.c_str(), NULL);
	g_menu_prepend_item(menu ,label);

	GMenuItem *copy = g_menu_item_new("Copy CID", "pm.copy-cid");
	g_menu_prepend_item(menu ,copy);
	
	GMenuItem *fav = g_menu_item_new("Add to Favorite Users", "pm.add-fav-user");
	g_menu_prepend_item(menu ,fav);
	
	GMenuItem *copyNicks = g_menu_item_new("Copy nick(s)",NULL);
	g_menu_prepend_item(menu , copyNicks);
/*	
	if(notCreated) {
		userCommandMenu->cleanMenu_gui();
		userCommandMenu->addUser(cid);
		userCommandMenu->addHub(hubUrl);
		userCommandMenu->buildMenu_gui();
		*u_item = gtk_menu_item_new_with_label(_("Users Commands"));
		gtk_menu_item_set_submenu(GTK_MENU_ITEM(u_item),userCommandMenu->getContainer());
		notCreated = false;
	}	
	*/
    return menu;
}
/*
void PrivateMessage::onCloseItem(gpointer data)
{
    BookEntry *entry = dynamic_cast<BookEntry*>((PrivateMessage *)data);
    if(entry)
		WulforManager::get()->getMainWindow()->removeBookEntry_gui(entry);
}
*/
void PrivateMessage::onCopyCID(GtkWidget* widget , GVariant* var , gpointer data)
{
  PrivateMessage *pm = (PrivateMessage *)data;
	
	GValue value = G_VALUE_INIT;
	g_value_init (&value, G_TYPE_STRING);
	g_value_set_string (&value, pm->cid.c_str());

	// Store the value in the clipboard object
	GdkClipboard *clipboard = gtk_widget_get_clipboard (widget);
	gdk_clipboard_set_value (clipboard, &value);

	g_value_unset (&value);

}

void PrivateMessage::onCopyNicks(GtkWidget* widget , GVariant* var , gpointer data)
{
	PrivateMessage *pm = (PrivateMessage *)data;
	string nicks = WulforUtil::getNicks(pm->cid, pm->hubUrl);
	
	GValue value = G_VALUE_INIT;
	g_value_init (&value, G_TYPE_STRING);
	g_value_set_string (&value, nicks.c_str());

	// Store the value in the clipboard object
	GdkClipboard *clipboard = gtk_widget_get_clipboard (widget);
	gdk_clipboard_set_value (clipboard, &value);

	g_value_unset (&value);
}

void PrivateMessage::onAddFavItem(GtkWidget* widget , GVariant* var , gpointer data)
{
	PrivateMessage *pm = (PrivateMessage *)data;
	pm->addFavoriteUser_client();
}

void PrivateMessage::onCopyIpItem_gui(GtkWidget* widget , GVariant* var , gpointer)
{
	gchar* ip = (gchar*)g_object_get_data(G_OBJECT(widget),"ip_addr");
	GValue value = G_VALUE_INIT;
	g_value_init (&value, G_TYPE_STRING);
	g_value_set_string (&value, ip);

	// Store the value in the clipboard object
	GdkClipboard *clipboard = gtk_widget_get_clipboard (widget);
	gdk_clipboard_set_value (clipboard, &value);

	g_value_unset (&value);
}
/*
void PrivateMessage::onRipeDbItem_gui(GtkWidget* widget, gpointer data)
{
	PrivateMessage *pm = (PrivateMessage *)data;
	string ip = (char*)g_object_get_data(G_OBJECT(widget),"ip_addr");
	string error = dcpp::Util::emptyString;
	dcpp::ParamMap params;
	params["IP"] = ip;
	string result = dcpp::Util::formatParams(SETTING(RIPE_DB),params);
	WulforUtil::openURI(result,error);
	pm->setStatus_gui(error);
}
*//*
void PrivateMessage::setImageButton(const string country)
{
	gtk_image_set_from_pixbuf (GTK_IMAGE(getWidget("ImageButton")),WulforUtil::LoadCountryPixbuf(country));
}
*/

gboolean PrivateMessage::key_pressed_gui ( GtkEventControllerKey* self,  guint keyval,  guint keycode,  GdkModifierType state,  gpointer data )
{

	if( keyval == GDK_KEY_Return  ) {
			onSendMessage_gui(NULL,data);
			return TRUE;
	}
	return FALSE;
}

void PrivateMessage::key_released_gui (  GtkEventControllerKey* self,  guint keyval,  guint keycode,  GdkModifierType state,  gpointer data )
{

}

