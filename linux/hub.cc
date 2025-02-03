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
#include <gdk/gdk.h>

#include "hub.hh"
#include "../dcpp/FavoriteManager.h"
#include "../dcpp/HashManager.h"
#include "../dcpp/SearchManager.h"
#include "../dcpp/ShareManager.h"
#include "../dcpp/UserCommand.h"
#include "../dcpp/version.h"
#include "../dcpp/ChatMessage.h"
#include "../dcpp/GeoManager.h"
#include "../dcpp/ConnectivityManager.h"
#include "../dcpp/HighlightManager.h"
#include "../dcpp/AVManager.h"
#include "../dcpp/Text.h"
#include "version.hh"
#include "GuiUtil.hh"
#include "privatemessage.hh"
#include "search.hh"
#include "settingsmanager.hh"
#include "emoticonsdialog.hh"
#include "emoticons.hh"
#include "UserCommandMenu.hh"
#include "wulformanager.hh"
#include "ignoremenu.hh"
#include "IgnoreTempManager.hh"

using namespace std;
using namespace dcpp;

const string Hub::tagPrefix = "#";

const GActionEntry Hub::hub_entries[] = {
    { "nick-to-chat", onNickToChat_gui  , NULL, NULL, NULL },
    { "msg-to-user", onMsgItemClicked_gui , NULL, NULL, NULL },
    { "browse-fl", onBrowseItemClicked_gui , NULL, NULL, NULL },
    { "match-fl", onMatchItemClicked_gui, NULL, NULL, NULL },
    { "grant-slot", onGrantItemClicked_gui, NULL, NULL, NULL },
    { "fav-hubs", onAddFavItem, NULL, NULL,NULL },
    { "rem-f-hub" , onRemoveFavHub , NULL, NULL,NULL},
    { "add-fav-user", onAddFavoriteUserClicked_gui, NULL, NULL, NULL},
    { "copy-url", onCopyHubUrl, NULL, NULL, NULL},
    { "reconnect", onReconnectItemTab, NULL, NULL, NULL},
    { "add-ignored", onAddIgnoreUserItemClicked_gui, NULL, NULL, NULL},
    { "rem-ignored", onRemoveIgnoreUserItemClicked_gui, NULL, NULL, NULL},
    { "set-tab-name", onSetTabText, NULL, NULL, NULL}
};

Hub::Hub(const string &address, const string &encoding):
	BookEntry(Entry::HUB, address, "hub", address),
	client(nullptr), address(address),
	encoding(encoding),	historyIndex(0),totalShared(0),
	scrollToBottom(true), PasswordDialog(false), WaitingPassword(false),
	notCreated(true), isFavBool(WGETI("notify-hub-chat-use")), width(-1)
{
	//@note because "." and this is used in CSS'ing
	//@ use the CID'ing of it
	setName(CID(address).toBase32());

	GSimpleActionGroup* simple = g_simple_action_group_new ();
	g_action_map_add_action_entries (G_ACTION_MAP (simple), hub_entries, G_N_ELEMENTS (hub_entries), (gpointer)this);
	gtk_widget_insert_action_group(getContainer(), "hub" ,G_ACTION_GROUP(simple));
	gtk_widget_insert_action_group(getLabelBox(), "hub" ,G_ACTION_GROUP(simple));
	gtk_widget_insert_action_group(getWidget("nickView"), "hub" ,G_ACTION_GROUP(simple));

	FavoriteHubEntry* p_faventry =  getFavoriteHubEntry();

	// Initialize nick treeview
	nickView.setView(GTK_TREE_VIEW(getWidget("nickView")), true, "hub");
	nickView.insertColumn(_("Nick"), G_TYPE_STRING, TreeView::ICON_STRING_TEXT_COLOR, 100, "Icon", "NickColor");
	nickView.insertColumn(_("Shared"), G_TYPE_INT64, TreeView::SIZE, 75);
	nickView.insertColumn(_("Description"), G_TYPE_STRING, TreeView::STRING, 85);
	nickView.insertColumn(_("Tag"), G_TYPE_STRING, TreeView::STRING, 100);
	nickView.insertColumn(_("Connection"), G_TYPE_STRING, TreeView::STRING, 85);
	nickView.insertColumn("IP", G_TYPE_STRING, TreeView::STRING, 85);
	nickView.insertColumn(_("eMail"), G_TYPE_STRING, TreeView::STRING, 90);
	nickView.insertColumn(_("Country"), G_TYPE_STRING, TreeView::PIXBUF_STRING, 70, "Pixbuf");
	nickView.insertColumn(_("Exact Share"), G_TYPE_INT64, TreeView::EXSIZE, 100);
	nickView.insertColumn(_("Slots"), G_TYPE_STRING, TreeView::STRING, 50);
	nickView.insertColumn(_("Hubs"), G_TYPE_STRING, TreeView::STRING, 50);
	nickView.insertColumn("PK", G_TYPE_STRING, TreeView::STRING, 80);
	nickView.insertColumn(_("Cheat"), G_TYPE_STRING, TreeView::STRING, 80);
	nickView.insertColumn(_("Generator"), G_TYPE_STRING, TreeView::STRING, 80);
	nickView.insertColumn(_("Support"), G_TYPE_STRING, TreeView::STRING, 80);
	nickView.insertHiddenColumn("Icon", G_TYPE_STRING);
	nickView.insertHiddenColumn("Nick Order", G_TYPE_STRING);
	nickView.insertHiddenColumn("CID", G_TYPE_STRING);
	nickView.insertHiddenColumn("NickColor", G_TYPE_STRING);
	nickView.insertHiddenColumn("Pixbuf", GDK_TYPE_PIXBUF);
	nickView.insertHiddenColumn("Client Type", G_TYPE_INT);

	if(p_faventry) {
		nickView.restoreSettings(p_faventry->get(SettingsManager::HUB_UL_ORDER,SETTING(HUB_UL_ORDER)),
												p_faventry->get(SettingsManager::HUB_UL_SIZE,SETTING(HUB_UL_SIZE)),
												p_faventry->get(SettingsManager::HUB_UL_VISIBLE,SETTING(HUB_UL_VISIBLE)));
	}else{
		//TODO:Maybe usefull also disabling?
		nickView.restoreSettings(SETTING(HUB_UL_ORDER),SETTING(HUB_UL_SIZE),SETTING(HUB_UL_VISIBLE));
	}

	nickView.finalize();
	nickStore = gtk_list_store_newv(nickView.getColCount(), nickView.getGTypes());
	gtk_tree_view_set_model(nickView.get(), GTK_TREE_MODEL(nickStore));
	g_object_unref(nickStore);

    gtk_tree_selection_set_mode(gtk_tree_view_get_selection(nickView.get()), GTK_SELECTION_MULTIPLE);
	nickSelection = gtk_tree_view_get_selection(nickView.get());

	sort = SETTING(SORT_FAVUSERS_FIRST) ? "Client Type" : "Nick Order";
	if(p_faventry)
		sort = p_faventry->get(SettingsManager::SORT_FAVUSERS_FIRST,SETTING(SORT_FAVUSERS_FIRST)) ? "Client Type" : "Nick Order";
	//Own sort
	gtk_tree_view_column_set_sort_indicator(gtk_tree_view_get_column(nickView.get(), nickView.col(_("Nick"))), TRUE);
	gtk_tree_sortable_set_sort_func(GTK_TREE_SORTABLE(nickStore), nickView.col(sort), sort_iter_compare_func_nick,
                                    (gpointer)this, NULL);
	nickView.setSortColumn_gui(_("Nick"),"Nick Order");
    gtk_tree_sortable_set_sort_column_id(GTK_TREE_SORTABLE(nickStore), nickView.col(sort), GTK_SORT_ASCENDING);

	//BMDC++
	nickView.setSelection(nickSelection);
	//nickView.buildCopyMenu(getWidget("CopyMenu"));

	g_object_set(G_OBJECT(nickView.get()), "has-tooltip", TRUE, NULL);
	g_signal_connect(nickView.get(), "query-tooltip", G_CALLBACK(onUserListTooltip_gui), (gpointer)this);
	g_signal_connect (nickSelection, "changed", G_CALLBACK (selection_changed_userlist_gui), GTK_WIDGET(nickView.get()));
	/* Set a tooltip on the column */
	set_Header_tooltip_gui();

	// Initialize the chat window
	string sColor = p_faventry ? p_faventry->get(SettingsManager::BACKGROUND_CHAT_COLOR, SETTING(BACKGROUND_CHAT_COLOR)) : SETTING(BACKGROUND_CHAT_COLOR);
	string sImage = p_faventry ? p_faventry->get(SettingsManager::BACKGROUND_CHAT_IMAGE, SETTING(BACKGROUND_CHAT_IMAGE)) : SETTING(BACKGROUND_CHAT_IMAGE);

	WulforUtil::setTextDeufaults(getWidget("chatText"),sColor,sImage,false,address);

	// the reference count on the buffer is not incremented and caller of this function won't own a new reference.
	chatBuffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(getWidget("chatText")));

	/* initial markers */
	GtkTextIter iter;
	gtk_text_buffer_get_end_iter(chatBuffer, &iter);

	chatMark = gtk_text_buffer_create_mark(chatBuffer, NULL, &iter, TRUE);
	start_mark = gtk_text_buffer_create_mark(chatBuffer, NULL, &iter, TRUE);
	end_mark = gtk_text_buffer_create_mark(chatBuffer, NULL, &iter, TRUE);
	tag_mark = gtk_text_buffer_create_mark(chatBuffer, NULL, &iter, TRUE);
	emot_mark = gtk_text_buffer_create_mark(chatBuffer, NULL, &iter, TRUE);

	// Initialize the user command menu
	userCommandMenu = new UserCommandMenu(g_menu_new(), GTK_WIDGET(nickView.get()), ::UserCommand::CONTEXT_USER);
	// Hub ...
	userCommandMenu1 = new UserCommandMenu(BookEntry::createmenu(), getLabelBox(), ::UserCommand::CONTEXT_HUB);
	userCommandMenu1->addHub(address);
	userCommandMenu1->buildMenu_gui();
	// IP Address...
//	userCommandMenu2 = new UserCommandMenu(getWidget("ipmenu"), ::UserCommand::CONTEXT_IP);
//  ignoreMenu = new IgnoreMenu(getWidget("ignoreMenuTime"));

	string packName = SETTING(EMOT_PACK);

	if(p_faventry)
	{
		packName = p_faventry->get(SettingsManager::EMOT_PACK, SETTING(EMOT_PACK));
	}
	// Emoticons dialog
	emotdialog = new EmoticonsDialog(getWidget("chatEntry"), getWidget("emotButton"), getWidget("emotPacksMenu"), packName, address);
	if ( (p_faventry) &&  ( !p_faventry->get(SettingsManager::USE_EMOTS, SETTING(USE_EMOTS))))
		gtk_widget_set_sensitive(getWidget("emotButton"), FALSE);

	useEmoticons = true;

	GtkAdjustment *adjustment = gtk_scrolled_window_get_vadjustment(GTK_SCROLLED_WINDOW(getWidget("chatScroll")));
// Connect the signals to their callback functions.
//	g_signal_connect(nickView.get(), "key-release-event", G_CALLBACK(onNickListKeyRelease_gui), (gpointer)this);
	g_signal_connect(getWidget("chatEntry"), "activate", G_CALLBACK(onSendMessage_gui), (gpointer)this);
	g_signal_connect(adjustment, "value_changed", G_CALLBACK(onChatScroll_gui), (gpointer)this);
	g_signal_connect(adjustment, "changed", G_CALLBACK(onChatResize_gui), (gpointer)this);
//	g_signal_connect(getWidget("copyLinkItem"), "activate", G_CALLBACK(onCopyURIClicked_gui), (gpointer)this);
//	g_signal_connect(getWidget("openLinkItem"), "activate", G_CALLBACK(onOpenLinkClicked_gui), (gpointer)this);
//	g_signal_connect(getWidget("copyhubItem"), "activate", G_CALLBACK(onCopyURIClicked_gui), (gpointer)this);
//	g_signal_connect(getWidget("openhubItem"), "activate", G_CALLBACK(onOpenHubClicked_gui), (gpointer)this);
//	g_signal_connect(getWidget("copyMagnetItem"), "activate", G_CALLBACK(onCopyURIClicked_gui), (gpointer)this);
//	g_signal_connect(getWidget("searchMagnetItem"), "activate", G_CALLBACK(onSearchMagnetClicked_gui), (gpointer)this);
//	g_signal_connect(getWidget("magnetPropertiesItem"), "activate", G_CALLBACK(onMagnetPropertiesClicked_gui), (gpointer)this);
//	g_signal_connect(getWidget("removeUserItem"), "activate", G_CALLBACK(onRemoveUserItemClicked_gui), (gpointer)this);
//	g_signal_connect(getWidget("userListCheckButton"), "toggled", G_CALLBACK(onUserListToggled_gui), (gpointer)this);
//	g_signal_connect(getWidget("emotButton"), "button-release-event", G_CALLBACK(onEmotButtonRelease_gui), (gpointer)this);
	g_signal_connect(getWidget("buttonrefresh"), "clicked", G_CALLBACK(onRefreshUserListClicked_gui), (gpointer)this);

	// Set the pane position
	gint panePosition = SETTING(NICK_PANE_POS);
	gtk_paned_set_position(GTK_PANED(getWidget("pane")), /*width -*/ panePosition);

	history.push_back("");

	/* initial tags map */
	TagsMap[Tag::TAG_GENERAL] = createTag_gui("TAG_GENERAL", Tag::TAG_GENERAL);
	TagsMap[Tag::TAG_MYOWN] = createTag_gui("TAG_MYOWN", Tag::TAG_MYOWN);
	TagsMap[Tag::TAG_SYSTEM] = createTag_gui("TAG_SYSTEM", Tag::TAG_SYSTEM);
	TagsMap[Tag::TAG_STATUS] = createTag_gui("TAG_STATUS", Tag::TAG_STATUS);
	TagsMap[Tag::TAG_TIMESTAMP] = createTag_gui("TAG_TIMESTAMP", Tag::TAG_TIMESTAMP);
	TagsMap[Tag::TAG_CHEAT] = createTag_gui("TAG_CHEAT", Tag::TAG_CHEAT);
	/*-*/
	TagsMap[Tag::TAG_MYNICK] = createTag_gui("TAG_MYNICK", Tag::TAG_MYNICK);
	TagsMap[Tag::TAG_NICK] = createTag_gui("TAG_NICK", Tag::TAG_NICK);
	TagsMap[Tag::TAG_OPERATOR] = createTag_gui("TAG_OPERATOR", Tag::TAG_OPERATOR);
	TagsMap[Tag::TAG_FAVORITE] = createTag_gui("TAG_FAVORITE", Tag::TAG_FAVORITE);
	TagsMap[Tag::TAG_URL] = createTag_gui("TAG_URL", Tag::TAG_URL);
	TagsMap[Tag::TAG_IPADR] = createTag_gui("TAG_IPADR", Tag::TAG_IPADR);

	BoldTag = gtk_text_buffer_create_tag(chatBuffer, "TAG_WEIGHT", "weight", PANGO_WEIGHT_BOLD, NULL);
	UnderlineTag = gtk_text_buffer_create_tag(chatBuffer, "TAG_UNDERLINE", "underline", PANGO_UNDERLINE_SINGLE, NULL);
	ItalicTag = gtk_text_buffer_create_tag(chatBuffer, "TAG_STYLE", "style", PANGO_STYLE_ITALIC, NULL);

	// Initialize favorite users list
	FavoriteManager::FavoriteMap map = FavoriteManager::getInstance()->getFavoriteUsers();

	for (FavoriteManager::FavoriteMap::const_iterator it = map.begin(); it != map.end(); ++it)
	{
		if (it->second.getUrl() == address)
		{
			userFavoriteMap.insert(UserMap::value_type(it->first.toBase32(), it->second.getNick()));
		}
	}

	// set default select tag (fix error show cursor in neutral space).
	selectedTag = TagsMap[Tag::TAG_GENERAL];

	RecentHubEntry* rhe = FavoriteManager::getInstance()->getRecentHubEntry(address);

	if(!rhe) {
		RecentHubEntry entry;
		entry.setName("***");
		entry.setDescription("***");
		entry.setUsers("0");
		entry.setShared("0");
		entry.setServer(address);
		FavoriteManager::getInstance()->addRecent(entry);
	}

	if(p_faventry)
	{
		// It is a FavoriteHub
		bool bShowUserList = p_faventry->getShowUserList();

		if ( (bShowUserList == false) && gtk_widget_get_visible(getWidget("scrolledwindow2"))) {
			gtk_widget_set_visible(getWidget("scrolledwindow2") , FALSE);
		} 

		isFavBool = p_faventry->getNotify();
	}

	setColorsRows();
// signals
    GtkGesture *gesture;
    gesture = gtk_gesture_click_new ();
    gtk_gesture_single_set_button (GTK_GESTURE_SINGLE (gesture), 3);
    g_signal_connect (gesture, "pressed",
                    G_CALLBACK (on_right_btn_pressed), (gpointer)this);
    g_signal_connect (gesture, "released",
                    G_CALLBACK (on_right_btn_released), (gpointer)this);
    gtk_widget_add_controller (GTK_WIDGET(nickView.get()), GTK_EVENT_CONTROLLER (gesture));
//------- keys stuff
	GtkEventController* keys = gtk_event_controller_key_new ();
	g_signal_connect (keys, "key-pressed",
                    G_CALLBACK (key_pressed_gui), (gpointer)this);
	g_signal_connect (keys, "key-released",
                    G_CALLBACK (key_released_gui), (gpointer)this);
	gtk_widget_add_controller (GTK_WIDGET(getWidget("chatEntry")), GTK_EVENT_CONTROLLER (keys));

}

void Hub::on_right_btn_pressed (GtkGestureClick* /*gesture*/,
                                   int                /*n_press*/,
                                   double             x,
                                   double             y,
                                   gpointer         *data)
{
	Hub* hub = (Hub*)data;

	GMenu *menu = g_menu_new ();
	GMenuItem* item = g_menu_item_new("Browse Filelist", "hub.browse-fl" );
	g_menu_append_item(menu ,item);

	GMenuItem* match = g_menu_item_new("Match Filelist", "hub.match-fl" );
	g_menu_append_item(menu ,match);

	GMenuItem* pm = g_menu_item_new("Message To User", "hub.msg-to-user" );
	g_menu_append_item(menu ,pm);

	GMenuItem* grant = g_menu_item_new("Grant Slot", "hub.grant-slot" );
	g_menu_append_item(menu ,grant);

	GMenuItem* favu = g_menu_item_new("Favorite User","hub.add-fav-user");
	g_menu_append_item(menu ,favu);

	GMenuItem* ign = g_menu_item_new("Ignore User","hub.add-ignored");
	g_menu_append_item(menu ,ign);

	GMenuItem* ignr = g_menu_item_new("Remove Ignored User","hub.rem-ignored");
	g_menu_append_item(menu ,ignr);

	GtkWidget *pop = gtk_popover_menu_new_from_model(G_MENU_MODEL(menu));
	gtk_widget_set_parent(pop, GTK_WIDGET(hub->nickView.get()));
	gtk_popover_set_pointing_to(GTK_POPOVER(pop), &(const GdkRectangle){x,y,1,1});
	gtk_popover_popup (GTK_POPOVER(pop));
}


void Hub::on_right_btn_released (GtkGestureClick *gesture,
                                    int             /* n_press*/,
                                    double          /* x*/,
                                    double           /*y*/,
                                    GtkWidget*       /*widget*/)
{
  gtk_gesture_set_state (GTK_GESTURE (gesture),
                         GTK_EVENT_SEQUENCE_CLAIMED);
}

FavoriteHubEntry* Hub::getFavoriteHubEntry()
{
	return FavoriteManager::getInstance()->getFavoriteHubEntry(address);
}

gint Hub::sort_iter_compare_func_nick(GtkTreeModel *model, GtkTreeIter  *a,
									GtkTreeIter  *b,  gpointer  data)
{
	Hub* hub = (Hub *)data;
    bool bSortInt = hub->sort == "Client Type";
    gint ret = 0;
    if(!bSortInt)
    {
        g_autofree gchar *nick_a = NULL , *nick_b = NULL;
        gtk_tree_model_get(model, a, hub->nickView.col(hub->sort), &nick_a, -1);
        gtk_tree_model_get(model, b, hub->nickView.col(hub->sort), &nick_b, -1);
        ret = 0;
        if (nick_a == NULL || nick_b == NULL)
        {
            if (nick_a == NULL && nick_b == NULL)
                ret = 0;

            ret = (nick_a == NULL) ? -1 : 1;
        }
        else
        {
            g_autofree gchar* a_nick = g_utf8_casefold(nick_a,-1);
            g_autofree gchar* b_nick = g_utf8_casefold(nick_b,-1);
            ret = g_utf8_collate(a_nick,b_nick);
        }
        return ret;
    }
    else
    {
       gint gia = 0,gib = 0;
        gtk_tree_model_get(model, a, hub->nickView.col(hub->sort), gia, -1);
        gtk_tree_model_get(model, b, hub->nickView.col(hub->sort), gib, -1);
        ret = 0;
        if(gia == gib)
            return 0;
        if(gia > gib)
            return -1;
        if(gia < gib)
            return 1;
   }
        return ret;
}

void Hub::setColorsRows()
{
	setColorRow(_("Nick"));
	setColorRow(_("Shared"));
	setColorRow(_("Description"));
	setColorRow(_("Tag"));
	setColorRow(_("Connection"));
	setColorRow("IP");
	setColorRow(_("eMail"));
	setColorRow(_("Country"));
	setColorRow(_("Exact Share"));
	setColorRow(_("Slots"));
	setColorRow(_("Hubs"));
	setColorRow("PK");
	setColorRow(_("Cheat"));
	setColorRow(_("Generator"));
	setColorRow(_("Support"));
}

void Hub::setColorRow(const string cell)
{

	if(nickView.getCellRenderOf(cell) != NULL)
		gtk_tree_view_column_set_cell_data_func(nickView.getColumn(cell),
								nickView.getCellRenderOf(cell),
								Hub::makeColor,
								(gpointer)this,
								NULL);

	if(nickView.getCellRenderOf2(cell) != NULL)
		gtk_tree_view_column_set_cell_data_func(nickView.getColumn(cell),
								nickView.getCellRenderOf2(cell),
								Hub::makeColor,
								(gpointer)this,
								NULL);
}

void Hub::makeColor(GtkTreeViewColumn *column, GtkCellRenderer *cell, GtkTreeModel *model, GtkTreeIter *iter, gpointer data)
{
		Hub* hub = (Hub *)data;
		if(!hub)
			return;
		if(model == NULL)
			return;
		if(iter == NULL)
			return;
		if(column == NULL)
			return;
		if(cell == NULL)
			return;

		string color;

		string nick = hub->nickView.getString(iter,_("Nick"),model);
		uint64_t size = hub->nickView.getValue<gint64>(iter,_("Shared"),model);
		int type = hub->nickView.getValue<gint>(iter,_("Client Type"),model);

		switch(type)
		{
			case BOT:
			{
				color = WGETS("userlist-bg-bot-hub");
				break;
			}
			case OPERATOR:
			{
				color = WGETS("userlist-bg-operator");
				break;
			}
			case FAVORITE:
			{
     	   		color = WGETS("userlist-bg-favorite");
				break;
			}
			case IGNORED:
			{
				color = WGETS("userlist-bg-ignored");
				break;
			}
			case PROTECTED:
			{
				color = WGETS("userlist-bg-protected");
				break;
			}
			case PASIVE:
			{
				color = WGETS("userlist-bg-pasive");
				break;
			}
			case NORMAL:;
			default:
			{
			  color = WGETS("userlist-bg-normal");
			}
		}
//TODO:  UI
	bool isSet = false;

	if(hub->client && hub->client->get(SettingsManager::USE_HIGHLITING, SETTING(USE_HIGHLITING)))
	{
		ColorList* cl = HighlightManager::getInstance()->getList();
		for(auto& l:*cl)
		{
			if(l.isSet(ColorSettings::CONTEXT_NICKLIST)) {
				string match = l.getMatch();
				if( (nick == match) || (l.usingRegexp() && dcpp::RegEx::match<string>(nick,match))){
					if(l.getHasBgColor())
					{
						g_object_set(cell,"cell-background-set",TRUE,"cell-background",l.getBgColor().c_str(),NULL);
						isSet = true;
					}
					if(l.getHasFgColor())
					{
						//g_object_set(G_OBJECT(cell),"foreground-set",TRUE,"foreground",l.getFgColor().c_str(),NULL);
						isSet = true;
					}
				}
			}
		}
	}//END
		if(AVManager::getInstance()->isNickVirused(nick))
		{
			color = WGETS("userlist-bg-virus-an");
			AVManager::AVEntry entry = AVManager::getInstance()->getEntryByNick(nick);
			if(entry.ui64share == size) {
				color = WGETS("userlist-bg-virus"); // if size and nick is ok
			}
		}

		if(isSet == false) {
			g_object_set(cell,"cell-background-set",TRUE,"cell-background",color.c_str(),NULL);
		}
		string sizeString;
		const gchar* title = gtk_tree_view_column_get_title(column);

		if(strcmp(title,_("Shared")) == 0)
		{
			sizeString = dcpp::Util::formatBytes(size);
			g_object_set(cell, "text", sizeString.c_str(), NULL);
		}
}

void Hub::onSizeWindowState_gui(GtkWidget* ,GdkRectangle *allocation, gpointer data)
{
	Hub* hub = (Hub*)data;
	hub->width = allocation->width;
}

Hub::~Hub()
{
	RecentHubEntry* p_rhe = FavoriteManager::getInstance()->getRecentHubEntry(address);

	if(p_rhe)
	{
		p_rhe->setName(client->getHubName());
		p_rhe->setDescription(client->getHubDescription());
		p_rhe->setUsers(Util::toString(client->getUserCount()));
		p_rhe->setShared(Util::toString(client->getAvailable()));
		FavoriteManager::getInstance()->updateRecent(p_rhe);
	}

	bool showUL = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(getWidget("userListCheckButton")));
	FavoriteHubEntry* entry = getFavoriteHubEntry();
	string order = dcpp::Util::emptyString, hwidth = dcpp::Util::emptyString, visible = dcpp::Util::emptyString;
	nickView.saveSettings(order,hwidth,visible);

	SettingsManager* sm = SettingsManager::getInstance();

	if(entry) {
		entry->set(SettingsManager::HUB_UL_VISIBLE, visible);
		entry->set(SettingsManager::HUB_UL_ORDER, order);
		entry->set(SettingsManager::HUB_UL_SIZE, hwidth);
		entry->set(SettingsManager::EMOT_PACK, emotdialog->getCurrent(address));
		entry->setShowUserList(showUL);
		FavoriteManager::getInstance()->save();
	} else {
		//No Favorite, Save to main setting @Possible Made Enable/Disable of this also ?
		sm->set(SettingsManager::HUB_UL_ORDER, order);
		sm->set(SettingsManager::HUB_UL_SIZE, hwidth);
		sm->set(SettingsManager::HUB_UL_VISIBLE ,visible);
	}

	disconnect_client(true);

	// Save the pane position
	gint panePosition = /*width -*/ gtk_paned_get_position(GTK_PANED(getWidget("pane")));
	sm->set(SettingsManager::NICK_PANE_POS, panePosition);

}

void Hub::show()
{
	// Connect to the hub
	connectClient_client(address, encoding);
}

void Hub::selection_changed_userlist_gui(GtkTreeSelection*, GtkWidget *ptree_view)
{
	gtk_widget_trigger_tooltip_query (ptree_view);
}

void Hub::columnHeader(int num, string name)
{
	GtkTreeViewColumn *col = gtk_tree_view_get_column (nickView.get(), num);
	gtk_tree_view_column_set_clickable (col, TRUE);
	g_object_set ( gtk_tree_view_column_get_button(col), "tooltip-text", name.c_str(), NULL);
}

void Hub::set_Header_tooltip_gui()
{
	columnHeader(nickView.col(_("Nick")),_("Nick"));
	columnHeader(nickView.col(_("Shared")),_("Shared"));
	columnHeader(nickView.col(_("Description")), _("Description"));
	columnHeader(nickView.col(_("Tag")), _("Tag"));
	columnHeader(nickView.col(_("Connection")), _("Connection"));
	columnHeader(nickView.col("IP"), "IP");
	columnHeader(nickView.col(_("eMail")), _("eMail"));
	columnHeader(nickView.col(_("Country")), _("Country"));
	columnHeader(nickView.col(_("Exact Share")), _("Exact Share"));
	columnHeader(nickView.col(_("Slots")), _("Slots"));
	columnHeader(nickView.col(_("Hubs")),_("Hubs"));
	columnHeader(nickView.col("PK"),"PK");
	columnHeader(nickView.col(_("Cheat")),_("Cheat"));
	columnHeader(nickView.col(_("Generator")),_("Generator"));
	columnHeader(nickView.col(_("Support")),_("Support"));
}

gboolean Hub::onUserListTooltip_gui(GtkWidget *widget, gint x, gint y, gboolean keyboard_tip, GtkTooltip *_tooltip, gpointer data)
{
	Hub* hub = (Hub*)data;
	if(!hub) return FALSE;
	if(!_tooltip) return FALSE;

	GtkTreeIter iter;
	GtkTreeView *tree_view = GTK_TREE_VIEW (widget);
	GtkTreeModel *model = gtk_tree_view_get_model (tree_view);
	GtkTreePath *path = NULL;

	if (!gtk_tree_view_get_tooltip_context (tree_view, x, y,
					  keyboard_tip,
					  &model, &path, &iter))
	return FALSE;

	g_autofree gchar *nick = NULL,
	*tag = NULL, *desc = NULL,*con = NULL,
	*ip = NULL,*e = NULL,*country = NULL,*slots = NULL,*hubs = NULL,
	*pk = NULL,*cheat = NULL,*gen = NULL,*sup = NULL,*cid = NULL;
	gint type = 0;
	gint64 ssize = 0;
	char buffer[1000];

	gtk_tree_model_get (model, &iter, hub->nickView.col(_("Nick")), &nick,
									hub->nickView.col(_("Description")), &desc,
									hub->nickView.col(_("Tag")), &tag,
									hub->nickView.col(_("Connection")), &con,
									hub->nickView.col("IP"), &ip,
									hub->nickView.col(_("eMail")), &e,
									hub->nickView.col(_("Country")), &country,
									hub->nickView.col(_("Exact Share")), &ssize,
									hub->nickView.col(_("Slots")), &slots,
									hub->nickView.col(_("Hubs")), &hubs,
									hub->nickView.col("PK"), &pk,
									hub->nickView.col(_("Cheat")), &cheat,
									hub->nickView.col(_("Generator")), &gen,
									hub->nickView.col(_("Support")), &sup,
									hub->nickView.col("CID"), &cid,
									hub->nickView.col("Client Type"),&type,
									-1);

	string sharesize  = Util::formatBytes(ssize);
	g_snprintf (buffer, 1000, " Nick: %s\n Connection: %s\n Description: %s\n Tag: %s\n Share: %s\n IP: %s\n eMail: %s\nCountry: %s\n Slots: %s\n Hubs: %s\n PK: %s\n Cheat: %s\n Generator: %s\n Support: %s\n CID: %s,\n Type %d", nick, con,desc, tag , sharesize.c_str() ,ip, e, country, slots, hubs, pk, cheat, gen, sup, cid,type);
	gtk_tooltip_set_text (_tooltip, g_filename_to_utf8(buffer,-1,NULL,NULL,NULL));
	if(path == NULL) return FALSE;
	if(tree_view == NULL) return FALSE;

	gtk_tree_view_set_tooltip_row (tree_view, _tooltip, path);
	gtk_tree_path_free (path);
	return TRUE;
}

void Hub::setStatus_gui(string statusBar, string text)
{
	if (!statusBar.empty() && !text.empty())
	{
		if (statusBar == "statusMain")
		{
			text = "[" + Util::getShortTimeString() + "] " + text;

            if(statustext.size() > (uint32_t)WGETI("max-tooltips"))
			{
			    statustext.pop();
			}

			queue<string> tmp = statustext;
            string statusTextOnToolTip;

			while(!tmp.empty())
            {
                	statusTextOnToolTip += "\n" + tmp.front();
                	tmp.pop();
			}

			statustext.push(text);
            statusTextOnToolTip += "\n" + text;

            gtk_widget_set_tooltip_text(getWidget("statusMain"), g_filename_to_utf8(statusTextOnToolTip.c_str(),-1,NULL,NULL,NULL));
		}

		gtk_statusbar_pop(GTK_STATUSBAR(getWidget(statusBar)), 0);
		gtk_statusbar_push(GTK_STATUSBAR(getWidget(statusBar)), 0, g_filename_to_utf8(text.c_str(),-1,NULL,NULL,NULL));
	}
}

bool Hub::findUser_gui(const string &cid, GtkTreeIter *iter)
{
	UnMapIter::const_iterator it = userIters.find(cid);

	if (it != userIters.end())
	{
		if (iter)
			*iter = it->second;

		return true;
	}

	return false;
}

bool Hub::findNick_gui(const string &nick, GtkTreeIter *iter)
{
	unordered_map<string, string>::const_iterator it = userMap.find(nick);

	if (it != userMap.end())
		return findUser_gui(it->second, iter);

	return false;
}

void Hub::updateUser_gui(ParamMap params)
{
	GtkTreeIter iter;
	uint64_t shared = Util::toInt64(params["Shared"]);
	const string& cid = params["CID"];
	const string icon = "bmdc-" + params["Icon"];
	const gchar* Nick = g_filename_to_utf8(params["Nick"].c_str(),-1,NULL,NULL,NULL);
	const string& nickOrder = params["Nick Order"];
	bool favorite = userFavoriteMap.find(cid) != userFavoriteMap.end();

	if (findUser_gui(cid, &iter))
	{
		totalShared += shared - nickView.getValue<int64_t>(&iter, _("Shared"));
		string nick = nickView.getString(&iter, _("Nick"));

		if (nick != params["Nick"])
		{
			// User has changed nick, update userMap and remove the old Nick tag
			userMap.erase(nick);
			removeTag_gui(nick);
			userMap.insert(UserMap::value_type(params["Nick"], cid));

			// update favorite
			if (favorite)
				userFavoriteMap[cid] = Nick;
		}

		gtk_list_store_set(nickStore, &iter,
			nickView.col(_("Nick")), Nick,
			nickView.col(_("Shared")), shared,
			nickView.col(_("Description")), params["Description"].c_str(),
			nickView.col(_("Tag")), params["Tag"].c_str(),
 			nickView.col(_("Connection")), params["Connection"].c_str(),
			nickView.col("IP"), params["IP"].c_str(),
			nickView.col(_("eMail")), params["eMail"].c_str(),
			nickView.col(_("Country")), params["Country"].c_str(),
			nickView.col(_("Exact Share")), shared,
			nickView.col(_("Slots")), params["Slots"].c_str(),
			nickView.col(_("Hubs")), params["Hubs"].c_str(),
			nickView.col("PK"), params["PK"].c_str(),
			nickView.col(_("Cheat")), params["Cheat"].c_str(),
			nickView.col(_("Generator")), params["Generator"].c_str(),
			nickView.col(_("Support")), params["Support"].c_str(),
			nickView.col("Icon"), icon.c_str(),
			nickView.col("Nick Order"), nickOrder.c_str(),
			nickView.col("CID"), cid.c_str(),
			nickView.col("NickColor"), params["NickColor"].c_str(),
			nickView.col("Pixbuf"), WulforUtil::LoadCountryPixbuf(params["Abbrevation"]),
			nickView.col("Client Type"), Util::toInt(params["Type"]),
			-1);
	}
	else
	{
		totalShared += shared;
		userMap.insert(UserMap::value_type(string(Nick), cid));

		gtk_list_store_insert_with_values(nickStore, &iter, userMap.size(),
			nickView.col(_("Nick")), Nick,
			nickView.col(_("Shared")), shared,
			nickView.col(_("Description")), params["Description"].c_str(),
			nickView.col(_("Tag")), params["Tag"].c_str(),
 			nickView.col(_("Connection")), params["Connection"].c_str(),
			nickView.col("IP"), params["IP"].c_str(),
			nickView.col(_("eMail")), params["eMail"].c_str(),
            nickView.col(_("Country")), params["Country"].c_str(),
            nickView.col(_("Exact Share")), shared,
            nickView.col(_("Slots")), params["Slots"].c_str(),
            nickView.col(_("Hubs")), params["Hubs"].c_str(),
            nickView.col("PK"), params["PK"].c_str(),
            nickView.col(_("Cheat")), params["Cheat"].c_str(),
            nickView.col(_("Generator")), params["Generator"].c_str(),
            nickView.col(_("Support")), params["Support"].c_str(),
			nickView.col("Icon"), icon.c_str(),
			nickView.col("Nick Order"), nickOrder.c_str(),
			nickView.col("CID"), cid.c_str(),
			nickView.col("NickColor"), params["NickColor"].c_str(),
            nickView.col("Pixbuf"),WulforUtil::LoadCountryPixbuf(params["Abbrevation"]),
            nickView.col("Client Type"), Util::toInt( params["Type"]),
			-1);

		userIters.insert(UnMapIter::value_type(cid, iter));

		if(client && client->get(SettingsManager::SHOW_JOINS,SETTING(SHOW_JOINS)))
		{
			// Show joins in chat by default
			addStatusMessage_gui(string(Nick) + _(" has joined"), Msg::STATUS, favorite ? Sound::FAVORITE_USER_JOIN : Sound::NONE);
			string message = string(Nick) + _(" has joined hub ") + client->getHubName();
			WulforManager::get()->getMainWindow()->addPrivateStatusMessage_gui(Msg::STATUS, cid, message);

			if (favorite)
				Notify::get()->showNotify("", message, Notify::FAVORITE_USER_JOIN);
		}
		else if ( client && ( (client->get(SettingsManager::FAV_SHOW_JOINS,SETTING(FAV_SHOW_JOINS))) && favorite ))
		{
			// Only show joins for favorite users
			string message = string(Nick) + _(" has joined hub ") + client->getHubName();
			addStatusMessage_gui(string(Nick) + _(" has joined"), Msg::STATUS, Sound::FAVORITE_USER_JOIN);
			WulforManager::get()->getMainWindow()->addPrivateStatusMessage_gui(Msg::STATUS, cid, message);
			Notify::get()->showNotify("", message, Notify::FAVORITE_USER_JOIN);
		}
	}

	if( !params["IP"].empty()) {

		auto list = FavoriteManager::getInstance()->getListIp();
		auto i = list.find(params["IP"]);
		if( i != list.end()) {

			addStatusMessage_gui(params["IP"] + _(" Has been connected "), Msg::STATUS, Sound::NONE);

		}
		for(auto& b:list)
		{
			if(b.second->isSet(FavoriteUser::FLAG_IP_RANGE) && dcpp::bmUtil::isIpInRange(params["IP"],b.first))
				addStatusMessage_gui(params["IP"] +_(" From range")+b.first+ _(" Has been connected "), Msg::STATUS, Sound::NONE);
		}
	}

	setStatus_gui("statusUsers", Util::toString(userMap.size()) + _(" Users"));
	setStatus_gui("statusShared", Util::formatBytes(totalShared));
}

void Hub::removeUser_gui(string cid)
{
	GtkTreeIter iter;
	string nick;
	gint order;

	if (findUser_gui(cid, &iter))
	{
		order = nickView.getValue<gint>(&iter, "Client Type");
		nick = nickView.getString(&iter, _("Nick"));
		totalShared -= nickView.getValue<int64_t>(&iter, _("Shared"));
		gtk_list_store_remove(nickStore, &iter);
		removeTag_gui(nick);
		userMap.erase(nick);
		userFavoriteMap.erase(nick);
		userIters.erase(cid);
		setStatus_gui("statusUsers", Util::toString(userMap.size()) + _(" Users"));
		setStatus_gui("statusShared", Util::formatBytes(totalShared));
		//There is no reason do this if client == null
		if (client && client->get(SettingsManager::SHOW_JOINS, SETTING(SHOW_JOINS)))
		{
			// Show parts in chat
			string message = nick + _(" has quit hub ") + client->getHubName();
			addStatusMessage_gui(nick + _(" has quit"), Msg::STATUS, (order == FAVORITE) ? Sound::FAVORITE_USER_QUIT : Sound::NONE);
			WulforManager::get()->getMainWindow()->addPrivateStatusMessage_gui(Msg::STATUS, cid, message);

			if (order == FAVORITE)
				Notify::get()->showNotify("", message, Notify::FAVORITE_USER_QUIT);
		}
		else if ( client && ( (client->get(SettingsManager::FAV_SHOW_JOINS,SETTING(FAV_SHOW_JOINS))) && (order == FAVORITE)))
		{
			// Only show parts for favorite users
			string message = nick + _(" has quit hub ") + client->getHubName();
			addStatusMessage_gui(nick + _(" has quit"), Msg::STATUS, Sound::FAVORITE_USER_QUIT);
			WulforManager::get()->getMainWindow()->addPrivateStatusMessage_gui(Msg::STATUS, cid, message);
			Notify::get()->showNotify("", message, Notify::FAVORITE_USER_QUIT);
		}
	}
}

/*
 * Remove nick tag from text view
 */
void Hub::removeTag_gui(const string &nick)
{
	GtkTextTagTable *textTagTable = gtk_text_buffer_get_tag_table(chatBuffer);
	GtkTextTag *tag = gtk_text_tag_table_lookup(textTagTable, (tagPrefix + nick).c_str());
	if (tag)
		gtk_text_tag_table_remove(textTagTable, tag);
}

void Hub::clearNickList_gui()
{
	// Remove all old nick tags from the text view
	unordered_map<string, string>::const_iterator it;
	for (it = userMap.begin(); it != userMap.end(); ++it)
		removeTag_gui(it->first);

	gtk_list_store_clear(nickStore);
	userMap.clear();
	userFavoriteMap.clear();

	userIters.clear();
	totalShared = 0;
	setStatus_gui("statusUsers", _("0 Users"));
	setStatus_gui("statusShared", "0 B");
}

//@TODO:rem
void Hub::popupNickMenu_gui()
{
	// Build user command menu
	userCommandMenu->cleanMenu_gui();
	ignoreMenu->cleanMenu_gui();

	GtkTreeIter iter;
	GList *list = gtk_tree_selection_get_selected_rows(nickSelection, NULL);
	string nicks = dcpp::Util::emptyString;
	string cid = dcpp::Util::emptyString;
	string ip = "0.0.0.0";
	string lastNick = dcpp::Util::emptyString;

	for (GList *i = list; i; i = i->next)
	{
		GtkTreePath *path = (GtkTreePath *)i->data;
		if (gtk_tree_model_get_iter(GTK_TREE_MODEL(nickStore), &iter, path))
		{
			cid = nickView.getString(&iter, "CID");
			ip = nickView.getString(&iter,_("IP"));
			lastNick = nickView.getString(&iter, _("Nick"));
			userCommandMenu->addUser(cid);

			nicks += " " + lastNick;
		}
		gtk_tree_path_free(path);
	}
	g_list_free(list);

	userCommandMenu->addHub(client->getHubUrl());
	userCommandMenu->buildMenu_gui();
	string color = WGETS("menu-userlist-color");//@ Settings of UserList Menu  text color (1st item)
	g_autofree gchar *markup = g_markup_printf_escaped ("<span fgcolor=\"%s\" ><b>%s</b></span>",color.c_str(),nicks.c_str());
}
//END
void Hub::getPassword_gui()
{
	if(!SETTING(PROMPT_PASSWORD))
	{
		addStatusMessage_gui(_("Waiting for input password (don't remove /password before your password)"), Msg::STATUS, Sound::NONE);

		gint pos = 0;
		GtkWidget *chatEntry = getWidget("chatEntry");
		gtk_editable_delete_text(GTK_EDITABLE(chatEntry), pos, -1);
		gtk_editable_insert_text(GTK_EDITABLE(chatEntry), "/password ", -1, &pos);
		gtk_editable_set_position(GTK_EDITABLE(chatEntry), pos);

		if (!WaitingPassword)
			WaitingPassword = true;
		return;
	}

	if (PasswordDialog)
		return;
	// Create password dialog
	string title = _("Enter hub password for ") + client->getHubUrl();
	GtkWidget *dialog = gtk_dialog_new_with_buttons(title.c_str(),
		GTK_WINDOW(WulforManager::get()->getMainWindow()->getContainer()),
		GTK_DIALOG_DESTROY_WITH_PARENT,
		"_OK",
		GTK_RESPONSE_OK,
		"_Cancel",
		GTK_RESPONSE_CANCEL,
		NULL);
	gtk_window_set_modal(GTK_WINDOW(dialog), TRUE);
	gtk_dialog_set_default_response(GTK_DIALOG(dialog), GTK_RESPONSE_OK);
	GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL,0);

	GtkWidget *entry = gtk_entry_new();
	g_object_set(entry, "can-focus", TRUE, "visibility", FALSE, "activates-default", TRUE, NULL);

	gtk_box_append(GTK_BOX(box), entry);

	GtkWidget *frame = gtk_frame_new(NULL);
	g_object_set(frame, "border-width", 8, NULL);

	GtkWidget *label = gtk_label_new(NULL);
	gtk_label_set_markup(GTK_LABEL(label), _("<b>Enter your password</b>"));
	gtk_frame_set_label_widget(GTK_FRAME(frame), label);

	gtk_frame_set_child(GTK_FRAME(frame), box);
	gtk_box_append(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog)))  , frame);

	g_object_set_data(G_OBJECT(dialog), "password-entry", (gpointer) entry);

	g_signal_connect(dialog, "response", G_CALLBACK(onPasswordDialog), (gpointer) this);

	gtk_widget_show(dialog);

	PasswordDialog = true;
	WaitingPassword = true;
}

void Hub::onPasswordDialog(GtkWidget *dialog, gint response, gpointer data)
{
	Hub *hub = (Hub *) data;

	GtkWidget *entry = (GtkWidget *)g_object_get_data(G_OBJECT(dialog), "password-entry");

	if (response == GTK_RESPONSE_OK)
	{
		string password = gtk_editable_get_text(GTK_EDITABLE(entry));
		hub->setPassword_client(password);
	}
	else
		hub->client->disconnect(true);

	gtk_widget_hide(dialog);
	hub->PasswordDialog = false;
	hub->WaitingPassword = false;
}

void Hub::addStatusMessage_gui(string message, Msg::TypeMsg typemsg, Sound::TypeSound sound)
{
	if (!message.empty())
	{
		if (sound != Sound::NONE)
			Sound::get()->playSound(sound);

		setStatus_gui("statusMain", message);

		if( client && client->get(SettingsManager::STATUS_IN_CHAT, SETTING(STATUS_IN_CHAT)))
		{
			string line = "*** " + message;
			addMessage_gui("", line, typemsg, "");
			return;
		}
		//fallback...
		if( SETTING(STATUS_IN_CHAT) )
		{
			string line = "*** " + message;
			addMessage_gui("", line, typemsg, "");
		}
	}
}

void Hub::nickToChat_gui(const string &nick)
{
	if (!gtk_widget_is_focus(getWidget("chatEntry")))
		gtk_widget_grab_focus(getWidget("chatEntry"));

	gint pos = gtk_editable_get_position(GTK_EDITABLE(getWidget("chatEntry")));
	gtk_editable_insert_text(GTK_EDITABLE(getWidget("chatEntry")), (nick + (!pos? ": " : " ")).c_str(), -1, &pos);
	gtk_editable_set_position(GTK_EDITABLE(getWidget("chatEntry")), pos);
}

void Hub::addMessage_gui(string cid, string message, Msg::TypeMsg typemsg, string sCountry)
{
	message = message.c_str();
	if (message.empty())
		return;

	GtkTextIter iter;
	string line = dcpp::Util::emptyString;

	// Add a new line if this isn't the first line in buffer.
	if(gtk_text_buffer_get_line_count(chatBuffer) >= 1)
		line += "\n";

	if( client && client->get(SettingsManager::TIME_STAMPS, SETTING(TIME_STAMPS)))
			line += string(g_filename_to_utf8( string("[" + Util::getShortTimeString() + "] ").c_str(),-1,NULL,NULL,NULL));
	#ifdef _WIN32
		line += string(g_filename_to_utf8(message.c_str(),-1,NULL,NULL,NULL));
	#else
		line += message;
	#endif

	gtk_text_buffer_get_end_iter(chatBuffer, &iter);
	gtk_text_buffer_insert(chatBuffer, &iter, line.c_str(), line.size());

	switch (typemsg)
	{
		case Msg::MYOWN:
			tagMsg = Tag::TAG_MYOWN;
			break;

		case Msg::SYSTEM:
			tagMsg = Tag::TAG_SYSTEM;
			break;

		case Msg::STATUS:
			tagMsg = Tag::TAG_STATUS;
			break;

		case Msg::CHEAT:
			tagMsg = Tag::TAG_CHEAT;
			break;

		case Msg::GENERAL:

		default:
		{
			tagMsg = Tag::TAG_GENERAL;

			if(isFavBool)
			{
				MainWindow *mw = WulforManager::get()->getMainWindow();
				typedef Func3<MainWindow, string, string, Notify::TypeNotify> F3;
				F3 *func = new F3(mw,&MainWindow::showNotification_gui, client->getHubName(), message, Notify::HUB_CHAT);
				WulforManager::get()->dispatchGuiFunc(func);
			}
		}
	}

	totalEmoticons = 0;

	applyTags_gui(cid, line,sCountry);

	gtk_text_buffer_get_end_iter(chatBuffer, &iter);

	// Limit size of chat text
	if ( gtk_text_buffer_get_line_count(chatBuffer) > maxLines + 1)
	{
		GtkTextIter next;
		gtk_text_buffer_get_start_iter(chatBuffer, &iter);
		gtk_text_buffer_get_iter_at_line(chatBuffer, &next, 1);
		gtk_text_buffer_delete(chatBuffer, &iter, &next);
	}
	gtk_text_view_scroll_mark_onscreen (GTK_TEXT_VIEW(getWidget("chatText")), end_mark);//this might need?

}

void Hub::applyTags_gui(const string cid, const string line,string sCountry)
{
	GtkTextIter start_iter;
	gtk_text_buffer_get_end_iter(chatBuffer, &start_iter);

	// apply timestamp tag
	if (client && client->get(SettingsManager::TIME_STAMPS,SETTING(TIME_STAMPS)))
	{
		gtk_text_iter_backward_chars(&start_iter,
			g_utf8_strlen(line.c_str(), -1) - g_utf8_strlen(Util::getShortTimeString().c_str(), -1) - 3);

		GtkTextIter ts_start_iter, ts_end_iter;
		ts_end_iter = start_iter;

		gtk_text_buffer_get_end_iter(chatBuffer, &ts_start_iter);
		gtk_text_iter_backward_chars(&ts_start_iter, g_utf8_strlen(line.c_str(), -1));

		gtk_text_buffer_apply_tag(chatBuffer, TagsMap[Tag::TAG_TIMESTAMP], &ts_start_iter, &ts_end_iter);
	}
	else
		gtk_text_iter_backward_chars(&start_iter, g_utf8_strlen(line.c_str(), -1));

	//
	if( ( (!sCountry.empty()) &&  client && client->get(SettingsManager::GET_USER_COUNTRY,SETTING(GET_USER_COUNTRY))) )
	{
		gtk_text_buffer_get_end_iter(chatBuffer,&start_iter);

		GtkTextIter iter = start_iter;
		gtk_text_iter_starts_line (&start_iter);
		gtk_text_iter_forward_char (&start_iter);

		gtk_text_iter_backward_to_tag_toggle(&start_iter,TagsMap[Tag::TAG_TIMESTAMP]);
		gtk_text_buffer_insert_paintable(chatBuffer,&start_iter,WulforUtil::convertPixBuf(WulforUtil::LoadCountryPixbuf(sCountry)));

	}
	// apply tags: nick, link, hub-url, magnet
	GtkTextIter tag_start_iter, tag_end_iter;

	gtk_text_buffer_move_mark(chatBuffer, start_mark, &start_iter);
	gtk_text_buffer_move_mark(chatBuffer, end_mark, &start_iter);

	string tagName;
	Tag::TypeTag tagStyle = Tag::TAG_GENERAL;

	bool firstNick = false;
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
			gtk_text_buffer_move_mark(chatBuffer, start_mark, &start_iter);
			gtk_text_buffer_move_mark(chatBuffer, end_mark, &start_iter);

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
		bool isNick = false;
		gchar *temp = gtk_text_iter_get_text(&tag_start_iter, &tag_end_iter);

		if(client && client->get(SettingsManager::USE_HIGHLITING, SETTING(USE_HIGHLITING)))
		{
			GtkTextTag *tag = gtk_text_tag_table_lookup(gtk_text_buffer_get_tag_table(chatBuffer), temp);
			bool isTab = false;

			if(WulforUtil::isHighlightingWord(chatBuffer,tag, string(temp), isTab, (gpointer)this))
			{
				gtk_text_buffer_apply_tag(chatBuffer, tag, &tag_start_iter, &tag_end_iter);
				if(isTab)
				{
					typedef Func0<Hub> F0;
					F0 *func = new F0(this, &Hub::setUrgent_gui);
					WulforManager::get()->dispatchGuiFunc(func);

				}
			}
		}

		if (!C_EMPTY(temp))
		{
			tagName = temp;
			g_free(temp);
			GtkTreeIter iter;

			// Special case: catch nicks in the form <nick> at the beginning of the line.
			if (!firstNick && tagName[0] == '<' && tagName[tagName.size() - 1] == '>')
			{
				tagName = tagName.substr(1, tagName.size() - 2);
				firstNick = TRUE;
			}

			if (findNick_gui(tagName, &iter))
			{
				isNick = TRUE;
				//callback = G_CALLBACK(onNickTagEvent_gui);
				gint order = nickView.getValue<gint>(&iter, "Client Type");

				if (tagName == client->getMyNick())
					tagStyle = Tag::TAG_MYNICK;
				else if (order == FAVORITE)
					tagStyle = Tag::TAG_FAVORITE;
				else if ( (order == BOT) || (order == OPERATOR))//bot fix
					tagStyle = Tag::TAG_OPERATOR;
				else if (order == NORMAL || order == IGNORED || order == PROTECTED || order == PASIVE)
					tagStyle = Tag::TAG_NICK;

				tagName = tagPrefix + tagName;
			}
			else
			{
			   bool notlink = false;

				if (!notlink)
				{
					/*if (WulforUtil::isLink(tagName))
					{
						callback = G_CALLBACK(onLinkTagEvent_gui);
					}
					else if (WulforUtil::isHubURL(tagName))
					{
						callback = G_CALLBACK(onHubTagEvent_gui);
					}
					else if (WulforUtil::isMagnet(tagName))
					{
						callback = G_CALLBACK(onMagnetTagEvent_gui);
					}*/

					tagStyle = Tag::TAG_URL;
				}

				if(!notlink &&  WulforUtil::HitIP(tagName))
				{
//					callback = G_CALLBACK(onIpTagEvent_gui);
					tagStyle = Tag::TAG_IPADR;
					isIp = true;
					//userCommandMenu2->cleanMenu_gui();
					//userCommandMenu2->addIp(tagName);
					//userCommandMenu2->addHub(address);
					//userCommandMenu2->buildMenu_gui();
				}
			}
		}

		if (callback)
		{
			gtk_text_buffer_move_mark(chatBuffer, tag_mark, &tag_end_iter);

			// check for the tags in our buffer
			GtkTextTag *tag = gtk_text_tag_table_lookup (gtk_text_buffer_get_tag_table(chatBuffer), tagName.c_str());

			if (!tag)
			{
				if (isNick || isIp)
					tag = gtk_text_buffer_create_tag(chatBuffer, tagName.c_str(), NULL);
				else
					tag = gtk_text_buffer_create_tag(chatBuffer, tagName.c_str(), "underline", PANGO_UNDERLINE_SINGLE, NULL);

				g_signal_connect(tag, "event", callback, (gpointer)this);
			}

			/* apply tags */
			if (/*callback == G_CALLBACK(onMagnetTagEvent_gui) && WGETB("use-magnet-split")*/false)
			{
				string line;

				if (WulforUtil::splitMagnet(tagName, line))
				{
					dcassert(tagStyle == Tag::TAG_URL);

					gtk_text_buffer_delete(chatBuffer, &tag_start_iter, &tag_end_iter);
					gtk_text_buffer_insert_with_tags(chatBuffer, &tag_start_iter,
						line.c_str(), line.size(), tag, TagsMap[tagStyle], NULL);
				}
			}
			else
			{
				dcassert(tagStyle >= Tag::TAG_FIRST && tagStyle < Tag::TAG_LAST);

				gtk_text_buffer_apply_tag(chatBuffer, tag, &tag_start_iter, &tag_end_iter);
				gtk_text_buffer_apply_tag(chatBuffer, TagsMap[tagStyle], &tag_start_iter, &tag_end_iter);
			}

			applyEmoticons_gui();

			gtk_text_buffer_get_iter_at_mark(chatBuffer, &start_iter, tag_mark);

			if (gtk_text_iter_is_end(&start_iter))
				return;

			start = false;
		}
		else
		{
			if (gtk_text_iter_is_end(&start_iter))
			{
				if (!gtk_text_iter_equal(&tag_start_iter, &tag_end_iter))
					gtk_text_buffer_move_mark(chatBuffer, end_mark, &tag_end_iter);

				applyEmoticons_gui();

				break;
			}

			gtk_text_buffer_move_mark(chatBuffer, end_mark, &tag_end_iter);
		}
	}
}

void Hub::applyEmoticons_gui()
{
	GtkTextIter start_iter, end_iter;

	gtk_text_buffer_get_iter_at_mark(chatBuffer, &start_iter, start_mark);
	gtk_text_buffer_get_iter_at_mark(chatBuffer, &end_iter, end_mark);

	if(gtk_text_iter_equal(&start_iter, &end_iter))
		return;

	/* apply general tag */
	dcassert(tagMsg >= Tag::TAG_GENERAL && tagMsg < Tag::TAG_TIMESTAMP);
	gtk_text_buffer_apply_tag(chatBuffer, TagsMap[tagMsg], &start_iter, &end_iter);

	/* emoticons */
	if (tagMsg == Tag::TAG_SYSTEM || tagMsg == Tag::TAG_STATUS)
	{
		return;
	}
	else if (!emotdialog->getEmot(address)->useEmoticons_gui())
	{
		if (SETTING(USE_EMOTS))
			setStatus_gui("statusMain", _(" *** Emoticons not loaded"));
		return;
	}
	else if (!useEmoticons)
	{
		setStatus_gui("statusMain", _(" *** Emoticons mode off"));
		return;
	}
	else if (totalEmoticons >= EMOTICONS_MAX)
	{
		setStatus_gui("statusMain", _(" *** Emoticons limit"));
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
	Emot::List &list = emotdialog->getEmot(address)->getPack_gui();

	/* set start mark */
	gtk_text_buffer_move_mark(chatBuffer, emot_mark, &start_iter);

	for (;;)
	{
		/* get start and end iter positions at marks */
		gtk_text_buffer_get_iter_at_mark(chatBuffer, &start_iter, emot_mark);
		gtk_text_buffer_get_iter_at_mark(chatBuffer, &end_iter, end_mark);

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
						gtk_text_buffer_get_iter_at_mark(chatBuffer, &tmp_end_iter, end_mark);
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
				setStatus_gui("statusMain", _(" *** Emoticons limit"));
				return;
			}

			/* delete text-emoticon and insert pixbuf-emoticon */
			gtk_text_buffer_delete(chatBuffer, &p_start, &p_end);
			if(*p_it)
			{
				GtkImage* image = GTK_IMAGE(gtk_image_new_from_pixbuf((*p_it)->getPixbuf()));
				GdkPaintable* paint = gtk_image_get_paintable(GTK_IMAGE(image));
				gtk_text_buffer_insert_paintable(chatBuffer, &p_start,paint);
			}


			searchEmoticons++;
			totalEmoticons++;

			/* set emoticon mark to start */
			gtk_text_buffer_move_mark(chatBuffer, emot_mark, &p_start);

			/* check full emoticons */
			gtk_text_buffer_get_iter_at_mark(chatBuffer, &start_iter, start_mark);
			gtk_text_buffer_get_iter_at_mark(chatBuffer, &end_iter, end_mark);

			if (gtk_text_iter_get_offset(&end_iter) - gtk_text_iter_get_offset(&start_iter) == searchEmoticons - 1)
				return;
		}
		else
			return;
	}
}

/*
 * Unfortunately, we can't underline the tag on mouse over since it would
 * underline all the tags with that name.
 */
void Hub::updateCursor_gui(GtkWidget *widget)
{
	gint x, y, buf_x, buf_y;
	GtkTextIter iter;
	GSList *tagList = NULL;
	GtkTextTag *newTag = NULL;
	GdkDevice *dev = NULL;
#if !GTK_CHECK_VERSION(3,20,0)
	GdkDeviceManager *device_manager = NULL;
	device_manager = gdk_display_get_device_manager (gdk_window_get_display (gtk_widget_get_window(widget)));
	dev = gdk_device_manager_get_client_pointer (device_manager);
#else
	GdkDisplay* win = gtk_widget_get_display(widget);
	GdkSeat* seat = gdk_display_get_default_seat(win);
	dev = gdk_seat_get_pointer(seat);
#endif
	// Check for tags under the cursor, and change mouse cursor appropriately
	gtk_text_view_window_to_buffer_coords(GTK_TEXT_VIEW(widget), GTK_TEXT_WINDOW_WIDGET, x, y, &buf_x, &buf_y);
	gtk_text_view_get_iter_at_location(GTK_TEXT_VIEW(widget), &iter, buf_x, buf_y);
	tagList = gtk_text_iter_get_tags(&iter);

	if (tagList != NULL)
	{
		newTag = GTK_TEXT_TAG(tagList->data);

		if (find(TagsMap + Tag::TAG_GENERAL, TagsMap + Tag::TAG_LAST, newTag) != TagsMap + Tag::TAG_LAST)
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
			selectedTagStr = WulforUtil::getTagName(newTag);

			if (find(TagsMap, TagsMap + Tag::TAG_MYNICK, newTag) == TagsMap + Tag::TAG_MYNICK)
			{
				// Cursor was in neutral space.
				//gdk_window_set_cursor(gtk_text_view_get_window(GTK_TEXT_VIEW(widget), GTK_TEXT_WINDOW_TEXT), handCursor);
			}
			//else
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

void Hub::preferences_gui()
{
	WulforSettingsManager *wsm = WulforSettingsManager::getInstance();
	string fore, back;
	bool bold = false, italic = false;

	for (int i = Tag::TAG_FIRST; i < Tag::TAG_LAST; i++)
	{
		if(i == Tag::TAG_PRIVATE)
				continue;

		getSettingTag_gui(wsm, (Tag::TypeTag)i, fore, back, bold, italic);

		g_object_set(TagsMap[i],
			"foreground", fore.c_str(),
			"background", back.c_str(),
			"weight", bold ? TEXT_WEIGHT_BOLD : TEXT_WEIGHT_NORMAL,
			"style", italic ? TEXT_STYLE_ITALIC : TEXT_STYLE_NORMAL,
			NULL);
	}

	gtk_widget_queue_draw(getWidget("nickView"));
	gtk_widget_queue_draw(getWidget("emotButton"));
	FavoriteHubEntry* p_faventry = getFavoriteHubEntry();

	if ( (p_faventry) && (!p_faventry->get(SettingsManager::USE_EMOTS,SETTING(USE_EMOTS))))
	{
		if (gtk_widget_is_sensitive(getWidget("emotButton")))
			gtk_widget_set_sensitive(getWidget("emotButton"), FALSE);
	}
	else if (!gtk_widget_is_sensitive(getWidget("emotButton")))
	{
		gtk_widget_set_sensitive(getWidget("emotButton"), TRUE);
	}

	// resort users
	sort = SETTING(SORT_FAVUSERS_FIRST) ? "Client Type" : "Nick Order";

	if(p_faventry)
		sort = p_faventry->get(SettingsManager::SORT_FAVUSERS_FIRST, SETTING(SORT_FAVUSERS_FIRST)) ? "Client Type" : "Nick Order";

	gtk_tree_sortable_set_sort_column_id(GTK_TREE_SORTABLE(nickStore), nickView.col(sort), GTK_SORT_ASCENDING);
	//Set Colors
// Re-Initialize the chat window
	string color = p_faventry ? p_faventry->get(SettingsManager::BACKGROUND_CHAT_COLOR, SETTING(BACKGROUND_CHAT_COLOR)) : SETTING(BACKGROUND_CHAT_COLOR);
	string image = p_faventry ? p_faventry->get(SettingsManager::BACKGROUND_CHAT_IMAGE, SETTING(BACKGROUND_CHAT_IMAGE)) : SETTING(BACKGROUND_CHAT_IMAGE);

	WulforUtil::setTextDeufaults(getWidget("chatText"),color,image,false,address);
	gtk_widget_queue_draw(getWidget("chatText"));
	setColorsRows();
}

void Hub::getSettingTag_gui(WulforSettingsManager *wsm, Tag::TypeTag type, string &fore, string &back, bool &bold, bool &italic)
{
	switch (type)
	{
		case Tag::TAG_MYOWN:

			fore = wsm->getString("text-myown-fore-color");
			back = wsm->getString("text-myown-back-color");
			bold = wsm->getInt("text-myown-bold");
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

		case Tag::TAG_CHEAT:

			fore = wsm->getString("text-cheat-fore-color");
			back = wsm->getString("text-cheat-back-color");
			bold = (bool)wsm->getInt("text-cheat-bold");
			italic = (bool)wsm->getInt("text-cheat-italic");
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

			fore = wsm->getString("text-general-fore-color");
			back = wsm->getString("text-general-back-color");
			italic = wsm->getInt("text-general-italic");
			bold = wsm->getBool("text-bold-autors");

		break;

		case Tag::TAG_GENERAL:

		default:
			fore = wsm->getString("text-general-fore-color");
			back = wsm->getString("text-general-back-color");
			bold = (bool)wsm->getInt("text-general-bold");
			italic = (bool)wsm->getInt("text-general-italic");
	}
}

GtkTextTag* Hub::createTag_gui(const string &tagname, Tag::TypeTag type)
{
	WulforSettingsManager *wsm = WulforSettingsManager::getInstance();
	GtkTextTag *tag = gtk_text_tag_table_lookup( gtk_text_buffer_get_tag_table(chatBuffer), tagname.c_str());

	if (!tag)
	{
		string fore, back;
		bool bold = false, italic = false;

		getSettingTag_gui(wsm, type, fore, back, bold, italic);

		tag = gtk_text_buffer_create_tag(chatBuffer, tagname.c_str(),
			"foreground", fore.c_str(),
			"background", back.c_str(),
			"weight", bold ? TEXT_WEIGHT_BOLD : TEXT_WEIGHT_NORMAL,
			"style", italic ? TEXT_STYLE_ITALIC : TEXT_STYLE_NORMAL,
			NULL);

	}

	return tag;
}

void Hub::addStatusMessage_gui(string message, Msg::TypeMsg typemsg, Sound::TypeSound sound, Notify::TypeNotify notify)
{
	if (notify == Notify::HUB_CONNECT)
		setIcon_gui(WGETS("icon-hub-online"));
	else if (notify == Notify::HUB_DISCONNECT)
		setIcon_gui(WGETS("icon-hub-offline"));

	addStatusMessage_gui(message, typemsg, sound);
	Notify::get()->showNotify("<b>" + client->getHubUrl() + ":</b> ", message, notify);
}

void Hub::clickAction(gpointer data)
{
/*	//TODO:maybe..some other & UI settings & fav?
	switch((CActions::User)WGETI("double-click-action"))
	{
		case CActions::BROWSE:
		{
			onBrowseItemClicked_gui(NULL, data);
			break;
		}
		case CActions::NICK_TO_CHAT:
		{
			onNickToChat_gui(NULL, data);
			break;
		}
	    case CActions::PM_TO_NICK:
	    {
			onMsgItemClicked_gui(NULL, data);
			break;
		}
		case CActions::MATCH_Q:
		{
			onMatchItemClicked_gui(NULL ,data);
			break;
		}
		case CActions::GRANT_SLOT:
		{
			onGrantItemClicked_gui(NULL, data);
			break;
		}
		case CActions::ADD_AS_FAV:
		{
			onAddFavoriteUserClicked_gui(NULL, data);
			break;
		}
		case CActions::GET_PARTIAL_FILELIST:
		{
			onPartialFileListOpen_gui(NULL, data);
			break;
		}
		default: break;
	}*/
}
/*
gboolean Hub::onEntryKeyPress_gui(GtkWidget *entry, GdkEventKey *event, gpointer data)
{
	Hub *hub = (Hub *)data;

	if ( (( WGETB("key-hub-with-ctrl") && ((event->keyval == GDK_KEY_Up) && (event->state & GDK_CONTROL_MASK ) )) )
			|| ( !WGETB("key-hub-with-ctrl") && (event->keyval == GDK_KEY_Up ) ))
	{
		size_t index = hub->historyIndex - 1;
		if (index < hub->history.size())
		{
			hub->historyIndex = index;
			gtk_entry_set_text(GTK_ENTRY(entry), hub->history[index].c_str());
		}
		return TRUE;
	}
	else if ( ( WGETB("key-hub-with-ctrl") && ((event->keyval == GDK_KEY_Down ) && (event->state & GDK_CONTROL_MASK) ))
			|| ( !WGETB("key-hub-with-ctrl") && (event->keyval == GDK_KEY_Down )) )
	{
		size_t index = hub->historyIndex + 1;
		if (index < hub->history.size())
		{
			hub->historyIndex = index;
			gtk_entry_set_text(GTK_ENTRY(entry), hub->history[index].c_str());
		}
		return TRUE;
	}
	else if (event->keyval == GDK_KEY_Tab)
	{
		string current;
		string::size_type start, end;
		string text(gtk_entry_get_text(GTK_ENTRY(entry)));
		int curpos = gtk_editable_get_position(GTK_EDITABLE(entry));

		// Allow tab to focus other widgets if entry is empty
		if (curpos <= 0 && text.empty())
			return FALSE;

		// Erase ": " at the end of the nick.
		if (curpos > 2 && text.substr(curpos - 2, 2) == ": ")
		{
			text.erase(curpos - 2, 2);
			curpos -= 2;
		}

		start = text.rfind(' ', curpos - 1);
		end = text.find(' ', curpos - 1);

		// Text to match starts at the beginning
		if (start == string::npos)
			start = 0;
		else
			++start;

		if (start < end)
		{
			current = text.substr(start, end - start);

			if (hub->completionKey.empty() || Text::toLower(current).find(Text::toLower(hub->completionKey)) == string::npos)
				hub->completionKey = current;

			GtkTreeIter iter;
			bool valid = gtk_tree_model_get_iter_first(GTK_TREE_MODEL(hub->nickStore), &iter);
			bool useNext = (current == hub->completionKey);
			string key = Text::toLower(hub->completionKey);
			string complete = hub->completionKey;

			while (valid)
			{
				string nick = hub->nickView.getString(&iter, _("Nick"));
				string::size_type tagEnd = 0;
				if (useNext && (tagEnd = Text::toLower(nick).find(key)) != string::npos)
				{
					if (tagEnd == 0 || nick.find_first_of("]})", tagEnd - 1) == tagEnd - 1)
					{
						complete = nick;
						if (start <= 0)
							complete.append(": ");
						break;
					}
				}

				if (nick == current)
					useNext = TRUE;

				valid = gtk_tree_model_iter_next(GTK_TREE_MODEL(hub->nickStore),&iter);
			}

			text.replace(start, end - start, complete);
			gtk_entry_set_text(GTK_ENTRY(entry), text.c_str());
			gtk_editable_set_position(GTK_EDITABLE(entry), start + complete.length());
		}
		else
			hub->completionKey.clear();

		return TRUE;
	}

	hub->completionKey.clear();
	return FALSE;
}

/*
gboolean Hub::onNickTagEvent_gui(GtkTextTag *tag, GObject*, GdkEvent *event, GtkTextIter*, gpointer data)
{
	if (event->type == GDK_2BUTTON_PRESS)
	{
		Hub *hub = (Hub *)data;
		string tagName = WulforUtil::getTagName(tag);

		hub->nickToChat_gui(tagName.substr(tagPrefix.size()));

		return TRUE;
	}
	else if (event->type == GDK_BUTTON_PRESS)
	{
		GtkTreeIter nickIter;
		string tagName = WulforUtil::getTagName(tag);
		Hub *hub = (Hub *)data;

		if (hub->findNick_gui(tagName.substr(tagPrefix.size()), &nickIter))
		{
			// Select the user in the nick list view
			GtkTreePath *path = gtk_tree_model_get_path(GTK_TREE_MODEL(hub->nickStore), &nickIter);
			gtk_tree_view_scroll_to_cell(hub->nickView.get(), path, gtk_tree_view_get_column(hub->nickView.get(), hub->nickView.col(_("Nick"))), FALSE, 0.0, 0.0);
			gtk_tree_view_set_cursor(hub->nickView.get(), path, NULL, FALSE);
			gtk_tree_path_free(path);

			if (event->button.button == 3)
				hub->popupNickMenu_gui();
		}

		return TRUE;
	}

	return FALSE;
}
*/
/*
gboolean Hub::onLinkTagEvent_gui(GtkTextTag*, GObject*, GdkEvent *event, GtkTextIter*, gpointer data)
{
	if (event->type == GDK_BUTTON_PRESS)
	{
		switch (event->button.button)
		{
			case 1:
            {
				onOpenLinkClicked_gui(NULL, data);
				break;
            }
			case 3:
            {
				Hub *hub = (Hub *)data;
				// Popup uri context menu
				gtk_widget_show_all(hub->getWidget("linkMenu"));
				gtk_menu_popup_at_pointer(GTK_MENU(hub->getWidget("linkMenu")),NULL);
				break;
            }
		}
		return TRUE;
	}
	return FALSE;
}*/
/*
gboolean Hub::onHubTagEvent_gui(GtkTextTag*, GObject*, GdkEvent *event, GtkTextIter*, gpointer data)
{
	if (event->type == GDK_BUTTON_PRESS)
	{
		switch (event->button.button)
		{
			case 1:
				onOpenHubClicked_gui(NULL, data);
				break;
			case 3:
				Hub *hub = (Hub *)data;
				// Popup uri context menu
				gtk_widget_show_all(hub->getWidget("hubMenu"));
				gtk_menu_popup_at_pointer(GTK_MENU(hub->getWidget("hubMenu")),NULL);
				break;
		}
		return TRUE;
	}
	return FALSE;
}*/
/*
gboolean Hub::onIpTagEvent_gui(GtkTextTag *tag, GObject*, GdkEvent *event , GtkTextIter*, gpointer data)
{
	Hub *hub = (Hub *)data;
	if(event->type == GDK_BUTTON_PRESS)
	{
		if(event->button.button == 3)
		{
            string tmp = WulforUtil::getTagName(tag);

			//g_signal_connect(hub->getWidget("ripeitem"), "activate", G_CALLBACK(onRipeDbItem_gui),(gpointer)hub);
			//g_signal_connect(hub->getWidget("copyipItem"), "activate", G_CALLBACK(onCopyIpItem_gui),(gpointer)hub);
			g_object_set_data_full(G_OBJECT(hub->getWidget("ripeitem")),"ip_addr",g_strdup(tmp.c_str()),g_free);
			g_object_set_data_full(G_OBJECT(hub->getWidget("copyipItem")),"ip_addr",g_strdup(tmp.c_str()),g_free);

			//gtk_menu_popup_at_pointer(GTK_MENU(hub->getWidget("ipMenu")),NULL);
			return TRUE;
		}
	}
	return FALSE;
}
*/
void Hub::onCopyIpItem_gui(GtkWidget* widget, gpointer)
{
	gchar* ip = (gchar *)g_object_get_data(G_OBJECT(widget),"ip_addr");

	GValue value = G_VALUE_INIT;
	g_value_init (&value, G_TYPE_STRING);
	g_value_set_string (&value, ip);
	// Store the value in the clipboard object
	GdkClipboard *clipboard = gtk_widget_get_clipboard (GTK_WIDGET(widget));
	gdk_clipboard_set_value (clipboard, &value);
	g_value_unset (&value);

}

void Hub::onRipeDbItem_gui(GtkWidget* widget, gpointer data)
{
	Hub* hub = (Hub*)data;
	string ip = (gchar*)g_object_get_data(G_OBJECT(widget),"ip_addr");
	dcpp::ParamMap params;
	params["IP"] = ip;
	string result = dcpp::Util::formatParams(SETTING(RIPE_DB),params);
	WulforUtil::openURI(result/*,error*/);
}
/*
gboolean Hub::onMagnetTagEvent_gui(GtkTextTag* tag, GObject*, GdkEvent *event, GtkTextIter*, gpointer data)
{
	if (event->type == GDK_BUTTON_PRESS)
	{
		Hub *hub = (Hub *)data;
        hub->selectedTagStr = WulforUtil::getTagName(tag);
		//switch (event->button.button)
		{
		//	case 1:
				// Search for magnet
				WulforManager::get()->getMainWindow()->actionMagnet_gui(hub->selectedTagStr);
		//		break;
		//	case 3:
				// Popup magnet context menu
//				gtk_menu_popup_at_pointer(GTK_MENU(hub->getWidget("magnetMenu")),NULL);
		//		break;
		}
		return TRUE;
	}
	return FALSE;
}*/
/*
gboolean Hub::onChatPointerMoved_gui(GtkWidget *widget, GdkEventMotion*, gpointer data)
{
	Hub *hub = (Hub *)data;

	hub->updateCursor_gui(widget);

	return FALSE;
}
/*
gboolean Hub::onChatVisibilityChanged_gui(GtkWidget *widget, GdkEventVisibility*, gpointer data)
{
	Hub *hub = (Hub *)data;

	hub->updateCursor_gui(widget);

	return FALSE;
}*/
/*
gboolean Hub::onEmotButtonRelease_gui(GtkWidget *wid, GdkEventButton *event, gpointer data)
{
	Hub *hub = (Hub *)data;

	switch (event->button)
	{
		case 1: //show emoticons dialog

			hub->emotdialog->showEmotDialog_gui();
		break;

		case 3: //show emoticons menu

			hub->emotdialog->buildEmotMenu_gui();

			GtkWidget *check_item = NULL;
			GtkWidget *emot_menu = hub->getWidget("emotPacksMenu");

			check_item = gtk_separator_menu_item_new();
			gtk_menu_shell_append(GTK_MENU_SHELL(emot_menu), check_item);
			gtk_widget_show(check_item);

			check_item = gtk_check_menu_item_new_with_label(_("Use Emoticons"));
			gtk_menu_shell_append(GTK_MENU_SHELL(emot_menu), check_item);

			if (hub->useEmoticons)
				gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(check_item), TRUE);

			g_signal_connect(check_item, "activate", G_CALLBACK(onUseEmoticons_gui), data);

			gtk_widget_show_all(emot_menu);
			gtk_menu_popup_at_widget(GTK_MENU(emot_menu),wid,GDK_GRAVITY_SOUTH_WEST,GDK_GRAVITY_NORTH_WEST,NULL);
		break;
	}

	return FALSE;
}
*/
void Hub::onChatScroll_gui(GtkAdjustment *adjustment, gpointer data)
{
	Hub *hub = (Hub *)data;
	gdouble value = gtk_adjustment_get_value(adjustment);
	hub->scrollToBottom = value >= ( gtk_adjustment_get_upper(adjustment) - gtk_adjustment_get_page_size (adjustment));
}

void Hub::onChatResize_gui(GtkAdjustment *adjustment, gpointer data)
{
	Hub *hub = (Hub *)data;
	gdouble value = gtk_adjustment_get_value(adjustment);

    if (hub->scrollToBottom && value < (gtk_adjustment_get_upper(adjustment) - gtk_adjustment_get_page_size (adjustment)))
	{
		GtkTextIter iter;

		gtk_text_buffer_get_end_iter(hub->chatBuffer, &iter);
		gtk_text_buffer_move_mark(hub->chatBuffer, hub->chatMark, &iter);
		gtk_text_view_scroll_mark_onscreen (GTK_TEXT_VIEW(hub->getWidget("chatText")), hub->chatMark);
	}
}

void Hub::onSendMessage_gui(GtkEntry *e, gpointer data)
{
	Hub *hub = (Hub *)data;

	string text = gtk_editable_get_text(GTK_EDITABLE(hub->getWidget("chatEntry")));
	if (text.empty())
		return;

	gtk_editable_set_text(GTK_EDITABLE(hub->getWidget("chatEntry")), "");

	typedef Func2<Hub, string, bool> F2;
	F2 *func2;

	// Store line in chat history
	hub->history.pop_back();
	hub->history.push_back(text);
	hub->history.push_back("");
	hub->historyIndex = hub->history.size() - 1;
	if (hub->history.size() > ((size_t)WGETI("hub-max-history") + 1))
		hub->history.erase(hub->history.begin());

	// Process special commands
	if (text[0] == '/')
	{
		typedef Func1<Hub, string> F1;
		F1 *func;
		string command = text, param ,mess = dcpp::Util::emptyString, status = dcpp::Util::emptyString, params;
		bool thirdPerson = false;
		string::size_type separator = text.find_first_of(' ');
		if (separator != string::npos && text.size() > separator + 1)
		{
			params = text.substr(separator + 1);
		}
	    if(WulforUtil::checkCommand(command, param, mess, status, thirdPerson))
        {
			if(!mess.empty())
				hub->sendMessage_client(mess, thirdPerson);

			if(!status.empty())
				hub->addStatusMessage_gui(status, Msg::SYSTEM, Sound::NONE);
			return;
        }
		if (command == "clear")
		{
			GtkTextIter startIter, endIter;
			gtk_text_buffer_get_start_iter(hub->chatBuffer, &startIter);
			gtk_text_buffer_get_end_iter(hub->chatBuffer, &endIter);
			gtk_text_buffer_delete(hub->chatBuffer, &startIter, &endIter);
		}
		else if (command == "close")
		{
			/// @todo: figure out why this sometimes closes and reopens the tab
			WulforManager::get()->getMainWindow()->removeBookEntry_gui(hub);
		}
		else if (command == "favorite" || command == "fav")
		{
			hub->addAsFavorite_client();
		}
		else if (command == "fuser" || command == "fu")
		{
			if (hub->client->getMyNick() == params) //dont add yoursefl
					return;

			if (hub->userMap.find(param) != hub->userMap.end())
			{
				string cid = hub->userMap[params];
				if (hub->userFavoriteMap.find(cid) == hub->userFavoriteMap.end())
				{
					Func1<Hub, const string&> *func = new Func1<Hub,const string&>(hub, &Hub::addFavoriteUser_client, cid);
					WulforManager::get()->dispatchClientFunc(func);
				} else {
					hub->addStatusMessage_gui(param + _(" is favorite user"), Msg::STATUS, Sound::NONE);
				}
			} else {
				hub->addStatusMessage_gui(_("User not found: ") + params, Msg::SYSTEM, Sound::NONE);
			}
		}
		else if (command == "removefu" || command == "rmfu")
		{
			if (hub->client->getMyNick() == params)
				return;

			UserMap::const_iterator it = find_if(hub->userFavoriteMap.begin(), hub->userFavoriteMap.end(),
				CompareSecond<string, string>(params));

			if (it != hub->userFavoriteMap.end())
			{
				Func1<Hub, const string&> *func = new Func1<Hub,const string&>(hub, &Hub::removeFavoriteUser_client, it->first);
				WulforManager::get()->dispatchClientFunc(func);
			}
			else
				hub->addStatusMessage_gui(params + _(" is not favorite user"), Msg::STATUS, Sound::NONE);
		}
		else if (command == "listfu" || command == "lsfu")
		{
			string list;
			for (UserMap::const_iterator it = hub->userFavoriteMap.begin(); it != hub->userFavoriteMap.end(); ++it)
			{
				list += " " + it->second;
			}
			hub->addMessage_gui("", _("User favorite list:") + (list.empty()? list = _(" empty...") : list), Msg::SYSTEM,"");
		}
		else if (command == "getlist")
		{
			if (hub->userMap.find(params) != hub->userMap.end())
			{
				typedef Func3<Hub, string, bool,bool> F3;
				F3 *func = new F3(hub, &Hub::getFileList_client, hub->userMap[params], FALSE,FALSE);
				WulforManager::get()->dispatchClientFunc(func);
			}
			else
				hub->addStatusMessage_gui(_("User not found: ") + param, Msg::SYSTEM, Sound::NONE);
		}
		else if (command == "grant")
		{
			if (hub->userMap.find(params) != hub->userMap.end())
			{
				func = new F1(hub, &Hub::grantSlot_client, hub->userMap[params]);
				WulforManager::get()->dispatchClientFunc(func);
			}
			else
				hub->addStatusMessage_gui(_("Not found user: ") + param, Msg::SYSTEM, Sound::NONE);
		}
		//use with caution is ip not properly checked
		/*else if(command == "addip")
		{
			if(!params.empty()) {

				auto i = params.find(' ');
				if(i == string::npos) {
					FavoriteManager::getInstance()->addFavoriteIp(params);
				} else {
					string r = params.substr(i+1);
					if(r == "range")
						FavoriteManager::getInstance()->addFavoriteIp(params.substr(0,i),time(NULL),FavoriteUser::FLAG_IP_RANGE);
				}
			}else {
				hub->addStatusMessage_gui(_("No IP address or range of IP given to the command"), Msg::SYSTEM , Sound::NONE );
			}
		}
		else if( command == "listip")
		{
			auto list = FavoriteManager::getInstance()->getListIp();
			string tmp = _("List WatchIPs\n");
			for(auto it:list)
					tmp += _("IP: ") + it.first + _(" Last Seen: ")+Util::formatTime("%Y-%m-%d %H:%M", it.second->getLastSeen())+"\n";

			hub->addMessage_gui("",tmp,Msg::SYSTEM,"");
		} else if ( command == "remip") {

			if(!params.empty()) {
				FavoriteManager::getInstance()->remFavoriteIp(params);
			}else
				hub->addStatusMessage_gui(_("No IP/range given to the command"), Msg::SYSTEM , Sound::NONE );

		}*/else if (command == "emoticons" || command == "emot")
		{
			if (hub->useEmoticons)
			{
				hub->useEmoticons = false;
				hub->addStatusMessage_gui(_("Emoticons mode off"), Msg::SYSTEM, Sound::NONE);
			}
			else
			{
				hub->useEmoticons = true;
				hub->addStatusMessage_gui(_("Emoticons mode on"), Msg::SYSTEM, Sound::NONE);
			}
		}
		else if (command == "help")
		{
			hub->addMessage_gui("", string(_("*** Available commands:")) + "\n" +
			"\r\n/clear\r\n\t" + _("Clear chat") +
			"\r\n/close\r\n\t" + _("Close chat") +
			"\r\n/favorite, /fav\r\n\t" + _("Add a hub to favorites") +
			"\r\n/fuser, /fu <nick>\r\n\t" + _("Add user to favorites list") +
			"\r\n/removefu, /rmfu <nick>\r\n\t" + _("Remove user favorite") +
			"\r\n/listfu, /lsfu\r\n\t" + _("Show favorites list") +
			"\r\n/getlist <nick>\r\n\t" + _("Get file list") +
			"\r\n/grant <nick>\r\n\t" + _("Grant extra slot") +
			"\r\n/help\r\n\t" + _("Show help") +
			"\r\n/join <address>\r\n\t" + _("Connect to the hub") +
			"\r\n/me <message>\r\n\t" + _("Say a third person") +
			"\r\n/pm <nick>\r\n\t" + _("Private message") +
			"\r\n/exec <cmd>\r\n\t" + _("Execute Bash chunk") +
			"\r\n/userlist\r\n\t" + _("User list show/hide") +
			"\r\n/emoticons, /emot\r\n\t" + _("Emoticons on/off") +
			"\r\n/sc <stop/start>\r\n\t" + _("Start/Stop checkers") +
			"\r\n/scmyinfo\r\n\t" + _("Start/Stop checkers of Myinfo") +
			"\r\n/showjoins\r\n\t" + _("Join/Parts: enable , other disable ") +
			"\r\n/showfavjoins\r\n\t" + _("Fav Joins/Parts: enable , other disable") +
			"\r\n/addfavorite\r\n\t" + _("Add Indepent Fav") +
			"\r\n/topic\r\n\t" + _("Show topic") +
			"\r\n/raw <rawtext>\r\n\t" + _("Send Raw data") +
			"\r\n/conn\r\n\t" + _("Show Conection Setup Info") +
			"\r\n/getuserinfo <nick>\r\n\t" +_("Show Get User Info") +
			//"\r\n/addip <ip/range>\r\n\t" + _("Get <ip> to watchlist") +
			//"\r\n/listip\r\n\t" + _("List watched ips") +
			"\r\n/info\r\n\t" + _("Get Info about hub connection & favorite hub info") +
			"\r\n/addfavorite\r\n\t" + _("Add Nick to Favorite users") +
			"\r\n/getloadhubinfo\r\n\t" + _("Get load hub info") +
			WulforUtil::commands
			,Msg::SYSTEM,"");
		}
		else if (command == "join" && !param.empty())
		{
			if (SETTING(JOIN_OPEN_NEW_WINDOW))
			{
				// Assumption: new hub is same encoding as current hub.
				WulforManager::get()->getMainWindow()->showHub_gui(param, hub->encoding);
			}
			else
			{
				F2 *func = new F2(hub, &Hub::redirect_client, param, TRUE);
				WulforManager::get()->dispatchClientFunc(func);
			}
		}
		else if (command == "me")
		{
			func2 = new F2(hub, &Hub::sendMessage_client, params, true);
			WulforManager::get()->dispatchClientFunc(func2);
		}
		else if (command == "topic" )
		{
			hub->addMessage_gui("", _("Topic: ") + hub->client->getHubDescription(), Msg::SYSTEM, "");
		}
		else if ( command == "info" )
		{

			auto isGlobalString = [](std::string i,std::string n) -> bool { return i == n;};
			auto isGlobalBool = [](bool i,bool n) -> bool { return i == n;};

			map<string, string> info;
            info[_("Hub address")] = hub->address;
			info[_("Hub IP & port")] = (dcpp::Util::isIp6(hub->client->getIp()) == true) ? ("["+hub->client->getIp()+"]:"+(std::to_string(hub->client->getPort())))  : hub->client->getIpPort();
			info[_("Online users")] = Util::toString(hub->client->getUserCount());
			info[_("Shared")] = Util::formatBytes(hub->client->getAvailable());

			info[_("Nick")] = hub->client->get(SettingsManager::NICK,SETTING(NICK)) +_(" - ") + (isGlobalString(hub->client->get(SettingsManager::NICK,SETTING(NICK)),SETTING(NICK)) ? _("Global") : _("User Per Favorite Hub Set"));
			info[_("Description")] = hub->client->get(SettingsManager::DESCRIPTION,SETTING(DESCRIPTION)) +_(" - ") + (isGlobalString(hub->client->get(SettingsManager::DESCRIPTION,SETTING(DESCRIPTION)),SETTING(DESCRIPTION)) ? _("Global") : _("User Per Favorite Hub Set"));
			info[_("Email")] = hub->client->get(SettingsManager::EMAIL,SETTING(EMAIL)) +_(" - ")+(isGlobalString(hub->client->get(SettingsManager::EMAIL,SETTING(EMAIL)),SETTING(EMAIL)) ? _("Global") : _("User Per Favorite Hub Set"));
			info[_("External / WAN IP")] = hub->client->get(SettingsManager::EXTERNAL_IP,SETTING(EXTERNAL_IP))+_(" - ")+(isGlobalString(hub->client->get(SettingsManager::EXTERNAL_IP,SETTING(EXTERNAL_IP)),SETTING(EXTERNAL_IP)) ? _("Global") : _("User Per Favorite Hub Set"));
			info[_("External / WAN IP6")] = hub->client->get(SettingsManager::EXTERNAL_IP6,SETTING(EXTERNAL_IP6))+_(" - ")+(isGlobalString(hub->client->get(SettingsManager::EXTERNAL_IP6,SETTING(EXTERNAL_IP6)),SETTING(EXTERNAL_IP6)) ? _("Global") : _("User Per Favorite Hub Set"));
			info[_("Encoding")] =  hub->client->getEncoding();
			info[_("Hide Share")] = hub->client->getHideShare() ? _("Yes") : _("No");
			FavoriteHubEntry* fav = hub->getFavoriteHubEntry();
			if(fav){
				info[_("Notification")] = fav->getNotify() ? _("Yes") : _("No");
				info[_("Mode")] = std::to_string(fav->getMode());
			}
			info[_("Log Chat")] = (hub->client->get(SettingsManager::LOG_CHAT_B,SETTING(LOG_CHAT_B)) ? string(_("Yes")) : _("No")) +_(" ") + (isGlobalBool(hub->client->get(SettingsManager::LOG_CHAT_B,SETTING(LOG_CHAT_B)),SETTING(LOG_CHAT_B)) ? _("Global") : _("User Per Favorite Hub Set"));
			info[_("Bold Tab")] = (hub->client->get(SettingsManager::BOLD_HUB,SETTING(BOLD_HUB)) ? string(_("Yes")) : _("No"))  +_(" ")+(isGlobalBool(hub->client->get(SettingsManager::BOLD_HUB,SETTING(BOLD_HUB)),SETTING(BOLD_HUB)) ? _("Global") : _("User Per Favorite Hub Set"));
			info[_("Show Country")] = (hub->client->get(SettingsManager::GET_USER_COUNTRY,SETTING(GET_USER_COUNTRY)) ? string(_("Yes")) : _("No"))+_(" ")+(isGlobalBool(hub->client->get(SettingsManager::GET_USER_COUNTRY,SETTING(GET_USER_COUNTRY)),SETTING(GET_USER_COUNTRY)) ? _(" Global ") : _(" User Per Favorite Hub Set "));
			info[_("Show IPs")] = (hub->client->get(SettingsManager::USE_IP,SETTING(USE_IP)) ? string(_("Yes")) : _("No"))+_(" ")+(isGlobalBool(hub->client->get(SettingsManager::USE_IP,SETTING(USE_IP)),SETTING(USE_IP)) ? _(" Global") : _(" User Per Favorite Hub Set "));

			string text;

			for(auto i = info.begin();i!=info.end();++i) {
				text += _("\n") + (*i).first + _(": ") + Text::toT((*i).second);
			}

			hub->addMessage_gui("",text,Msg::SYSTEM,"");

		} else if ( command == "conn") {

			string info = ConnectivityManager::getInstance()->getInformation();
			hub->addMessage_gui("", info, Msg::SYSTEM,"");

		}else if (command == "pm")
		{
			size_t j = params.find(" ");
			if(j != string::npos)
			{
			  string nick = params.substr(0, j);
			  string mess  = params.substr(j+1);
			  UserPtr ui = ClientManager::getInstance()->getUser(nick,hub->client->getHubUrl());
			  if(ui)
			  {
			  		WulforManager::get()->getMainWindow()->addPrivateMessage_gui(Msg::PRIVATE, ui->getCID().toBase32(), "",mess,true);
			  }
			}
			else if (hub->userMap.find(params) != hub->userMap.end())
				WulforManager::get()->getMainWindow()->addPrivateMessage_gui(Msg::UNKNOWN, hub->userMap[params], hub->client->getHubUrl());
			else
				hub->addStatusMessage_gui(_("User not found:: ") + param, Msg::SYSTEM, Sound::NONE);
		}
		else if (command == "userlist")
		{
			if (gtk_widget_get_visible(hub->getWidget("scrolledwindow2"))) {
				gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(hub->getWidget("userListCheckButton")), FALSE);
			} else {
				gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(hub->getWidget("userListCheckButton")), TRUE);
			}
		}
		else if (command == "exec")
		{
			FILE *pipe = popen( param.c_str(), "r" );
			gchar *command_res = NULL;
			gsize command_length = -1;
			GIOChannel* gio_chanel = g_io_channel_unix_new( fileno( pipe ) );
			GIOStatus gio_status = g_io_channel_read_to_end( gio_chanel, &command_res, &command_length, NULL );
			if( gio_status == G_IO_STATUS_NORMAL )
			{
				F2 *func = new F2( hub, &Hub::sendMessage_client, string(command_res,command_length-1), false );
				WulforManager::get()->dispatchClientFunc(func);
			}
			g_io_channel_shutdown( gio_chanel ,FALSE, NULL );
			g_free( command_res );
			pclose( pipe );
		}
		else if (command == "sc")
		{
			hub->addStatusMessage_gui(hub->client->startCheck(params), Msg::SYSTEM, Sound::NONE);
		}
		else if(command == "addfavorite")
		{
			FavoriteManager::getInstance()->addFavoriteIUser(params);
			hub->addStatusMessage_gui(_("Added Indepent Fav ") + params, Msg::SYSTEM, Sound::NONE);
		}
		else if (command == "scmyinfo")
		{
			if(param == "stop") {
				hub->client->stopMyInfoCheck();
				hub->addStatusMessage_gui(_("Stop MyInfo check"), Msg::SYSTEM, Sound::NONE);
			} else	{
				hub->client->startMyInfoCheck();
				hub->addStatusMessage_gui(_("Started MyInfo check"), Msg::SYSTEM, Sound::NONE);
			}
		} else if (command == "raw" )
		{
		   if(hub->client->isConnected()) {
				hub->client->send(Util::convertCEscapes(param));
			} else {
				hub->addStatusMessage_gui(_("Don't connected to hub"), Msg::SYSTEM, Sound::NONE);
			}
		}
		else if ( command == "showjoins")
		{
			if(params.empty()) return;

	        if(params == "enable") {
				hub->addStatusMessage_gui(_("Join/part showing on"), Msg::SYSTEM, Sound::NONE);
				hub->client->set(SettingsManager::SHOW_JOINS,true);
			} else {
				hub->addStatusMessage_gui(_("Join/part showing off"), Msg::SYSTEM, Sound::NONE);
				hub->client->set(SettingsManager::SHOW_JOINS,false);
       	}
		}
		else if ( command == "showfavjoins")
		{
			if(params.empty()) return;

			if(params == "enable") {
				hub->addStatusMessage_gui(_("Join/part for Fav showing on"), Msg::SYSTEM, Sound::NONE);
				hub->client->set(SettingsManager::FAV_SHOW_JOINS,true);
			} else {
				hub->addStatusMessage_gui(_("Join/part for fav showing off"), Msg::SYSTEM, Sound::NONE);
				hub->client->set(SettingsManager::FAV_SHOW_JOINS,false);
			}
		}
		else if ( command == "getuserinfo")
		{
			if(params.empty()) {
				hub->addStatusMessage_gui(_("No user given to command"),Msg::SYSTEM,Sound::NONE);
			} else {
				ClientManager* cm = ClientManager::getInstance();
				cm->lock();
				UserPtr ui = cm->findUser(params,hub->client->getHubUrl());
				OnlineUser* ou = cm->findOnlineUser(ui->getCID(),hub->client->getHubUrl());
				Identity& id = ou->getIdentity();
				hub->addMessage_gui("",WulforUtil::formatReport(id)+"\n",Msg::SYSTEM,"");

			}
		}
		else if ( command == "getloadhubinfo")
		{
			ClientManager* cm = ClientManager::getInstance();
			hub->addMessage_gui("",cm->getHubsLoadInfo(), Msg::SYSTEM,"");

		}
		// protect command
		else if (command == "password")
		{
			if (!hub->WaitingPassword)
				return;

			hub->setPassword_client(params);
			hub->WaitingPassword = FALSE;
		}
		else
		{
			if (SETTING(SEND_UNKNOWN_COMMANDS))
			{
					if(SETTING(SERVER_COMMANDS)) {
						if(text[0] == '!' || text[0] == '+' || text[0] == '-')
							hub->addMessage_gui("",_("Server command: ") + text, Msg::SYSTEM,"");
					}
				hub->sendMessage_client(text, false);
			}
			else {
				hub->addStatusMessage_gui(_("Unknown command '") + text + _("': type /help for a list of available commands"), Msg::SYSTEM, Sound::NONE);
			}
		}
	}
	else
	{
		if(SETTING(SERVER_COMMANDS)) {
			if(text[0] == '!' || text[0] == '+' || text[0] == '-')
				hub->addMessage_gui("",_("Server command: ") + text, Msg::SYSTEM,"");
		}
		hub->sendMessage_client(text, false);
	}
}

void Hub::onNickToChat_gui(GtkWidget*,GVariant*, gpointer data)
{
	Hub *hub = (Hub *)data;

	if (gtk_tree_selection_count_selected_rows(hub->nickSelection) > 0)
	{
		string nicks;
		GtkTreeIter iter;
		GtkTreePath *path;
		GList *list = gtk_tree_selection_get_selected_rows(hub->nickSelection, NULL);

		for (GList *i = list; i; i = i->next)
		{
			path = (GtkTreePath *)i->data;

			if (gtk_tree_model_get_iter(GTK_TREE_MODEL(hub->nickStore), &iter, path))
				nicks += hub->nickView.getString(&iter, _("Nick")) + ", ";

			gtk_tree_path_free(path);
		}

		g_list_free(list);

		if (!nicks.empty())
		{
			nicks.erase(nicks.size() - 2);
			hub->nickToChat_gui(nicks);
		}
	}
}

void Hub::onBrowseItemClicked_gui(GtkWidget*,GVariant*, gpointer data)
{
	Hub *hub = (Hub *)data;

	if (gtk_tree_selection_count_selected_rows(hub->nickSelection) > 0)
	{
		string cid;
		GtkTreeIter iter;
		GtkTreePath *path;
		typedef Func3<Hub, string, bool,bool> F3;
		F3 *func;
		GList *list = gtk_tree_selection_get_selected_rows(hub->nickSelection, NULL);

		for (GList *i = list; i; i = i->next)
		{
			path = (GtkTreePath *)i->data;
			if (gtk_tree_model_get_iter(GTK_TREE_MODEL(hub->nickStore), &iter, path))
			{
				cid = hub->nickView.getString(&iter, "CID");
				func = new F3(hub, &Hub::getFileList_client, cid, false,false);
				WulforManager::get()->dispatchClientFunc(func);
			}
			gtk_tree_path_free(path);
		}
		g_list_free(list);
	}
}

void Hub::onMatchItemClicked_gui(GtkWidget*,GVariant*, gpointer data)
{
	Hub *hub = (Hub *)data;

	if (gtk_tree_selection_count_selected_rows(hub->nickSelection) > 0)
	{
		string cid;
		GtkTreeIter iter;
		GtkTreePath *path;
		typedef Func3<Hub, string, bool,bool> F3;
		F3 *func;
		GList *list = gtk_tree_selection_get_selected_rows(hub->nickSelection, NULL);

		for (GList *i = list; i; i = i->next)
		{
			path = (GtkTreePath *)i->data;
			if (gtk_tree_model_get_iter(GTK_TREE_MODEL(hub->nickStore), &iter, path))
			{
				cid = hub->nickView.getString(&iter, "CID");
				func = new F3(hub, &Hub::getFileList_client, cid, TRUE,FALSE);
				WulforManager::get()->dispatchClientFunc(func);
			}
			gtk_tree_path_free(path);
		}
		g_list_free(list);
	}
}

void Hub::onMsgItemClicked_gui(GtkWidget*,GVariant*, gpointer data)
{
	Hub *hub = (Hub *)data;

	if (gtk_tree_selection_count_selected_rows(hub->nickSelection) > 0)
	{
		string cid;
		GtkTreeIter iter;
		GtkTreePath *path;
		GList *list = gtk_tree_selection_get_selected_rows(hub->nickSelection, NULL);
		const string &hubUrl = hub->client->getHubUrl();

		for (GList *i = list; i; i = i->next)
		{
			path = (GtkTreePath *)i->data;
			if (gtk_tree_model_get_iter(GTK_TREE_MODEL(hub->nickStore), &iter, path))
			{
				cid = hub->nickView.getString(&iter, "CID");
				WulforManager::get()->getMainWindow()->addPrivateMessage_gui(Msg::UNKNOWN, cid, hubUrl);
			}
			gtk_tree_path_free(path);
		}
		g_list_free(list);
	}
}

void Hub::onGrantItemClicked_gui(GtkWidget*,GVariant*, gpointer data)
{
	Hub *hub = (Hub *)data;

	if (gtk_tree_selection_count_selected_rows(hub->nickSelection) > 0)
	{
		string cid;
		GtkTreeIter iter;
		GtkTreePath *path;
		typedef Func1<Hub, string> F1;
		F1 *func;
		GList *list = gtk_tree_selection_get_selected_rows(hub->nickSelection, NULL);

		for (GList *i = list; i; i = i->next)
		{
			path = (GtkTreePath *)i->data;
			if (gtk_tree_model_get_iter(GTK_TREE_MODEL(hub->nickStore), &iter, path))
			{
				cid = hub->nickView.getString(&iter, "CID");
				func = new F1(hub, &Hub::grantSlot_client, cid);
				WulforManager::get()->dispatchClientFunc(func);
			}
			gtk_tree_path_free(path);
		}
		g_list_free(list);
	}
}

void Hub::onRemoveUserItemClicked_gui(GtkWidget*,GVariant* v, gpointer data)
{
	Hub *hub = (Hub *)data;

	if (gtk_tree_selection_count_selected_rows(hub->nickSelection) > 0)
	{
		string cid;
		GtkTreeIter iter;
		GtkTreePath *path;
		typedef Func1<Hub, string> F1;
		F1 *func;
		GList *list = gtk_tree_selection_get_selected_rows(hub->nickSelection, NULL);

		for (GList *i = list; i; i = i->next)
		{
			path = (GtkTreePath *)i->data;
			if (gtk_tree_model_get_iter(GTK_TREE_MODEL(hub->nickStore), &iter, path))
			{
				cid = hub->nickView.getString(&iter, "CID");
				func = new F1(hub, &Hub::removeUserFromQueue_client, cid);
				WulforManager::get()->dispatchClientFunc(func);
			}
			gtk_tree_path_free(path);
		}
		g_list_free(list);
	}
}
/*
void Hub::onCopyURIClicked_gui(GtkWidget*,GVariant* v, gpointer data)
{
	Hub *hub = (Hub *)data;

	GValue value = G_VALUE_INIT;
		g_value_init (&value, G_TYPE_STRING);
		g_value_set_string (&value, address.c_str());
		// Store the value in the clipboard object
		GdkClipboard *clipboard = gtk_widget_get_clipboard (GTK_WIDGET(fh->hub->selectedTagStr.c_str()));
		gdk_clipboard_set_value (clipboard, &value);

		g_value_unset (&value);

}
*/
/*
void Hub::onOpenLinkClicked_gui(GtkWidget*,GVariant* v, gpointer data)
{
	Hub *hub = (Hub *)data;
	string error = dcpp::Util::emptyString;
	WulforUtil::openURI(hub->selectedTagStr,error);

	if(!error.empty())
		hub->setStatus_gui("statusMain", error);
}

void Hub::onOpenHubClicked_gui(GtkWidget*,GVariant* v, gpointer data)
{
	Hub *hub = (Hub *)data;

	WulforManager::get()->getMainWindow()->showHub_gui(hub->selectedTagStr);
}

void Hub::onSearchMagnetClicked_gui(GtkWidget*,GVariant* v, gpointer data)
{
	Hub *hub = (Hub *)data;

	WulforManager::get()->getMainWindow()->addSearch_gui(hub->selectedTagStr);
}

void Hub::onDownloadClicked_gui(GtkWidget*,GVariant* v, gpointer data)
{
	Hub *hub = (Hub *)data;
	WulforManager::get()->getMainWindow()->fileToDownload_gui(hub->selectedTagStr, SETTING(DOWNLOAD_DIRECTORY));
}
*/
void Hub::onUseEmoticons_gui(GtkWidget*, gpointer data)
{
	Hub *hub = (Hub *)data;

	hub->useEmoticons = !hub->useEmoticons;
}
/*
void Hub::onDownloadToClicked_gui(GtkMenuItem*, gpointer data)
{
	GtkWidget *dialog = WulforManager::get()->getMainWindow()->getChooserDialog_gui();
	gtk_window_set_title(GTK_WINDOW(dialog), _("Choose a directory"));
	gtk_file_chooser_set_action(GTK_FILE_CHOOSER(dialog), GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER);
	gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(dialog), Text::fromUtf8(WGETS("magnet-choose-dir")).c_str());
	gint response = gtk_dialog_run(GTK_DIALOG(dialog));

	if (response == GTK_RESPONSE_OK)
	{
		g_autofree gchar *temp = gtk_file_chooser_get_current_folder(GTK_FILE_CHOOSER(dialog));

		if (temp)
		{
			Hub *hub = (Hub *)data;
			string path = Text::toUtf8(temp) + G_DIR_SEPARATOR_S;

			WulforManager::get()->getMainWindow()->fileToDownload_gui(hub->selectedTagStr, path);
		}
	}

}

void Hub::onMagnetPropertiesClicked_gui(GtkWidget*,GVariant* v, gpointer data)
{
	Hub *hub = (Hub *)data;

	WulforManager::get()->getMainWindow()->propertiesMagnetDialog_gui(hub->selectedTagStr);
}
*/
void Hub::onUserListToggled_gui(GtkWidget*, gpointer data)
{
	Hub *hub = (Hub *)data;

	if (gtk_widget_get_visible(hub->getWidget("scrolledwindow2"))) {
		gtk_widget_set_visible (hub->getWidget("scrolledwindow2") , FALSE);
	} else {
		gtk_widget_set_visible (hub->getWidget("scrolledwindow2") , TRUE);
	}
}

void Hub::onAddFavoriteUserClicked_gui(GtkWidget* wid,GVariant* v, gpointer data)
{
	Hub *hub = (Hub *)data;

	if (gtk_tree_selection_count_selected_rows(hub->nickSelection) > 0)
	{
		string cid, nick;
		gint order;
		GtkTreeIter iter;
		GtkTreePath *path;
		typedef Func1<Hub, const string&> F1;
		GList *list = gtk_tree_selection_get_selected_rows(hub->nickSelection, NULL);

		for (GList *i = list; i; i = i->next)
		{
			path = (GtkTreePath *)i->data;

			if (gtk_tree_model_get_iter(GTK_TREE_MODEL(hub->nickStore), &iter, path))
			{
				cid = hub->nickView.getString(&iter, "CID");
				nick = hub->nickView.getString(&iter, _("Nick"));
				order = hub->nickView.getValue<gint>(&iter, "Client Type");

				if (!cid.empty() && nick != hub->client->getMyNick())
				{
					if (order != FAVORITE)
					{
						F1 *func = new F1(hub, &Hub::addFavoriteUser_client, cid);
						WulforManager::get()->dispatchClientFunc(func);
					}
					else
						hub->addStatusMessage_gui(nick + _(" is favorite user"), Msg::STATUS, Sound::NONE);
				}
			}
			gtk_tree_path_free(path);
		}
		g_list_free(list);
	}
}

void Hub::onRemoveFavoriteUserClicked_gui(GtkWidget*,GVariant* v, gpointer data)
{
	Hub *hub = (Hub *)data;

	if (gtk_tree_selection_count_selected_rows(hub->nickSelection) > 0)
	{
		string cid, nick; gint order;
		GtkTreeIter iter;
		GtkTreePath *path;
		typedef Func1<Hub, const string&> F1;
		GList *list = gtk_tree_selection_get_selected_rows(hub->nickSelection, NULL);

		for (GList *i = list; i; i = i->next)
		{
			path = (GtkTreePath *)i->data;

			if (gtk_tree_model_get_iter(GTK_TREE_MODEL(hub->nickStore), &iter, path))
			{
				cid = hub->nickView.getString(&iter, "CID");
				nick = hub->nickView.getString(&iter, _("Nick"));
				order = hub->nickView.getValue<gint>(&iter, "Client Type");

				if (!cid.empty() && nick != hub->client->getMyNick())
				{
					if (order == FAVORITE)
					{
						F1 *func = new F1(hub, &Hub::removeFavoriteUser_client, cid);
						WulforManager::get()->dispatchClientFunc(func);
					}
					else
						hub->addStatusMessage_gui(nick + _(" is not favorite user"), Msg::STATUS, Sound::NONE);
				}
			}
			gtk_tree_path_free(path);
		}
		g_list_free(list);
	}
}

void Hub::onAddIgnoreUserItemClicked_gui(GtkWidget*,GVariant* v, gpointer data)
{
	Hub *hub = (Hub *)data;

	if (gtk_tree_selection_count_selected_rows(hub->nickSelection) > 0)
	{
		string cid;
		GtkTreeIter iter;
		GtkTreePath *path;
		GList *list = gtk_tree_selection_get_selected_rows(hub->nickSelection, NULL);
		for (GList *i = list; i; i = i->next)
		{
			path = (GtkTreePath *)i->data;
			if (gtk_tree_model_get_iter(GTK_TREE_MODEL(hub->nickStore), &iter, path))
			{
				cid = hub->nickView.getString(&iter, "CID");
				UserPtr user = ClientManager::getInstance()->findUser(CID(cid),hub->client->getHubUrl());

				if (user)
				{
					FavoriteManager::getInstance()->addFavoriteUser(user);
					FavoriteManager::getInstance()->setIgnore(user,true);
				}
				else
				{
					string message = _("User Ignored ");
					message += WulforUtil::getNicks(user, hub->client->getHubUrl());
					hub->addStatusMessage_gui(message, Msg::SYSTEM, Sound::NONE);
				}
			}
		  gtk_tree_path_free(path);
		  }
		g_list_free(list);
    }
}

void Hub::onRemoveIgnoreUserItemClicked_gui(GtkWidget*,GVariant* v, gpointer data)
{
	Hub *hub = (Hub *)data;

	if (gtk_tree_selection_count_selected_rows(hub->nickSelection) > 0)
	{
	string cid;
	GtkTreeIter iter;
	GtkTreePath *path;

	GList *list = gtk_tree_selection_get_selected_rows(hub->nickSelection, NULL);
	for (GList *i = list; i; i = i->next)
		{
			path = (GtkTreePath *)i->data;
			if (gtk_tree_model_get_iter(GTK_TREE_MODEL(hub->nickStore), &iter, path))
			{
				cid = hub->nickView.getString(&iter, "CID");
				UserPtr user = ClientManager::getInstance()->findUser(CID(cid),hub->client->getHubUrl());

				if (user)
				{
					if( FavoriteManager::getInstance()->isFavoriteUser(user) && FavoriteManager::getInstance()->getFavoriteUser(user)->isSet(FavoriteUser::FLAG_IGNORE) )
					{
						FavoriteManager::getInstance()->setIgnore(user,false);

					}
				}
				else
				{
					string message = _("User removed from ignored ");
					message += WulforUtil::getNicks(user, hub->client->getHubUrl());
					hub->addStatusMessage_gui(message, Msg::SYSTEM, Sound::NONE);

				}
			}
		  gtk_tree_path_free(path);
		  }
		g_list_free(list);
    }
}
//Protect Users
void Hub::onProtectUserClicked_gui(GtkWidget*,GVariant* v, gpointer data)
{
	Hub *hub =(Hub *)data;
	if (gtk_tree_selection_count_selected_rows(hub->nickSelection) >0)
	{
		string cid;
		GtkTreeIter iter;
		GtkTreePath *path;
		GList *list = gtk_tree_selection_get_selected_rows(hub->nickSelection, NULL);

		for (GList *i = list; i; i = i->next)
		{
			path = (GtkTreePath *)i->data;
			if (gtk_tree_model_get_iter(GTK_TREE_MODEL(hub->nickStore), &iter, path))
			{
				cid = hub->nickView.getString(&iter, _("CID"));
			}
			gtk_tree_path_free(path);
		}
		g_list_free(list);

		if (!cid.empty())
		{
			OnlineUser *ou = ClientManager::getInstance()->findOnlineUser(CID(cid), hub->client->getHubUrl());
			if(ou->getUser() && !ou->getUser()->isSet(User::PROTECT))
				ou->getUser()->setFlag(User::PROTECT);
			dcpp::StringMap params;
			hub->getParams_client(params, ou->getIdentity());
			hub->addProtected_gui(params);
		}
	}
}

void Hub::onUnProtectUserClicked_gui(GtkWidget*,GVariant* v, gpointer data)
{
	Hub *hub =(Hub *)data;
	if (gtk_tree_selection_count_selected_rows(hub->nickSelection) >0 )
	{
		string cid;
		GtkTreeIter iter;
		GtkTreePath *path;
		GList *list = gtk_tree_selection_get_selected_rows(hub->nickSelection, NULL);

		for (GList *i = list; i; i = i->next)
		{
			path = (GtkTreePath *)i->data;
			if (gtk_tree_model_get_iter(GTK_TREE_MODEL(hub->nickStore), &iter, path))
			{
				cid = hub->nickView.getString(&iter, _("CID"));
			}
			gtk_tree_path_free(path);
		}
		g_list_free(list);

		if (!cid.empty())
		{
			OnlineUser *ou = ClientManager::getInstance()->findOnlineUser(CID(cid), hub->client->getHubUrl());
			if(ou->getUser() && !ou->getUser()->isSet(User::PROTECT))
				ou->getUser()->unsetFlag(User::PROTECT);
			dcpp::StringMap params;
			hub->getParams_client(params, ou->getIdentity());

		}
	}
}

void Hub::onShowReportClicked_gui(GtkWidget*,GVariant* v, gpointer data)
{
    Hub *hub = (Hub *)data;
	string cid,icon;

	if (gtk_tree_selection_count_selected_rows(hub->nickSelection) > 0)
	{
		GtkTreeIter iter;
		GtkTreePath *path;
		GList *list = gtk_tree_selection_get_selected_rows(hub->nickSelection, NULL);

		for (GList *i = list ; i ; i=i->next)
		{
			path = (GtkTreePath*)i->data;
			if (gtk_tree_model_get_iter(GTK_TREE_MODEL(hub->nickStore), &iter,path))
			{
				cid = hub->nickView.getString(&iter, "CID") ;
				icon = hub->nickView.getString(&iter, "Icon");
			}
			gtk_tree_path_free(path);
		}
		g_list_free(list);

		OnlineUser *ou = ClientManager::getInstance()->findOnlineUser(CID(cid), hub->client->getHubUrl());
		Identity id = ou->getIdentity();

		hub->addMessage_gui("", WulforUtil::formatReport(id)+"\nIcon\t"+icon+"\n", Msg::CHEAT,"");

	}
}
//Test SUR
void Hub::onTestSURItemClicked_gui(GtkWidget*,GVariant* v, gpointer data)
{
	Hub *hub = (Hub *)data;

	if (gtk_tree_selection_count_selected_rows(hub->nickSelection) > 0)
	{
		std::queue<std::string> nicks;
		GtkTreeIter iter;
		GtkTreePath *path;
		GList *list = gtk_tree_selection_get_selected_rows(hub->nickSelection, NULL);

		for (GList *i = list; i; i = i->next)
		{
			path = (GtkTreePath *)i->data;
			if (gtk_tree_model_get_iter(GTK_TREE_MODEL(hub->nickStore), &iter, path))
			{
				nicks.push(hub->nickView.getString(&iter, "CID"));
			}
			gtk_tree_path_free(path);
		}
		g_list_free(list);

		while (!nicks.empty())
		{
		   	string 	nick = nicks.front();
			OnlineUser *ou = ClientManager::getInstance()->findOnlineUser( CID(nick),hub->client->getHubUrl());

			if(ou)
			{

				if(ou->getIdentity().isHub()) return;

				try {
					HintedUser hintedUser(ou->getUser(), hub->client->getHubUrl());
					ClientManager::getInstance()->addCheckToQueue(hintedUser, false);
				}catch(...)
				{ }
			}
			nicks.pop();
		}
	}
}
//check FL
void Hub::onCheckFLItemClicked_gui(GtkWidget*,GVariant* v, gpointer data)
{
	Hub *hub = (Hub *)data;

	if (gtk_tree_selection_count_selected_rows(hub->nickSelection) > 0)
	{
		string nick;
		GtkTreeIter iter;
		GtkTreePath *path = NULL;
		GList *plist = gtk_tree_selection_get_selected_rows(hub->nickSelection, NULL);

		for (GList *i = plist; i; i = i->next)
		{
			path = (GtkTreePath *)i->data;
			if (gtk_tree_model_get_iter(GTK_TREE_MODEL(hub->nickStore), &iter, path))
			{
				nick = hub->nickView.getString(&iter, _("CID"));
			}
			gtk_tree_path_free(path);
		}
		g_list_free(plist);

		if (!nick.empty())
		{
			OnlineUser *ou = ClientManager::getInstance()->findOnlineUser(CID(nick),hub->client->getHubUrl());
			if(ou)
			{
				if(ou->getIdentity().isHub()) return;

				try {
					 HintedUser hintedUser(ou->getUser(), hub->client->getHubUrl());
					 ClientManager::getInstance()->addCheckToQueue(hintedUser, true);
				}
				 catch(...)
				{ }
			}
		}
	}
}

void Hub::onRefreshUserListClicked_gui(GtkWidget*, gpointer data)
{
	Hub *hub = (Hub *)data;
	hub->clearNickList_gui();
	hub->client->refreshuserlist();
}

void Hub::addFavoriteUser_gui(dcpp::StringMap params)
{
	const string &cid = params["CID"];

	if (userFavoriteMap.find(cid) == userFavoriteMap.end())
	{
		GtkTreeIter iter;
		const string &nick = params["Nick"];
		userFavoriteMap.insert(UserMap::value_type(cid, nick));

		// resort users
		if (findUser_gui(cid, &iter))
		{
			gtk_list_store_set(nickStore, &iter,
				nickView.col("Client Type"), Util::toInt(params["Type"]),
				nickView.col("NickColor"), WGETS("userlist-text-favorite").c_str(),
				-1);
			removeTag_gui(nick);
		}

		string message = nick + _(" added to favorites list");
		addStatusMessage_gui(message, Msg::STATUS, Sound::NONE);
		WulforManager::get()->getMainWindow()->addPrivateStatusMessage_gui(Msg::STATUS, cid, message);

	}
}

void Hub::removeFavoriteUser_gui(dcpp::StringMap params)
{
	const string &cid = params["CID"];

	if (userFavoriteMap.find(cid) != userFavoriteMap.end())
	{
		GtkTreeIter iter;
		const string &nick = params["Nick"];
		userFavoriteMap.erase(cid);

		// resort users
		if (findUser_gui(cid, &iter))
		{
			gtk_list_store_set(nickStore, &iter,
				nickView.col("NickColor"), WGETS("userlist-text-normal").c_str(),
				-1);
			removeTag_gui(nick);
		}

		string message = nick + _(" removed from favorites list");
		addStatusMessage_gui(message, Msg::STATUS, Sound::NONE);
		WulforManager::get()->getMainWindow()->addPrivateStatusMessage_gui(Msg::STATUS, cid, message);
	}
}

void Hub::addProtected_gui(ParamMap params)
{
	const string &cid = params["CID"];
	GtkTreeIter iter;
	const string &nick = params["Nick"];
	// resort users
	if (findUser_gui(cid, &iter))
	{
		gtk_list_store_set(nickStore, &iter,
                   nickView.col("NickColor"), WGETS("userlist-text-protected").c_str(),
                   nickView.col("Client Type"), Util::toInt(params["Type"]),
			-1);
		removeTag_gui(nick);
	}
}

void Hub::addPrivateMessage_gui(Msg::TypeMsg typemsg, string CID, string cid, string url, string message, bool useSetting)
{
	if (userFavoriteMap.find(CID) != userFavoriteMap.end())
		typemsg = Msg::FAVORITE;

	WulforManager::get()->getMainWindow()->addPrivateMessage_gui(typemsg, cid, url, message, useSetting);
}

void Hub::addFavoriteUser_client(const string& cid)
{
	UserPtr user = ClientManager::getInstance()->findUser(CID(cid),client->getHubUrl());

	if (user)
	{
		FavoriteManager::getInstance()->addFavoriteUser(user);
	}
}

void Hub::removeFavoriteUser_client(const string &cid)
{
	UserPtr user = ClientManager::getInstance()->findUser(CID(cid),client->getHubUrl());

	if (user)
	{
		FavoriteManager::getInstance()->removeFavoriteUser(user);
	}
}

void Hub::connectClient_client(string address, string encoding)
{
	dcassert(client == NULL);

	if (address.substr(0, 6) == "adc://" || address.substr(0, 7) == "adcs://")
		encoding = "UTF-8";
	else if (encoding.empty() || encoding == "Global hub default") // latter for 1.0.3 backwards compatability
		encoding = WGETS("default-charset");

	if (encoding == WulforUtil::ENCODING_LOCALE)
		encoding = Text::systemCharset;

	// Only pick "UTF-8" part of "UTF-8 (Unicode)".
	string::size_type i = encoding.find(' ', 0);
	if (i != string::npos)
		encoding = encoding.substr(0, i);

	client = ClientManager::getInstance()->getClient(address);
	client->setEncoding(encoding);
	client->addListener(this);
	client->connect();
	FavoriteManager::getInstance()->addListener(this);
	QueueManager::getInstance()->addListener(this);
}

void Hub::disconnect_client(bool shutdownHub /* = FALSE */)
{
	if (client)
	{
		FavoriteManager::getInstance()->removeListener(this);
		QueueManager::getInstance()->removeListener(this);
		client->removeListener(this);
		if(shutdownHub)
			client->disconnect(true);

		ClientManager::getInstance()->putClient(client);
		client = nullptr;
	}
}

void Hub::setPassword_client(string password)
{
	if (client && !password.empty())
	{
		client->setPassword(password);
		client->password(password);
	}
}

void Hub::sendMessage_client(string message, bool thirdPerson)
{
	if (client && !message.empty())
		client->hubMessage(message, thirdPerson);
}

void Hub::getFileList_client(string cid, bool bMatch,bool bPartial)
{
	string message;

	if (!cid.empty())
	{
		try
		{
			OnlineUser* ou = ClientManager::getInstance()->findOnlineUser(CID(cid), client->getHubUrl());
			if (ou)
			{
				const HintedUser hintedUser(ou->getUser(), client->getHubUrl());

				if (ou->getUser() == ClientManager::getInstance()->getMe())
				{
					// Don't download file list, open locally instead
					WulforManager::get()->getMainWindow()->openOwnList_client(true);
				}
				else if (bMatch)
				{
					QueueManager::getInstance()->addList(hintedUser, QueueItem::FLAG_MATCH_QUEUE);
				}
				else
				{
					QueueManager::getInstance()->addList(hintedUser, bPartial ? (QueueItem::FLAG_CLIENT_VIEW | QueueItem::FLAG_PARTIAL_LIST) : (QueueItem::FLAG_CLIENT_VIEW));
				}
			}
			else
			{
				message = _("User not found");
			}
		}
		catch (const Exception &e)
		{
			message = e.getError();
			LogManager::getInstance()->message(message);
		}
	}

	if (!message.empty())
	{
		typedef Func3<Hub, string, Msg::TypeMsg, Sound::TypeSound> F3;
		F3 *func = new F3(this, &Hub::addStatusMessage_gui, message, Msg::SYSTEM, Sound::NONE);
		WulforManager::get()->dispatchGuiFunc(func);
	}
}

void Hub::grantSlot_client(string cid)
{
	string message = _("User not found");

	if (!cid.empty())
	{
		UserPtr user = ClientManager::getInstance()->findUser(CID(cid),client->getHubUrl());
		if (user)
		{
			const string hubUrl = client->getHubUrl();
			UploadManager::getInstance()->reserveSlot(HintedUser(user, hubUrl));
			message = _("Slot granted to ") + WulforUtil::getNicks(user, hubUrl);
		}
	}

	typedef Func3<Hub, string, Msg::TypeMsg, Sound::TypeSound> F3;
	F3 *func = new F3(this, &Hub::addStatusMessage_gui, message, Msg::STATUS, Sound::NONE);
	WulforManager::get()->dispatchGuiFunc(func);
}

void Hub::removeUserFromQueue_client(string cid)
{
	if (!cid.empty())
	{
		UserPtr user = ClientManager::getInstance()->findUser(CID(cid),client->getHubUrl());
		if (user)
			QueueManager::getInstance()->removeSource(user, QueueItem::Source::FLAG_REMOVED);
	}
}

void Hub::redirect_client(string address, bool bFollow)
{
	if (!address.empty())
	{
		if (ClientManager::getInstance()->isConnected(address))
		{
			string error = _("Unable to connect: already connected to the requested hub");
			typedef Func3<Hub, string, Msg::TypeMsg, Sound::TypeSound> F3;
			F3 *f3 = new F3(this, &Hub::addStatusMessage_gui, error, Msg::STATUS, Sound::NONE);
			WulforManager::get()->dispatchGuiFunc(f3);
			return;
		}

		if (bFollow)
		{
			// the client is dead, long live the client!
			disconnect_client();

			Func0<Hub> *func = new Func0<Hub>(this, &Hub::clearNickList_gui);
			WulforManager::get()->dispatchGuiFunc(func);

			connectClient_client(address, encoding); //this assume a redirect hub 'can' use same encoding
		}
	}
}

void Hub::addAsFavorite_client()
{
	typedef Func3<Hub, string, Msg::TypeMsg, Sound::TypeSound> F3;
	F3 *func;

	FavoriteHubEntry *existingHub = FavoriteManager::getInstance()->getFavoriteHubEntry(client->getHubUrl());
	FavoriteHubEntry *exHub = NULL;
	string sHubUrl = client->getHubUrl();
	size_t i = sHubUrl.find("dchub://");
	if(i != string::npos) exHub = FavoriteManager::getInstance()->getFavoriteHubEntry(sHubUrl.substr(i));

	if (!exHub && !existingHub)
	{
		FavoriteHubEntry entry;
		entry.setServer(client->getHubUrl());
		entry.setName(client->getHubName());
		entry.set(SettingsManager::DESCRIPTION, client->get(SettingsManager::DESCRIPTION,SETTING(DESCRIPTION)));
		entry.get(SettingsManager::NICK, client->getMyNick());
		entry.setEncoding(encoding);
		if(!client->getPassword().empty())
			entry.setPassword(client->getPassword());
		else
			entry.setPassword(dcpp::Util::emptyString);

		entry.setGroup(dcpp::Util::emptyString);

		FavoriteManager::getInstance()->addFavorite(entry);

		func = new F3(this, &Hub::addStatusMessage_gui, _("Favorite hub added"), Msg::STATUS, Sound::NONE);
		WulforManager::get()->dispatchGuiFunc(func);
	}
	else
	{
		func = new F3(this, &Hub::addStatusMessage_gui, _("Favorite hub already exists"), Msg::STATUS, Sound::NONE);
		WulforManager::get()->dispatchGuiFunc(func);
	}
}

void Hub::removeAsFavorite_client()
{
    typedef Func3<Hub, string, Msg::TypeMsg, Sound::TypeSound> F3;
	F3 *func;
    FavoriteHubEntry *existingHub = FavoriteManager::getInstance()->getFavoriteHubEntry(client->getHubUrl());

	if (existingHub)
	{
        FavoriteManager::getInstance()->removeFavorite(existingHub);

        func = new F3(this, &Hub::addStatusMessage_gui, _("Favorite hub removed"), Msg::STATUS, Sound::NONE);
        WulforManager::get()->dispatchGuiFunc(func);
	}
	else
	{
        func = new F3(this, &Hub::addStatusMessage_gui, _("This hub is not a favorite hub"), Msg::STATUS, Sound::NONE);
        WulforManager::get()->dispatchGuiFunc(func);
	}
}

void Hub::reconnect_client()
{
	Func0<Hub> *func = new Func0<Hub>(this, &Hub::clearNickList_gui);
	WulforManager::get()->dispatchGuiFunc(func);

	if (client)
		client->reconnect();
}

string Hub::getIcons(const Identity& id)
{
	string tmp = "other";
	if(id.isOp()) {
		 tmp = "op";
	} else /*status*/
	{
		string conn = id.getConnection();
		if(	(conn == "28.8Kbps") || (conn == "33.6Kbps") ||	(conn == "56Kbps") || (conn == "Modem") || (conn == "ISDN")) {
				tmp =  "modem";
			} else if(	(conn == "Satellite") || (conn == "Microwave") || (conn == "Wireless")) {
				tmp = "wireless";
			} else if(	(conn == "DSL") ||	(conn == "Cable")) {
				tmp = "dsl";
			} else if(	(strncmp(conn.c_str(), "LAN", 3) == 0)) {
				tmp = "lan";
			} else if( (strncmp(conn.c_str(), "NetLimiter", 10) == 0)) {
				tmp = "netlimiter";
			} else {
				size_t n = 0;
				if( (n = conn.find("/s")) != string::npos)
					conn.erase(n);

				double us = conn.empty() ? (8 * Util::toInt64(id.get("US")) / 1024 / 1024): Util::toDouble(conn);

				if(us >= 10) {
					tmp = "ten";
				} else if(us >= 0.1) {
					tmp = "zeroone";
				} else if(us >= 0.01) {
					tmp = "zerozeroone";
				} else if(us >= 0) {
					tmp = "other";
				}
			}
	}

	if(id.isAway()) {
			tmp += "-away";
	}

	if(!id.isTcpActive(client)) {
			tmp += "-pasive";
	}
	return tmp;
}

void Hub::getParams_client(ParamMap &params, Identity &id)
{
	params.insert(ParamMap::value_type("Icon", getIcons(id)));

	if (id.isOp())
	{
		params.insert(ParamMap::value_type("Nick Order", "O" + id.getNick()));
	}
	else
	{
		params.insert(ParamMap::value_type("Nick Order", "U" + id.getNick()));
	}

	params.insert(ParamMap::value_type("Nick", id.getNick()));
	params.insert(ParamMap::value_type("Shared", Util::toString(id.getBytesShared())));
	params.insert(ParamMap::value_type("Description", id.getDescription()));
	params.insert(ParamMap::value_type("Tag", id.getTag()));
	params.insert(ParamMap::value_type("Connection", id.getConnection()));
	params.insert(ParamMap::value_type("IP", id.getIp()));
	params.insert(ParamMap::value_type("eMail", id.getEmail()));
	params.insert(ParamMap::value_type("CID", id.getUser()->getCID().toBase32()));
	//BMDC++
	if( !id.isHub() || !id.isBot() || !id.getIp().empty() ) { //should *not* getting CC from Bot/Hub User
		params.insert(ParamMap::value_type("Country", (SETTING(GET_USER_COUNTRY)) ? /*GeoManager::getInstance()->getCountry(id.getIp())*/id.getCountry() : dcpp::Util::emptyString ));
		params.insert(ParamMap::value_type("Abbrevation", (SETTING(GET_USER_COUNTRY)) ? GeoManager::getInstance()->getCountryAbbrevation(id.getIp()): dcpp::Util::emptyString ));
	}
	params.insert(ParamMap::value_type("Slots", id.get("SL")));
	const string hubs = Util::toString(Util::toInt(id.get("HN")) + Util::toInt(id.get("HR")) + Util::toInt(id.get("HO")));//@Hubs
	params.insert(ParamMap::value_type("Hubs", hubs));
	params.insert(ParamMap::value_type("PK", id.get("PK")));
	params.insert(ParamMap::value_type("Cheat", id.get("CS")));
	params.insert(ParamMap::value_type("Generator", id.get("GE")));
	params.insert(ParamMap::value_type("Support", id.get("SU")));

	if(id.isBot() || id.isHub()) {
        params.insert(ParamMap::value_type("Type", "0"));
        params.insert(ParamMap::value_type("NickColor",WGETS("userlist-text-bot-hub")));
	} else if (id.isOp()) {
        params.insert(ParamMap::value_type("Type", "1"));
        params.insert(ParamMap::value_type("NickColor",WGETS("userlist-text-operator")));
    }
      else if(id.getUser()->isSet(User::PROTECT))
	{
		params.insert(ParamMap::value_type("NickColor", WGETS("userlist-text-protected")));
		params.insert(ParamMap::value_type("Type", "5"));
	}
	 else if(FavoriteManager::getInstance()->isFavoriteUser(id.getUser())
	&& !FavoriteManager::getInstance()->getFavoriteUser(id.getUser())->isSet(FavoriteUser::FLAG_IGNORE))
	{
		params.insert(ParamMap::value_type("Type", "3"));
		params.insert(ParamMap::value_type("NickColor", WGETS("userlist-text-favorite")));
	}
	else if(FavoriteManager::getInstance()->isFavoriteIUser(id.getNick()))
	{
		params.insert(ParamMap::value_type("Type", "3"));
		params.insert(ParamMap::value_type("NickColor", WGETS("userlist-text-favorite")));
	}
	else if(FavoriteManager::getInstance()->isFavoriteUser(id.getUser())
	&& FavoriteManager::getInstance()->getFavoriteUser(id.getUser())->isSet(FavoriteUser::FLAG_IGNORE))
     	{
		params.insert(ParamMap::value_type("Type", "4"));
		params.insert(ParamMap::value_type("NickColor", WGETS("userlist-text-ignored")));
	}else if(id.getUser()->isSet(User::PASSIVE))
	{
		params.insert(ParamMap::value_type("Type", "6"));
		params.insert(ParamMap::value_type("NickColor", WGETS("userlist-text-pasive")));
	}
	else {
        params.insert(ParamMap::value_type("Type", "7"));
		params.insert(ParamMap::value_type("NickColor",WGETS("userlist-text-normal")));
	}

	if ( AVManager::getInstance()->isNickVirused(id.getNick()) )
	{
		params.insert(ParamMap::value_type("NickColor",WGETS("userlist-text-virus")));
	}

}

void Hub::on(FavoriteManagerListener::UserAdded, const FavoriteUser &user) noexcept
{
	if (user.getUrl() != client->getHubUrl())
		return;

	ParamMap params;
	params.insert(ParamMap::value_type("Nick", user.getNick()));
	params.insert(ParamMap::value_type("CID", user.getUser()->getCID().toBase32()));
	params.insert(ParamMap::value_type("Order", ClientManager::getInstance()->isOp(user.getUser(), user.getUrl()) ? "O" : "U"));
	params.insert(ParamMap::value_type("Type", string(user.isSet(FavoriteUser::FLAG_IGNORE) ? "I" : "C") + user.getNick()));

	Func1<Hub, ParamMap> *func = new Func1<Hub, ParamMap>(this, &Hub::addFavoriteUser_gui, params);
	WulforManager::get()->dispatchGuiFunc(func);
}

void Hub::on(dcpp::FavoriteManagerListener::StatusChanged, const FavoriteUser& fu) noexcept
{
	if (fu.getUrl() != client->getHubUrl())
		return;

	ParamMap params;
	params.insert(ParamMap::value_type("Nick", fu.getNick()));
	params.insert(ParamMap::value_type("CID", fu.getUser()->getCID().toBase32()));
	params.insert(ParamMap::value_type("Order", ClientManager::getInstance()->isOp(fu.getUser(), fu.getUrl()) ? "O" : "U"));
	params.insert(ParamMap::value_type("Type", string(fu.isSet(FavoriteUser::FLAG_IGNORE) ? "I" : "C") + fu.getNick()));

	Func1<Hub, ParamMap> *func = new Func1<Hub, ParamMap>(this, &Hub::addFavoriteUser_gui, params);
	WulforManager::get()->dispatchGuiFunc(func);
}

void Hub::on(FavoriteManagerListener::UserRemoved, const FavoriteUser &user) noexcept
{
	if (user.getUrl() != client->getHubUrl())
		return;

	ParamMap params;
	params.insert(ParamMap::value_type("Nick", user.getNick()));
	params.insert(ParamMap::value_type("CID", user.getUser()->getCID().toBase32()));

	Func1<Hub, ParamMap> *func = new Func1<Hub, ParamMap>(this, &Hub::removeFavoriteUser_gui, params);
	WulforManager::get()->dispatchGuiFunc(func);
}
//Indepent Fav
void Hub::on(FavoriteManagerListener::FavoriteIAdded, const string &nick, FavoriteUser* &user) noexcept
{
	ParamMap params;
	params.insert(ParamMap::value_type("CID", user->getCid()));
	params.insert(ParamMap::value_type("Nick", nick));
	params.insert(ParamMap::value_type("Type", "C" + nick));
	Func1<Hub, ParamMap> *func = new Func1<Hub, ParamMap>(this,&Hub::addFavoriteUser_gui,params);
	WulforManager::get()->dispatchGuiFunc(func);
}

void Hub::on(FavoriteManagerListener::FavoriteIRemoved, const string &nick, FavoriteUser* &user) noexcept
{
	ParamMap params;
	params.insert(ParamMap::value_type("Nick", nick));
	params.insert(ParamMap::value_type("CID", user->getCid()));
	Func1<Hub, ParamMap> *func = new Func1<Hub, ParamMap>(this,&Hub::removeFavoriteUser_gui, params);
	WulforManager::get()->dispatchGuiFunc(func);
}
//end
void Hub::on(ClientListener::Connecting, Client *) noexcept
{
	typedef Func3<Hub, string, Msg::TypeMsg, Sound::TypeSound> F3;
	F3 *f3 = new F3(this, &Hub::addStatusMessage_gui, _("Connecting to ") + client->getHubUrl() + "...", Msg::STATUS, Sound::NONE);
	WulforManager::get()->dispatchGuiFunc(f3);
}

void Hub::on(ClientListener::Connected, Client *) noexcept
{
	typedef Func4<Hub, string, Msg::TypeMsg, Sound::TypeSound, Notify::TypeNotify> F4;
	F4 *func = new F4(this, &Hub::addStatusMessage_gui, _("Connected"), Msg::STATUS, Sound::HUB_CONNECT, Notify::HUB_CONNECT);
	WulforManager::get()->dispatchGuiFunc(func);
}

void Hub::on(ClientListener::UserUpdated, Client *, const OnlineUser &user) noexcept
{
	Identity id = user.getIdentity();

	if (!id.isHidden())
	{
		ParamMap params;
		getParams_client(params, id);
		Func1<Hub, ParamMap> *func = new Func1<Hub, ParamMap>(this, &Hub::updateUser_gui, params);
		WulforManager::get()->dispatchGuiFunc(func);
	}
}

void Hub::on(ClientListener::UsersUpdated, Client *, const OnlineUserList &list) noexcept
{
	Identity id;
	typedef Func1<Hub, ParamMap> F1;
	F1 *func;

	for (OnlineUserList::const_iterator it = list.begin(); it != list.end(); ++it)
	{
		id = (*it)->getIdentity();
		if (!id.isHidden())
		{
			ParamMap params;
			getParams_client(params, id);
			func = new F1(this, &Hub::updateUser_gui, params);
			WulforManager::get()->dispatchGuiFunc(func);
		}
	}
}

void Hub::on(ClientListener::UserRemoved, Client *, const OnlineUser &user) noexcept
{
	Func1<Hub, string> *func = new Func1<Hub, string>(this, &Hub::removeUser_gui, user.getUser()->getCID().toBase32());
	WulforManager::get()->dispatchGuiFunc(func);
}

void Hub::on(ClientListener::Redirect, Client *, const string &address) noexcept
{
	// redirect_client() crashes unless I put it into the dispatcher (why?)
	typedef Func2<Hub, string, bool> F2;
	F2 *func = new F2(this, &Hub::redirect_client, address, SETTING(AUTO_FOLLOW));
	WulforManager::get()->dispatchClientFunc(func);
}

void Hub::on(ClientListener::Failed, Client *, const string &reason) noexcept
{
	Func0<Hub> *f0 = new Func0<Hub>(this, &Hub::clearNickList_gui);
	WulforManager::get()->dispatchGuiFunc(f0);

	typedef Func4<Hub, string, Msg::TypeMsg, Sound::TypeSound, Notify::TypeNotify> F4;
	F4 *f4 = new F4(this, &Hub::addStatusMessage_gui, _("Connect failed: ") + reason, Msg::SYSTEM, Sound::HUB_DISCONNECT, Notify::HUB_DISCONNECT);
	WulforManager::get()->dispatchGuiFunc(f4);
}

void Hub::on(ClientListener::GetPassword, Client *) noexcept
{
	if (!client->getPassword().empty()) {
		client->password(client->getPassword());
        //Show if stored pass send in status
		typedef Func4<Hub, string,Msg::TypeMsg, Sound::TypeSound, Notify::TypeNotify> F4;
		F4 *func4 = new F4(this, &Hub::addStatusMessage_gui, _("Send Stored password... "), Msg::SYSTEM, Sound::NONE, Notify::NONE);
		WulforManager::get()->dispatchGuiFunc(func4);
	}
	else
	{
		Func0<Hub> *func = new Func0<Hub>(this, &Hub::getPassword_gui);
		WulforManager::get()->dispatchGuiFunc(func);
	}
}

void Hub::on(ClientListener::HubUpdated, Client *) noexcept
{
	typedef Func1<Hub, const string> F1;
	typedef Func1<Hub, string> FX;
	string hubName = dcpp::Util::emptyString;
	string hubText = client->get(SettingsManager::HUB_TEXT_STR, SETTING(HUB_TEXT_STR));
	string iconPath = client->get(SettingsManager::HUB_ICON_STR, SETTING(HUB_ICON_STR));

	if(!iconPath.empty())
	{
		F1 *f = new F1(this, &BookEntry::setIconPixbufs_gui, iconPath);
		WulforManager::get()->dispatchGuiFunc(f);
	} else
	{
		F1 *f = new F1(this, &BookEntry::setIcon_gui, "bmdc-hub-online");
		WulforManager::get()->dispatchGuiFunc(f);
	}

	if(!hubText.empty())
	{
		FX *f = new FX(this, &BookEntry::setLabel_gui, hubText);
		WulforManager::get()->dispatchGuiFunc(f);
		return;
	}

	if (client->getHubName().empty())
		hubName += client->getAddress() + ":" + std::to_string(client->getPort());
	else
		hubName += client->getHubName();

	if (!client->getHubDescription().empty())
		hubName += " - " + client->getHubDescription();

	FX *func1 = new FX(this, &BookEntry::setLabel_gui, hubName);
	WulforManager::get()->dispatchGuiFunc(func1);
}

string Hub::formatAdditionalInfo(const string& sIp, bool bIp, bool bCC) {
	string sRet = dcpp::Util::emptyString;

	if(!sIp.empty()) {

		string country_name = bCC ? GeoManager::getInstance()->getCountry(sIp) : Util::emptyString;
		bool showCc = bCC && !country_name.empty();

		if(bIp) {
			sRet = "[ " + sIp + " ] ";
		}

		if(showCc) {
			sRet += "[" + country_name + "] ";
		}

	}
	return sRet;
}

void Hub::on(ClientListener::Message, Client*, const ChatMessage& message) noexcept
{
	if (message.text.empty())
		return;

	Msg::TypeMsg typemsg;
	string line;
	string tmp_text = message.text;
	OnlineUser* ou = nullptr;
	Identity fid;
	ClientManager * pClientMan = ClientManager::getInstance();
	{
		pClientMan->lock();
		ou = pClientMan->findOnlineUser(message.from->getCID(),client->getHubUrl());
		fid = ou->getIdentity();
	}

	IgnoreTempManager* pIgnMan = IgnoreTempManager::getInstance();
	bool bIsNickIgnored = pIgnMan->isNickIgnored(fid.getNick());
	bool bIsIpIgnored = pIgnMan->isIpIgnored(fid.getIp());
	bool bIsCidIgnored = pIgnMan->isCidIgnored(message.from->getCID().toBase32());

	if(bIsNickIgnored || bIsIpIgnored || bIsCidIgnored)
	{
		string sError = _("Temp Ignored User: ") + fid.getNick() + " " + fid.getIp() + " " + ou->getUser()->getCID().toBase32();
		typedef Func3<Hub, string, Msg::TypeMsg, Sound::TypeSound> F3;
		F3 *func = new F3(this, &Hub::addStatusMessage_gui, sError, Msg::STATUS, Sound::NONE);
		WulforManager::get()->dispatchGuiFunc(func);
		return;
	}

	string sCc = dcpp::Util::emptyString;
	if( (!fid.isHub()) && (!fid.isBot()) )
	{
		sCc = GeoManager::getInstance()->getCountryAbbrevation(fid.getIp());
		string info = formatAdditionalInfo(fid.getIp(), client->get(SettingsManager::USE_IP,SETTING(USE_IP)), client->get(SettingsManager::GET_USER_COUNTRY,SETTING(GET_USER_COUNTRY)));
		//Extra Info
		dcpp::ParamMap params;
		params["hubURL"] = client->getHubUrl();
		client->getHubIdentity().getParams(params, "hub", false);
		client->getMyIdentity().getParams(params, "my", true);
		fid.getParams(params, "user", true);
		string extraInfo = Util::formatParams(client->get(SettingsManager::CHAT_EXTRA_INFO,SETTING(CHAT_EXTRA_INFO)), params);
		if(!extraInfo.empty())
			info += " " + extraInfo + " ";
		line += info;
	}

	bool bThird = false;

	string mess = message.text;
	{
		size_t nestle = message.text.find("/me");
		if(nestle != string::npos)
		 {

			size_t nt = message.text.find_first_of(" ",nestle);
			if( message.text.compare(0,nt,"/me") == 0) {
				 	bThird = true;
					mess.replace(0,nt+1,"");
			}
		}
	}

	if (bThird || message.thirdPerson)
		line += "* " + fid.getNick() + " " +  (mess.empty() ? message.text : mess);
	else
		line += "<" + fid.getNick() + "> " + message.text;

	    if(FavoriteManager::getInstance()->getFavoriteUser(fid.getUser()) && FavoriteManager::getInstance()->getFavoriteUser(fid.getUser())->isSet(FavoriteUser::FLAG_IGNORE))
	{
		string error = _("Ignored message from User ") + fid.getNick() + " from " + ((message.to && message.replyTo) ? "PM" : "Mainchat");
		error += _("\nMessage: ") + message.text + "\n";

		dcpp::ParamMap params;
		params["message"] = error;

		if (client->get(SettingsManager::LOG_CHAT_B,SETTING(LOG_CHAT_B)))
			LOG(LogManager::SYSTEM, params);

		typedef Func3<Hub, string, Msg::TypeMsg, Sound::TypeSound> F3;
		F3 *func = new F3(this, &Hub::addStatusMessage_gui, error, Msg::STATUS, Sound::NONE);
		WulforManager::get()->dispatchGuiFunc(func);
		return;
	}

	if(message.to && message.replyTo)
	{
		//private message

		string error;
		UserPtr user = (message.replyTo == ClientManager::getInstance()->getMe()) ?
			message.to : message.replyTo;
		Identity uid;
		{
			pClientMan->lock();
			OnlineUser* ou2 = pClientMan->findOnlineUser(user->getCID(), client->getHubUrl());
			uid = ou2->getIdentity();
		}
		if (fid.isOp()) typemsg = Msg::OPERATOR;
		else if (message.from == client->getMyIdentity().getUser()) typemsg = Msg::MYOWN;
		else typemsg = Msg::PRIVATE;

		if (uid.isHub() && SETTING(IGNORE_HUB_PMS))
		{
			error = _("Ignored private message from hub");
			typedef Func3<Hub, string, Msg::TypeMsg, Sound::TypeSound> F3;
			F3 *func = new F3(this, &Hub::addStatusMessage_gui, error, Msg::STATUS, Sound::NONE);
			WulforManager::get()->dispatchGuiFunc(func);
		}
		else if (uid.isBot() && SETTING(IGNORE_BOT_PMS))
		{
			error = _("Ignored private message from bot ") + uid.getNick();
			typedef Func3<Hub, string, Msg::TypeMsg, Sound::TypeSound> F3;
			F3 *func = new F3(this, &Hub::addStatusMessage_gui, error, Msg::STATUS, Sound::NONE);
			WulforManager::get()->dispatchGuiFunc(func);
		}
		else
		{
			typedef Func6<Hub, Msg::TypeMsg, string, string, string, string, bool> F6;
			F6 *func = new F6(this, &Hub::addPrivateMessage_gui, typemsg, message.from->getCID().toBase32(),
			uid.getUser()->getCID().toBase32(), client->getHubUrl(), line, TRUE);
			WulforManager::get()->dispatchGuiFunc(func);
		}
	}
	else
	{
		 // chat message
		string cid = fid.getUser()->getCID().toBase32();

		if (fid.isHub()) typemsg = Msg::STATUS;
		else if (message.from == client->getMyIdentity().getUser()) typemsg = Msg::MYOWN;
		else typemsg = Msg::GENERAL;

		if (SETTING(FILTER_MESSAGES))
		{
			if ((message.text.find("Hub-Security") != string::npos &&
				message.text.find("was kicked by") != string::npos) ||
				(message.text.find("is kicking") != string::npos && message.text.find("because:") != string::npos))
			{
				typedef Func3<Hub, string, Msg::TypeMsg, Sound::TypeSound> F3;
				F3 *func = new F3(this, &Hub::addStatusMessage_gui, line, Msg::STATUS, Sound::NONE);
				WulforManager::get()->dispatchGuiFunc(func);

				return;
			}
		}

		if (client->get(SettingsManager::LOG_MAIN_CHAT,SETTING(LOG_MAIN_CHAT)))
		{
			dcpp::ParamMap params;
			params["message"] = tmp_text;
			client->getHubIdentity().getParams(params, "hub", false);
			params["hubURL"] = client->getHubUrl();
			client->getMyIdentity().getParams(params, "my", true);
			LOG(LogManager::CHAT, params);
		}

		typedef Func4<Hub, string, string, Msg::TypeMsg,string> F4;
		F4 *func = new F4(this, &Hub::addMessage_gui, cid, line, typemsg,sCc);
		WulforManager::get()->dispatchGuiFunc(func);

		// Set urgency hint if message contains user's nick
		if (  (client->get(SettingsManager::BOLD_HUB,SETTING(BOLD_HUB))) && fid.getUser() != client->getMyIdentity().getUser())
		{
			typedef Func0<Hub> F0;
			//@ only if not active tab and not setted to notify always
			if( !isActive_gui() && WGETB("bold-all-tab"))
			{
				F0 *func = new F0(this, &Hub::setUrgent_gui);
				WulforManager::get()->dispatchGuiFunc(func);
				return;
			}

			if (message.text.find(client->getMyIdentity().getNick()) != string::npos)
			{
				F0 *func = new F0(this, &Hub::setUrgent_gui);
				WulforManager::get()->dispatchGuiFunc(func);
			}
		}
	}
}

void Hub::on(ClientListener::StatusMessage, Client *, const string &message, int flag) noexcept
{
	if (!message.empty())
	{
		if (SETTING(FILTER_MESSAGES))
		{
			if ((message.find("Hub-Security") != string::npos && message.find("was kicked by") != string::npos) ||
				(message.find("is kicking") != string::npos && message.find("because:") != string::npos))
			{
				typedef Func3<Hub, string, Msg::TypeMsg, Sound::TypeSound> F3;
				F3 *func = new F3(this, &Hub::addStatusMessage_gui, message, Msg::STATUS, Sound::NONE);
				WulforManager::get()->dispatchGuiFunc(func);
				return;
			}
		}

		if (SETTING(LOG_STATUS_MESSAGES))
		{
			dcpp::ParamMap params;
			client->getHubIdentity().getParams(params, "hub", FALSE);
			params["hubURL"] = client->getHubUrl();
			client->getMyIdentity().getParams(params, "my", TRUE);
			params["message"] = message;
			LOG(LogManager::STATUS, params);
		}

		if(flag == FLAG_VIRUS) {
			typedef Func4<Hub, string, string, Msg::TypeMsg,string> F4;
			F4 *func = new F4(this, &Hub::addMessage_gui, "", message, Msg::CHEAT,"");
			WulforManager::get()->dispatchGuiFunc(func);
			return;
		}

		typedef Func4<Hub, string, string, Msg::TypeMsg,string> F4;
		F4 *func = new F4(this, &Hub::addMessage_gui, "", message, Msg::STATUS,"");
		WulforManager::get()->dispatchGuiFunc(func);
	}
}

void Hub::on(ClientListener::NickTaken, Client *) noexcept
{
	typedef Func3<Hub, string, Msg::TypeMsg, Sound::TypeSound> F3;
	F3 *func = new F3(this, &Hub::addStatusMessage_gui, _("Nick already taken"), Msg::STATUS, Sound::NONE);
	WulforManager::get()->dispatchGuiFunc(func);
}

void Hub::on(ClientListener::SearchFlood, Client *, const string &msg) noexcept
{
	typedef Func3<Hub, string, Msg::TypeMsg, Sound::TypeSound> F3;
	F3 *func = new F3(this, &Hub::addStatusMessage_gui, _("Search spam detected from ") + msg, Msg::STATUS, Sound::NONE);
	WulforManager::get()->dispatchGuiFunc(func);
}

void Hub::on(ClientListener::CheatMessage, Client *, const string &msg) noexcept
{
	typedef Func3<Hub, string, Msg::TypeMsg, Sound::TypeSound> F3;
	F3 *func = new F3(this, &Hub::addStatusMessage_gui, msg, Msg::CHEAT, Sound::NONE);
	WulforManager::get()->dispatchGuiFunc(func);
}

void Hub::on(ClientListener::HubTopic, Client *, const string &top) noexcept
{
    typedef Func3<Hub, string, Msg::TypeMsg, Sound::TypeSound> F3;
    F3 *func = new F3(this, &Hub::addStatusMessage_gui, _("Topic: ") + top, Msg::STATUS, Sound::NONE);
    WulforManager::get()->dispatchGuiFunc(func);
}
//Custom popup
GMenu* Hub::createmenu()
{
	GMenu *menu = BookEntry::createmenu();
	GMenuItem* label = g_menu_item_new(address.c_str(), NULL);
	g_menu_prepend_item(menu ,label);

	GMenuItem* copy = g_menu_item_new(_("Copy URL") , "hub.copy-url");
	g_menu_append_item(menu , copy);

	GMenuItem * fav = g_menu_item_new(_("Add to Favorite hubs"), "hub.fav-hubs");
	g_menu_append_item(menu , fav);

	GMenuItem *rem = g_menu_item_new(_("Remove from Favorite Hubs") , "hub.fav-rem");
	g_menu_append_item(menu , rem);

	GMenuItem *reconnectItem  = g_menu_item_new("Reconnect" , "hub.reconnect");
	g_menu_append_item(menu , reconnectItem);

	GMenuItem *setTabName  = g_menu_item_new("Set Tab Name" , "hub.set-tab-name");
	g_menu_append_item(menu , setTabName);


	GMenuItem * userCommands = g_menu_item_new("User Commands", NULL);
	g_menu_item_set_submenu(userCommands , G_MENU_MODEL(userCommandMenu1->getContainer()));
	g_menu_append_item(menu , userCommands);
	userCommandMenu1->addUser(client->getMyIdentity().getUser()->getCID().toBase32());
/*
		//custom share things...
        GtkWidget *shareView = NULL,*shareRefresh = NULL;
        ShareManager *sm = client->getShareManager();
        if(!sm->getName().empty())
        {
            shareView = gtk_menu_item_new_with_label (_("Show Own Filelist Browser"));
            shareRefresh = gtk_menu_item_new_with_label(_("Refresh Share for this hub"));
        }
	*/
	return menu;
}

void Hub::onShareView(GtkWidget* ,GVariant*, gpointer data)
{
	Hub* hub = (Hub*)data;
	ShareManager *sm = hub->client->getShareManager();
	sm->generateXmlList();
	WulforManager::get()->getMainWindow()->showShareBrowser_gui(HintedUser(ClientManager::getInstance()->getMe(),hub->client->getHubUrl()),sm->getBZXmlFile(),dcpp::Util::emptyString, 0, true);
}

void Hub::onRefreshShare(GtkWidget* ,GVariant*, gpointer data)
{
	Hub* hub = (Hub*)data;
	ShareManager *sm = hub->client->getShareManager();
	if(!sm->getName().empty())
		sm->refresh(true,true,false);

}

void Hub::onReconnectItemTab(GtkWidget* ,GVariant*, gpointer data)
{ 
	((Hub*)data)->reconnect_client();
}

void Hub::onCloseItem(GtkWidget* ,GVariant*, gpointer data)
{
    BookEntry *entry = (BookEntry *)data;
	if(!entry) return;
    WulforManager::get()->getMainWindow()->removeBookEntry_gui(entry);
}

void Hub::onCopyHubUrl(GtkWidget* widget ,GVariant*, gpointer data)
{
	Hub *hub = (Hub *)data;
	if(!hub) return;
	
	GValue value = G_VALUE_INIT;
	g_value_init (&value, G_TYPE_STRING);
	g_value_set_string (&value, hub->address.c_str());

	// Store the value in the clipboard object
	GdkClipboard *clipboard = gtk_widget_get_clipboard (widget);
	gdk_clipboard_set_value (clipboard, &value);

	g_value_unset (&value);
 
}

void Hub::onAddFavItem(GtkWidget* ,GVariant*, gpointer data)
{
	Hub *hub = (Hub *)data;
	if(!hub) return;
	hub->addAsFavorite_client();
}

void Hub::onRemoveFavHub(GtkWidget* ,GVariant*, gpointer data)
{
    Hub *hub = (Hub *)data;
	if(!hub) return;
    hub->removeAsFavorite_client();
}

void Hub::on_setImage_tab(GtkButton*, gpointer data)
{
	Hub *hub = (Hub *)data;
	if(!hub) return;

//	GtkFileDialog *dialog = gtk_file_dialog_new ();

//	 g_signal_connect(dialog, "response",
  //                       G_CALLBACK (onResponseSetTextIcon),
    //                     (gpointer)hub);
}
void Hub::onResponseSetTextIcon(GtkDialog *dialog,
                    int        response,
                    gpointer   data)
{
	/*Hub * hub = (Hub*)data;
		g_autofree gchar *filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));

		if(WulforUtil::is_format_supported(filename))
		{
			GdkPixbuf *pixbuf = gdk_pixbuf_new_from_file_at_scale(filename,15,15,FALSE,NULL);
			gtk_image_set_from_pixbuf(GTK_IMAGE(hub->tab_image),pixbuf);
			hub->client->set(SettingsManager::HUB_ICON_STR,string(filename));

			FavoriteHubEntryPtr fav = FavoriteManager::getInstance()->getFavoriteHubEntry(hub->client->getHubUrl());

			if(fav) {
				fav->set(SettingsManager::HUB_ICON_STR,hub->client->get(SettingsManager::HUB_ICON_STR,SETTING(HUB_ICON_STR)));
				FavoriteManager::getInstance()->save();
			}

			hub->client->fire(ClientListener::HubUpdated(), hub->client);

		}
*/
}

void Hub::onSetTabText(GtkWidget* ,GVariant*, gpointer data)
{ 
	((Hub *)data)->SetTabText(data); 
}

void Hub::SetTabText(gpointer data)
{
	Hub *hub = (Hub *)data;
	if (!hub) return;
	GtkDialog *dialog =  GTK_DIALOG(gtk_dialog_new_with_buttons (_("Setting for a Tab Text"),
								GTK_WINDOW(WulforManager::get()->getMainWindow()->getContainer()),
								GTK_DIALOG_DESTROY_WITH_PARENT,
								"_OK",
								GTK_RESPONSE_OK,
								"_Cancel",
								GTK_RESPONSE_CANCEL,
								NULL));

	GtkWidget *content_area = gtk_dialog_get_content_area (dialog);
	hub->TabEntry = gtk_entry_new();
	GtkWidget *label = gtk_label_new(_("Text: "));
	GtkWidget *hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL,0);

	GtkWidget *check = gtk_toggle_button_new_with_label(_("Set Icon Aviable"));
	GdkPixbuf *pixbuf =	gdk_pixbuf_new_from_file_at_scale(hub->client->get(SettingsManager::HUB_ICON_STR,SETTING(HUB_ICON_STR)).c_str(),15,15,FALSE,NULL);

	hub->tab_image = gtk_image_new_from_pixbuf(pixbuf);
	hub->tab_button = gtk_button_new_with_label(_("Set Icon: "));

	g_signal_connect(GTK_BUTTON(hub->tab_button), "clicked", G_CALLBACK(on_setImage_tab), hub);
	g_signal_connect(GTK_TOGGLE_BUTTON(check), "toggled", G_CALLBACK(onToglleButtonIcon), hub);

	gtk_box_append(GTK_BOX(hbox), check);
	gtk_box_append(GTK_BOX(hbox), hub->tab_button);
	gtk_box_append(GTK_BOX(hbox), hub->tab_image);
	gtk_box_append(GTK_BOX(content_area), label);
	gtk_box_append(GTK_BOX(content_area), hub->TabEntry);
	gtk_box_append(GTK_BOX(content_area), hbox);

	gtk_widget_show(hub->tab_button);
	gtk_widget_show(hub->tab_image);
	gtk_widget_show(hub->TabEntry);
	gtk_widget_show(label);
	gtk_widget_show(hbox);
	gtk_widget_show(check);
	gtk_editable_set_text(GTK_EDITABLE(hub->TabEntry) , hub->client->get(SettingsManager::HUB_TEXT_STR,SETTING(HUB_TEXT_STR)).c_str());

 	g_signal_connect(dialog, "response",
                         G_CALLBACK (onResponseSetText),
                         (gpointer)this);

}

void Hub::onResponseSetText(GtkDialog *dialog,
                    int        response,
                    gpointer   data)
{
	Hub* hub = (Hub*)data;
	if(response == GTK_RESPONSE_OK)
	{
		const gchar *text = gtk_editable_get_text(GTK_EDITABLE(hub->TabEntry));
		hub->client->set(SettingsManager::HUB_TEXT_STR,string(text));
		hub->client->fire(ClientListener::HubUpdated(), hub->client);

		FavoriteHubEntryPtr fav = FavoriteManager::getInstance()->getFavoriteHubEntry(hub->client->getHubUrl());
		if(fav) {
			fav->set(SettingsManager::HUB_TEXT_STR,hub->client->get(SettingsManager::HUB_TEXT_STR,SETTING(HUB_TEXT_STR)));
			FavoriteManager::getInstance()->save();
		}
	}
}

void Hub::onToglleButtonIcon(GtkToggleButton *button, gpointer data)
{
	Hub *pHub = (Hub *)data;
	if (!pHub) return;
	if (button == NULL) return;

	gboolean bActive = gtk_toggle_button_get_active(button);

	if(bActive)
	{
		pHub->client->set(SettingsManager::HUB_ICON_STR,string());
		pHub->client->fire(ClientListener::HubUpdated(), pHub->client);

		FavoriteHubEntry* fav = FavoriteManager::getInstance()->getFavoriteHubEntry(pHub->client->getHubUrl());
		if(fav) {
			fav->set(SettingsManager::HUB_ICON_STR,string());
			FavoriteManager::getInstance()->save();
		}
	}

	if( pHub->tab_button)
        gtk_widget_set_sensitive(pHub->tab_button, !bActive);

	if( pHub->tab_image)
        gtk_widget_set_sensitive(pHub->tab_image, !bActive);
}


gboolean Hub::key_pressed_gui ( GtkEventControllerKey* self,  guint keyval,  guint keycode,  GdkModifierType state,  gpointer data )
{

	if( keyval == GDK_KEY_Return  ) {
			onSendMessage_gui(NULL,data);
			return TRUE;
	}
	return FALSE;
}

void Hub::key_released_gui (  GtkEventControllerKey* self,  guint keyval,  guint keycode,  GdkModifierType state,  gpointer data )
{

}

