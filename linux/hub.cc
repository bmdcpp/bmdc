/*
 * Copyright © 2004-2012 Jens Oknelid, paskharen@gmail.com
 * Copyright © 2010-2012 Mank
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

#include "hub.hh"

#include <dcpp/FavoriteManager.h>
#include <dcpp/HashManager.h>
#include <dcpp/SearchManager.h>
#include <dcpp/ShareManager.h>
#include <dcpp/UserCommand.h>
#include <dcpp/version.h>
#include <dcpp/ChatMessage.h> //NOTE: core 0.762
#include <dcpp/GeoManager.h>
#include <dcpp/RegEx.h>
#ifdef _USELUA
	#include <dcpp/ScriptManager.h>
#endif
#include <dcpp/PluginManager.h>

#include "privatemessage.hh"
#include "search.hh"
#include "settingsmanager.hh"
#include "emoticonsdialog.hh"
#include "emoticons.hh"
#include "UserCommandMenu.hh"
#include "wulformanager.hh"
#include "WulforUtil.hh"
#include "version.hh"

using namespace std;
using namespace dcpp;

const string Hub::tagPrefix = "#";

Hub::Hub(const string &address, const string &encoding):
	BookEntry(Entry::HUB, address, "hub.glade", address),
	client(NULL),
	historyIndex(0),
	totalShared(0),
	address(address),
	encoding(encoding),
	scrollToBottom(TRUE),
	PasswordDialog(FALSE),
	WaitingPassword(FALSE),
	ImgLimit(0)
{
	// Configure the dialog
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(getWidget("userListCheckButton")), TRUE);

	// Initialize nick treeview
	nickView.setView(GTK_TREE_VIEW(getWidget("nickView")), true, "hub");
	nickView.insertColumn(_("Nick"), G_TYPE_STRING, TreeView::ICON_STRING_TEXT_COLOR, 100, "Icon", "NickColor");
	nickView.insertColumn(_("Shared"), G_TYPE_INT64, TreeView::SIZE, 75);
	nickView.insertColumn(_("Description"), G_TYPE_STRING, TreeView::STRING, 85);
	nickView.insertColumn(_("Tag"), G_TYPE_STRING, TreeView::STRING, 100);
	nickView.insertColumn(_("Connection"), G_TYPE_STRING, TreeView::STRING, 85);
	nickView.insertColumn("IP", G_TYPE_STRING, TreeView::STRING, 85);
	nickView.insertColumn(_("eMail"), G_TYPE_STRING, TreeView::STRING, 90);
	//BMDC++
	nickView.insertColumn(_("Country"), G_TYPE_STRING, TreeView::PIXBUF_STRING, 70, "Pixbuf");
	nickView.insertColumn(_("Exact Share"), G_TYPE_INT64, TreeView::EXSIZE, 100);
	nickView.insertColumn(_("Slots"), G_TYPE_STRING, TreeView::STRING, 50);
    nickView.insertColumn(_("Hubs"), G_TYPE_STRING, TreeView::STRING, 50);
    nickView.insertColumn("PK", G_TYPE_STRING, TreeView::STRING, 80);
    nickView.insertColumn(_("Cheat"), G_TYPE_STRING, TreeView::STRING, 80);
    nickView.insertColumn(_("Generator"), G_TYPE_STRING, TreeView::STRING, 80);
    nickView.insertColumn(_("Support"), G_TYPE_STRING, TreeView::STRING, 80);
     //BMDC++
	nickView.insertHiddenColumn("Icon", G_TYPE_STRING);
	nickView.insertHiddenColumn("Nick Order", G_TYPE_STRING);
	nickView.insertHiddenColumn("CID", G_TYPE_STRING);
	nickView.insertHiddenColumn("NickColor", G_TYPE_STRING);
	//BMDC++
	nickView.insertHiddenColumn("Pixbuf", GDK_TYPE_PIXBUF);
	nickView.insertHiddenColumn("Client Type", G_TYPE_STRING);
	nickView.finalize();
	nickStore = gtk_list_store_newv(nickView.getColCount(), nickView.getGTypes());
	gtk_tree_view_set_model(nickView.get(), GTK_TREE_MODEL(nickStore));
	g_object_unref(nickStore);

	nickSelection = gtk_tree_view_get_selection(nickView.get());
	gtk_tree_selection_set_mode(gtk_tree_view_get_selection(nickView.get()), GTK_SELECTION_MULTIPLE);
	string sort = BOOLSETTING(SORT_FAVUSERS_FIRST)? "Client Type" : "Nick Order";
	nickView.setSortColumn_gui(_("Nick"), sort);
	gtk_tree_sortable_set_sort_column_id(GTK_TREE_SORTABLE(nickStore), nickView.col(sort), GTK_SORT_ASCENDING);
	gtk_tree_view_column_set_sort_indicator(gtk_tree_view_get_column(nickView.get(), nickView.col(_("Nick"))), TRUE);
	gtk_tree_view_set_fixed_height_mode(nickView.get(), TRUE);
	//BMDC++
	nickView.setSelection(nickSelection);
	nickView.buildCopyMenu(getWidget("CopyMenu"));

    g_object_set(G_OBJECT(nickView.get()), "has-tooltip", TRUE, NULL);
    g_signal_connect(nickView.get(), "query-tooltip", G_CALLBACK(onUserListTooltip_gui), (gpointer)this);
	g_signal_connect (gtk_tree_view_get_selection (GTK_TREE_VIEW (nickView.get())), "changed", G_CALLBACK (selection_changed_userlist_gui), GTK_WIDGET(nickView.get()));
	/* Set a tooltip on the column */
	set_Header_tooltip_gui();

	// Initialize the chat window
	if (BOOLSETTING(USE_OEM_MONOFONT))
	{
		PangoFontDescription *fontDesc = pango_font_description_new();
		pango_font_description_set_family(fontDesc, "Mono");
		gtk_widget_modify_font(getWidget("chatText"), fontDesc);
		pango_font_description_free(fontDesc);
	}
	//..set Colors
	string strcolor = WGETS("background-color-chat");
	GdkColor color;
	gdk_color_parse(strcolor.c_str(),&color);

	gtk_widget_modify_base(getWidget("chatText"),GTK_STATE_NORMAL,&color);
	gtk_widget_modify_base(getWidget("chatText"),GTK_STATE_PRELIGHT,&color);
	gtk_widget_modify_base(getWidget("chatText"),GTK_STATE_ACTIVE,&color);
	gtk_widget_modify_base(getWidget("chatText"),GTK_STATE_INSENSITIVE,&color);

	// the reference count on the buffer is not incremented and caller of this function won't own a new reference.
	chatBuffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(getWidget("chatText")));

	/* initial markers */
	GtkTextIter iter;
	gtk_text_buffer_get_end_iter(chatBuffer, &iter);

	chatMark = gtk_text_buffer_create_mark(chatBuffer, NULL, &iter, FALSE);
	start_mark = gtk_text_buffer_create_mark(chatBuffer, NULL, &iter, TRUE);
	end_mark = gtk_text_buffer_create_mark(chatBuffer, NULL, &iter, TRUE);
	tag_mark = gtk_text_buffer_create_mark(chatBuffer, NULL, &iter, FALSE);
	emot_mark = gtk_text_buffer_create_mark(chatBuffer, NULL, &iter, TRUE);

	handCursor = gdk_cursor_new(GDK_HAND2);

#if !GTK_CHECK_VERSION(2, 12, 0)
	tips = gtk_tooltips_new();
	g_object_ref_sink(tips);
	/**/
	statusTips = gtk_tooltips_new();
	g_object_ref_sinK(statusTips);
#endif
	// image magnet
	imageLoad.first = "";
	imageLoad.second = NULL;
	imageMagnet.first = "";
	imageMagnet.second = "";

	// menu
	g_object_ref_sink(getWidget("nickMenu"));
	g_object_ref_sink(getWidget("magnetMenu"));
	g_object_ref_sink(getWidget("linkMenu"));
	g_object_ref_sink(getWidget("hubMenu"));
	g_object_ref_sink(getWidget("chatCommandsMenu"));
	g_object_ref_sink(getWidget("imageMenu"));

	// Initialize the user command menu
	userCommandMenu = new UserCommandMenu(getWidget("usercommandMenu"), ::UserCommand::CONTEXT_USER);//NOTE: core 0.762
	addChild(userCommandMenu);
    // Hub ...
    userCommandMenu1 = new UserCommandMenu(BookEntry::createmenu(), ::UserCommand::CONTEXT_HUB);
    addChild(userCommandMenu1);
	// Ip ...
	userCommandMenu2 = new UserCommandMenu(getWidget("ipmenu"), ::UserCommand::CONTEXT_IP);
	addChild(userCommandMenu2);

	// Emoticons dialog
	emotdialog = new EmoticonsDialog(getWidget("chatEntry"), getWidget("emotButton"), getWidget("emotPacksMenu"));
	if (!WGETB("emoticons-use"))
		gtk_widget_set_sensitive(getWidget("emotButton"), FALSE);
	useEmoticons = TRUE;

	// Chat commands
	g_object_set_data_full(G_OBJECT(getWidget("awayCommandItem")), "command", g_strdup("/away"), g_free);
	g_signal_connect(getWidget("awayCommandItem"), "activate", G_CALLBACK(onCommandClicked_gui), (gpointer)this);

	g_object_set_data_full(G_OBJECT(getWidget("backCommandItem")), "command", g_strdup("/back"), g_free);
	g_signal_connect(getWidget("backCommandItem"), "activate", G_CALLBACK(onCommandClicked_gui), (gpointer)this);

	g_object_set_data_full(G_OBJECT(getWidget("clearCommandItem")), "command", g_strdup("/clear"), g_free);
	g_signal_connect(getWidget("clearCommandItem"), "activate", G_CALLBACK(onCommandClicked_gui), (gpointer)this);

	g_object_set_data_full(G_OBJECT(getWidget("favCommandItem")), "command", g_strdup("/fav"), g_free);
	g_signal_connect(getWidget("favCommandItem"), "activate", G_CALLBACK(onCommandClicked_gui), (gpointer)this);

	g_object_set_data_full(G_OBJECT(getWidget("lsfuCommandItem")), "command", g_strdup("/lsfu"), g_free);
	g_signal_connect(getWidget("lsfuCommandItem"), "activate", G_CALLBACK(onCommandClicked_gui), (gpointer)this);

	g_object_set_data_full(G_OBJECT(getWidget("helpCommandItem")), "command", g_strdup("/help"), g_free);
	g_signal_connect(getWidget("helpCommandItem"), "activate", G_CALLBACK(onCommandClicked_gui), (gpointer)this);

	g_object_set_data_full(G_OBJECT(getWidget("joinCommandItem")), "command", g_strdup("/join"), g_free);
	g_signal_connect(getWidget("joinCommandItem"), "activate", G_CALLBACK(onCommandClicked_gui), (gpointer)this);

	g_object_set_data_full(G_OBJECT(getWidget("meCommandItem")), "command", g_strdup("/me"), g_free);
	g_signal_connect(getWidget("meCommandItem"), "activate", G_CALLBACK(onCommandClicked_gui), (gpointer)this);

	g_object_set_data_full(G_OBJECT(getWidget("rebuildCommandItem")), "command", g_strdup("/rebuild"), g_free);
	g_signal_connect(getWidget("rebuildCommandItem"), "activate", G_CALLBACK(onCommandClicked_gui), (gpointer)this);

	g_object_set_data_full(G_OBJECT(getWidget("limitimgCommandItem")), "command", g_strdup("/limg"), g_free);
	g_signal_connect(getWidget("limitimgCommandItem"), "activate", G_CALLBACK(onCommandClicked_gui), (gpointer)this);

	g_object_set_data_full(G_OBJECT(getWidget("versionCommandItem")), "command", g_strdup("/bmdc"), g_free);
	g_signal_connect(getWidget("versionCommandItem"), "activate", G_CALLBACK(onCommandClicked_gui), (gpointer)this);

	// chat commands button
	g_signal_connect(getWidget("chatCommandsButton"), "button-release-event", G_CALLBACK(onChatCommandButtonRelease_gui), (gpointer)this);

	// image menu
	g_signal_connect(getWidget("downloadImageItem"), "activate", G_CALLBACK(onDownloadImageClicked_gui), (gpointer)this);
	g_signal_connect(getWidget("removeImageItem"), "activate", G_CALLBACK(onRemoveImageClicked_gui), (gpointer)this);
	g_signal_connect(getWidget("openImageItem"), "activate", G_CALLBACK(onOpenImageClicked_gui), (gpointer)this);

	GtkAdjustment *adjustment = gtk_scrolled_window_get_vadjustment(GTK_SCROLLED_WINDOW(getWidget("chatScroll")));

	// Connect the signals to their callback functions.
	g_signal_connect(getContainer(), "focus-in-event", G_CALLBACK(onFocusIn_gui), (gpointer)this);
	g_signal_connect(nickView.get(), "button-press-event", G_CALLBACK(onNickListButtonPress_gui), (gpointer)this);
	g_signal_connect(nickView.get(), "button-release-event", G_CALLBACK(onNickListButtonRelease_gui), (gpointer)this);
	g_signal_connect(nickView.get(), "key-release-event", G_CALLBACK(onNickListKeyRelease_gui), (gpointer)this);
	g_signal_connect(getWidget("chatEntry"), "activate", G_CALLBACK(onSendMessage_gui), (gpointer)this);
	g_signal_connect(getWidget("chatEntry"), "key-press-event", G_CALLBACK(onEntryKeyPress_gui), (gpointer)this);
	g_signal_connect(getWidget("chatText"), "motion-notify-event", G_CALLBACK(onChatPointerMoved_gui), (gpointer)this);
	g_signal_connect(getWidget("chatText"), "visibility-notify-event", G_CALLBACK(onChatVisibilityChanged_gui), (gpointer)this);
	g_signal_connect(adjustment, "value_changed", G_CALLBACK(onChatScroll_gui), (gpointer)this);
	g_signal_connect(adjustment, "changed", G_CALLBACK(onChatResize_gui), (gpointer)this);
	g_signal_connect(getWidget("nickToChatItem"), "activate", G_CALLBACK(onNickToChat_gui), (gpointer)this);
	g_signal_connect(getWidget("copyNickItem"), "activate", G_CALLBACK(onCopyNickItemClicked_gui), (gpointer)this);
	g_signal_connect(getWidget("browseItem"), "activate", G_CALLBACK(onBrowseItemClicked_gui), (gpointer)this);
	g_signal_connect(getWidget("matchItem"), "activate", G_CALLBACK(onMatchItemClicked_gui), (gpointer)this);
	g_signal_connect(getWidget("msgItem"), "activate", G_CALLBACK(onMsgItemClicked_gui), (gpointer)this);
	g_signal_connect(getWidget("grantItem"), "activate", G_CALLBACK(onGrantItemClicked_gui), (gpointer)this);
	g_signal_connect(getWidget("copyLinkItem"), "activate", G_CALLBACK(onCopyURIClicked_gui), (gpointer)this);
	g_signal_connect(getWidget("openLinkItem"), "activate", G_CALLBACK(onOpenLinkClicked_gui), (gpointer)this);
	g_signal_connect(getWidget("copyhubItem"), "activate", G_CALLBACK(onCopyURIClicked_gui), (gpointer)this);
	g_signal_connect(getWidget("openhubItem"), "activate", G_CALLBACK(onOpenHubClicked_gui), (gpointer)this);
	g_signal_connect(getWidget("copyMagnetItem"), "activate", G_CALLBACK(onCopyURIClicked_gui), (gpointer)this);
	g_signal_connect(getWidget("searchMagnetItem"), "activate", G_CALLBACK(onSearchMagnetClicked_gui), (gpointer)this);
	g_signal_connect(getWidget("magnetPropertiesItem"), "activate", G_CALLBACK(onMagnetPropertiesClicked_gui), (gpointer)this);
	g_signal_connect(getWidget("removeUserItem"), "activate", G_CALLBACK(onRemoveUserItemClicked_gui), (gpointer)this);
	g_signal_connect(getWidget("userListCheckButton"), "toggled", G_CALLBACK(onUserListToggled_gui), (gpointer)this);
	g_signal_connect(getWidget("emotButton"), "button-release-event", G_CALLBACK(onEmotButtonRelease_gui), (gpointer)this);
	g_signal_connect(getWidget("favoriteUserItem"), "activate", G_CALLBACK(onAddFavoriteUserClicked_gui), (gpointer)this);
	g_signal_connect(getWidget("removeFavoriteUserItem"), "activate", G_CALLBACK(onRemoveFavoriteUserClicked_gui), (gpointer)this);
	//BMDC++
	g_signal_connect(getWidget("ignoreMenuItem"), "activate", G_CALLBACK(onAddIgnoreUserItemClicked_gui), (gpointer)this);
	g_signal_connect(getWidget("removeIgnoreMenuItem"), "activate", G_CALLBACK(onRemoveIgnoreUserItemClicked_gui), (gpointer)this);
	g_signal_connect(getWidget("reportItem"), "activate", G_CALLBACK(onShowReportClicked_gui), (gpointer)this);
	g_signal_connect(getWidget("checkItem"), "activate", G_CALLBACK(onCheckFLItemClicked_gui), (gpointer)this);
	g_signal_connect(getWidget("testItem"), "activate", G_CALLBACK(onTestSURItemClicked_gui), (gpointer)this);
	g_signal_connect(getWidget("protectItem"), "activate", G_CALLBACK(onProtectUserClicked_gui), (gpointer)this);
	g_signal_connect(getWidget("UnProtectItem"), "activate", G_CALLBACK(onUnProtectUserClicked_gui), (gpointer)this);
	g_signal_connect(getWidget("menurefresh"), "activate", G_CALLBACK(onRefreshUserListClicked_gui), (gpointer)this);
	g_signal_connect(getWidget("buttonrefresh"), "clicked", G_CALLBACK(onRefreshUserListClicked_gui), (gpointer)this);
	/**/
     g_signal_connect(getWidget("ripeitem"), "activate", G_CALLBACK(onRipeDbItem_gui),(gpointer)this);
     g_signal_connect(getWidget("copyipItem"), "activate", G_CALLBACK(onCopyIpItem_gui),(gpointer)this);
	//end
	g_signal_connect(getWidget("downloadBrowseItem"), "activate", G_CALLBACK(onDownloadToClicked_gui), (gpointer)this);
	g_signal_connect(getWidget("downloadItem"), "activate", G_CALLBACK(onDownloadClicked_gui), (gpointer)this);
	g_signal_connect(getWidget("italicButton"), "clicked", G_CALLBACK(onItalicButtonClicked_gui), (gpointer)this);
	g_signal_connect(getWidget("boldButton"), "clicked", G_CALLBACK(onBoldButtonClicked_gui), (gpointer)this);
	g_signal_connect(getWidget("underlineButton"), "clicked", G_CALLBACK(onUnderlineButtonClicked_gui), (gpointer)this);

	gtk_widget_grab_focus(getWidget("chatEntry"));

	// Set the pane position
	gint panePosition = WGETI("nick-pane-position");
	if (panePosition > 10)
	{
		gint width;
		GtkWindow *window = GTK_WINDOW(WulforManager::get()->getMainWindow()->getContainer());
		gtk_window_get_size(window, &width, NULL);
		gtk_paned_set_position(GTK_PANED(getWidget("pane")), width - panePosition);
	}

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
	TagsMap[Tag::TAG_HIGHL] = createTag_gui("TAG_HIGHL", Tag::TAG_HIGHL);

	BoldTag = gtk_text_buffer_create_tag(chatBuffer, "TAG_WEIGHT", "weight", PANGO_WEIGHT_BOLD, NULL);
	UnderlineTag = gtk_text_buffer_create_tag(chatBuffer, "TAG_UNDERLINE", "underline", PANGO_UNDERLINE_SINGLE, NULL);
	ItalicTag = gtk_text_buffer_create_tag(chatBuffer, "TAG_STYLE", "style", PANGO_STYLE_ITALIC, NULL);

	// Initialize favorite users list
	FavoriteManager::FavoriteMap map = FavoriteManager::getInstance()->getFavoriteUsers();
	FavoriteManager::FavoriteMap::const_iterator it;

	for (it = map.begin(); it != map.end(); ++it)
	{
		if (it->second.getUrl() == address)
		{
			userFavoriteMap.insert(UserMap::value_type(it->first.toBase32(), it->second.getNick()));
		}
	}

	// set default select tag (fix error show cursor in neutral space).
	selectedTag = TagsMap[Tag::TAG_GENERAL];

	RecentHubEntry entry;
	entry.setName("*");
	entry.setDescription("***");
	entry.setUsers("0");
	entry.setShared("*");
	entry.setServer(address);
	RecentHubEntry* r = FavoriteManager::getInstance()->getRecentHubEntry(address);
	if(r == NULL)
		FavoriteManager::getInstance()->addRecent(entry);
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

void Hub::setColorRow(string cell)
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

void Hub::makeColor(GtkTreeViewColumn *column,GtkCellRenderer *cell, GtkTreeModel *model, GtkTreeIter *iter,gpointer data)
{
		Hub *hub = (Hub *)data;
		string color = "#A52A2A";
		gchar *cltype;
		int64_t size;
		string sizeString;
		if(model == NULL)
			return;
		if(iter == NULL)
			return;
		if(column == NULL)
			return;
		if(cell == NULL)
			return;

		gtk_tree_model_get(model,iter,
		                     1, &size,
						20,&cltype,
						-1);
		gchar *a = hub->g_substr(cltype,0,0);
		//Hub&Bot
		if ( strcmp(a,"A") == 0)
		{
		  color = WGETS("userlist-bg-bot-hub");
		}//Op
		else if ( strcmp(a,"B") == 0)
		{
			color = WGETS("userlist-bg-operator");
		}//Fav
		else if ( strcmp(a,"C") == 0)
		{
     	   color = WGETS("userlist-bg-favorite");
		}//ignored
		else if ( strcmp(a, "I") == 0)
		{
		   color = WGETS("userlist-bg-ignored");
		}//protected
		else if ( strcmp(a,"R") == 0)
		{
		   color	= WGETS("userlist-bg-protected");
		}//pasive
		else if (strcmp(a,"P") == 0)
		{
		   color = WGETS("userlist-bg-pasive");
		}
		else {
		  color = WGETS("userlist-bg-normal");
		}

		g_object_set(cell,"cell-background-set",TRUE,"cell-background",color.c_str(),NULL);
		gchar *title = const_cast<gchar*>(gtk_tree_view_column_get_title(column));

		if(strcmp(title,_("Shared")) == 0)
		{
			if (size >= 0)
			{
				sizeString = dcpp::Util::formatBytes(size);
			}
			g_object_set(cell, "text", sizeString.c_str(), NULL);
		}
		g_free(cltype);
		g_free(a);
}

Hub::~Hub()
{
	RecentHubEntry* r = FavoriteManager::getInstance()->getRecentHubEntry(address);

	if(r)
	{
		r->setName(client->getHubName());
		r->setDescription(client->getHubDescription());
		r->setUsers(Util::toString(client->getUserCount()));
		r->setShared(Util::toString(client->getAvailable()));
		FavoriteManager::getInstance()->updateRecent(r);
	}

	disconnect_client();

	// Save the pane position
	gint width;
	GtkWindow *window = GTK_WINDOW(WulforManager::get()->getMainWindow()->getContainer());
	gtk_window_get_size(window, &width, NULL);
	gint panePosition = width - gtk_paned_get_position(GTK_PANED(getWidget("pane")));
	if (panePosition > 10)
		WSET("nick-pane-position", panePosition);

	if (handCursor)
	{
		gdk_cursor_unref(handCursor);
		handCursor = NULL;
	}

	delete emotdialog;

#if !GTK_CHECK_VERSION(2, 12, 0)
	g_object_unref(tips);
	g_object_unref(statusTips);
#endif
	g_object_unref(getWidget("nickMenu"));
	g_object_unref(getWidget("magnetMenu"));
	g_object_unref(getWidget("linkMenu"));
	g_object_unref(getWidget("hubMenu"));
	g_object_unref(getWidget("chatCommandsMenu"));
	g_object_unref(getWidget("imageMenu"));
}

void Hub::show()
{
	// Connect to the hub
	typedef Func2<Hub, string, string> F2;
	F2 *func = new F2(this, &Hub::connectClient_client, address, encoding);
	WulforManager::get()->dispatchClientFunc(func);
}

void Hub::selection_changed_userlist_gui(GtkTreeSelection *selection, GtkWidget *tree_view)
{
	gtk_widget_trigger_tooltip_query (tree_view);
}

void Hub::set_Header_tooltip_gui()//How beter ?
{
	GtkTreeViewColumn *column = gtk_tree_view_get_column (nickView.get(), 0);
	gtk_tree_view_column_set_clickable (column, TRUE);
	g_object_set (column->button, "tooltip-text", "Nick", NULL);

	GtkTreeViewColumn *column1 = gtk_tree_view_get_column (nickView.get(), 1);
	gtk_tree_view_column_set_clickable (column1, TRUE);
	g_object_set (column1->button, "tooltip-text", "Shared", NULL);

	GtkTreeViewColumn *column2 = gtk_tree_view_get_column (nickView.get(), 2);
	gtk_tree_view_column_set_clickable (column2, TRUE);
	g_object_set (column2->button, "tooltip-text", "Description", NULL);

	GtkTreeViewColumn *column3 = gtk_tree_view_get_column (nickView.get(), 3);
	gtk_tree_view_column_set_clickable (column3, TRUE);
	g_object_set (column3->button, "tooltip-text", "Tag", NULL);

	GtkTreeViewColumn *column4 = gtk_tree_view_get_column (nickView.get(), 4);
	gtk_tree_view_column_set_clickable (column4, TRUE);
	g_object_set (column4->button, "tooltip-text", "Conection", NULL);

	GtkTreeViewColumn *column5 = gtk_tree_view_get_column (nickView.get(), 5);
	gtk_tree_view_column_set_clickable (column5, TRUE);
	g_object_set (column5->button, "tooltip-text", "IP", NULL);

	GtkTreeViewColumn *column6 = gtk_tree_view_get_column (nickView.get(), 6);
	gtk_tree_view_column_set_clickable (column6, TRUE);
	g_object_set (column6->button, "tooltip-text", "eMail", NULL);

	GtkTreeViewColumn *column7 = gtk_tree_view_get_column (nickView.get(), 7);
	gtk_tree_view_column_set_clickable (column7, TRUE);
	g_object_set (column7->button, "tooltip-text", "Country", NULL);

	GtkTreeViewColumn *column8 = gtk_tree_view_get_column (nickView.get(), 8);
	gtk_tree_view_column_set_clickable (column8, TRUE);
	g_object_set (column8->button, "tooltip-text", "Exact Share", NULL);

	GtkTreeViewColumn *column9 = gtk_tree_view_get_column (nickView.get(), 9);
	gtk_tree_view_column_set_clickable (column9, TRUE);
	g_object_set (column9->button, "tooltip-text", "Slots", NULL);

	GtkTreeViewColumn *column10 = gtk_tree_view_get_column (nickView.get(), 10);
	gtk_tree_view_column_set_clickable (column10, TRUE);
	g_object_set (column10->button, "tooltip-text", "Hubs", NULL);

	GtkTreeViewColumn *column11 = gtk_tree_view_get_column (nickView.get(), 11);
	gtk_tree_view_column_set_clickable (column11, TRUE);
	g_object_set (column11->button, "tooltip-text", "PK", NULL);

	GtkTreeViewColumn *column12 = gtk_tree_view_get_column (nickView.get(), 12);
	gtk_tree_view_column_set_clickable (column12, TRUE);
	g_object_set (column12->button, "tooltip-text", "Cheat", NULL);

	GtkTreeViewColumn *column13 = gtk_tree_view_get_column (nickView.get(), 13);
	gtk_tree_view_column_set_clickable (column13, TRUE);
	g_object_set (column13->button, "tooltip-text", "Generator", NULL);

	GtkTreeViewColumn *column14 = gtk_tree_view_get_column (nickView.get(), 14);
	gtk_tree_view_column_set_clickable (column14, TRUE);
	g_object_set (column14->button, "tooltip-text", "Support", NULL);
}

gboolean Hub::onUserListTooltip_gui(GtkWidget *widget, gint x, gint y, gboolean keyboard_tip, GtkTooltip *_tooltip, gpointer data)
{
  GtkTreeIter iter;
  GtkTreeView *tree_view = GTK_TREE_VIEW (widget);
  GtkTreeModel *model = gtk_tree_view_get_model (tree_view);
  GtkTreePath *path = NULL;
  gchar *tmp, *tag, *desc,*con,*ip,*e,*country,*slots,*hubs,*pk,*cheat,*gen,*sup,*cid,*pathstring;
  gint64 ssize;

  char buffer[1000];

  if (!gtk_tree_view_get_tooltip_context (tree_view, &x, &y,
					  keyboard_tip,
					  &model, &path, &iter))
    return FALSE;

  gtk_tree_model_get (model, &iter, 0, &tmp,
									2, &desc,
									3, &tag,
									4, &con,
									5, &ip,
									6, &e,
									7, &country,
									8, &ssize,
									9, &slots,
									10, &hubs,
									11, &pk,
									12, &cheat,
									13, &gen,
									14, &sup,
									17, &cid,
  									-1);
  pathstring = gtk_tree_path_to_string (path);
  string sharesize  = Util::formatBytes(ssize);
  g_snprintf (buffer, 1000, " Nick: %s\n Connection: %s\n Description: %s\n Tag: %s\n Share: %s\n IP: %s\n eMail: %s\nCountry: %s\n Slots: %s\n Hubs: %s\n PK: %s\n Cheat: %s\n Generator: %s\n Support: %s\n CID: %s", tmp, con,desc, tag , sharesize.c_str() ,ip, e, country, slots, hubs, pk, cheat, gen, sup, cid);
  gtk_tooltip_set_text (_tooltip, buffer);

  gtk_tree_view_set_tooltip_row (tree_view, _tooltip, path);

  gtk_tree_path_free (path);
  g_free (pathstring);
  g_free (tmp);

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
            #if !GTK_CHECK_VERSION(2, 12, 0)
                gtk_tooltips_set_tip (statusTips, getWidget("statusMain"), statusTextOnToolTip.c_str(), NULL);
            #else
                gtk_widget_set_tooltip_text(getWidget("statusMain"), statusTextOnToolTip.c_str());
            #endif
		}

		gtk_statusbar_pop(GTK_STATUSBAR(getWidget(statusBar)), 0);
		gtk_statusbar_push(GTK_STATUSBAR(getWidget(statusBar)), 0, text.c_str());
	}
}

bool Hub::findUser_gui(const string &cid, GtkTreeIter *iter)
{
	unordered_map<string, GtkTreeIter>::const_iterator it = userIters.find(cid);

	if (it != userIters.end())
	{
		if (iter)
			*iter = it->second;

		return TRUE;
	}

	return FALSE;
}

bool Hub::findNick_gui(const string &nick, GtkTreeIter *iter)
{
	unordered_map<string, string>::const_iterator it = userMap.find(nick);

	if (it != userMap.end())
		return findUser_gui(it->second, iter);

	return FALSE;
}

void Hub::updateUser_gui(ParamMap params)
{
	GtkTreeIter iter;
	int64_t shared = Util::toInt64(params["Shared"]);
	const string& cid = params["CID"];
	const string icon = "bmdc-" + params["Icon"];
	const string &Nick = params["Nick"];
	const string &nickOrder = params["Nick Order"];
	bool favorite = userFavoriteMap.find(cid) != userFavoriteMap.end();
	//[BMDC++
	bool isPasive = userPasiveMap.find(cid) != userPasiveMap.end();
	bool isOperator = userOpMap.find(cid) != userOpMap.end();
	bool isIgnore = userIgnoreMap.find(cid) != userIgnoreMap.end();
	bool isProtected = userProtectMap.find(cid) != userProtectMap.end();

	GdkPixbuf *pixbuf = WulforUtil::LoadCountryPixbuf(params["Abbrevation"]);
	if(pixbuf != NULL)
        gdk_pixbuf_ref(pixbuf);

	//Color of OP,Pasive, Fav, Ignore, Protect
	string nickColor = (isProtected ? WGETS("userlist-text-protected") :(isOperator ? WGETS("userlist-text-operator") : (isPasive ? WGETS("userlist-text-pasive") :(favorite ? WGETS("userlist-text-favorite") : ( isIgnore ? WGETS("userlist-text-ignored") : WGETS("userlist-text-normal"))))));

	if (findUser_gui(cid, &iter))
	{
		totalShared += shared - nickView.getValue<int64_t>(&iter, _("Shared"));
		string nick = nickView.getString(&iter, _("Nick"));

		if (nick != Nick)
		{
			// User has changed nick, update userMap and remove the old Nick tag
			userMap.erase(nick);
			removeTag_gui(nick);
			userMap.insert(UserMap::value_type(Nick, cid));

			// update favorite
			if (favorite)
				userFavoriteMap[cid] = Nick;
			if (isPasive)
                 userPasiveMap[cid] = Nick;
            if(isOperator)
                 userOpMap[cid] = Nick;
            if(isIgnore)
                 userIgnoreMap[cid] = Nick;
            if(isProtected)
                 userProtectMap[cid] = Nick;
		}

		gtk_list_store_set(nickStore, &iter,
			nickView.col(_("Nick")), Nick.c_str(),
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
			nickView.col("NickColor"), nickColor.c_str(),
            nickView.col("Pixbuf"), pixbuf,
            nickView.col("Client Type"), params["Type"].c_str(),
			-1);
	}
	else
	{
		totalShared += shared;
		userMap.insert(UserMap::value_type(Nick, cid));

		gtk_list_store_insert_with_values(nickStore, &iter, userMap.size(),
			nickView.col(_("Nick")), Nick.c_str(),
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
			nickView.col("NickColor"), nickColor.c_str(),
            nickView.col("Pixbuf"), pixbuf,
            nickView.col("Client Type"), params["Type"].c_str(),
			-1);

		userIters.insert(UserIters::value_type(cid, iter));

		if (BOOLSETTING(SHOW_JOINS))
		{
			// Show joins in chat by default
			addStatusMessage_gui(Nick + _(" has joined"), Msg::STATUS, favorite? Sound::FAVORITE_USER_JOIN : Sound::NONE);
			string message = Nick + _(" has joined hub ") + client->getHubName();
			WulforManager::get()->getMainWindow()->addPrivateStatusMessage_gui(Msg::STATUS, cid, message);

			if (favorite)
				Notify::get()->showNotify("", message, Notify::FAVORITE_USER_JOIN);
		}
		else if (BOOLSETTING(FAV_SHOW_JOINS) && favorite)
		{
			// Only show joins for favorite users
			string message = Nick + _(" has joined hub ") + client->getHubName();
			addStatusMessage_gui(Nick + _(" has joined"), Msg::STATUS, Sound::FAVORITE_USER_JOIN);
			WulforManager::get()->getMainWindow()->addPrivateStatusMessage_gui(Msg::STATUS, cid, message);
			Notify::get()->showNotify("", message, Notify::FAVORITE_USER_JOIN);
		}
	}
	setColorsRows();
	setStatus_gui("statusUsers", Util::toString(userMap.size()) + _(" Users"));
	setStatus_gui("statusShared", Util::formatBytes(totalShared));
}

void Hub::removeUser_gui(string cid)
{
	GtkTreeIter iter;
	string nick, order;

	if (findUser_gui(cid, &iter))
	{
		order = nickView.getString(&iter, "Client Type");//F
		nick = nickView.getString(&iter, _("Nick"));
		totalShared -= nickView.getValue<int64_t>(&iter, _("Shared"));
		gtk_list_store_remove(nickStore, &iter);
		removeTag_gui(nick);
		userMap.erase(nick);
		//BMDC++
		userOpMap.erase(nick);
		userPasiveMap.erase(nick);
		userProtectMap.erase(nick);
		userIgnoreMap.erase(nick);
		userFavoriteMap.erase(nick);

		userIters.erase(cid);
		setStatus_gui("statusUsers", Util::toString(userMap.size()) + _(" Users"));
		setStatus_gui("statusShared", Util::formatBytes(totalShared));

		if (client->settings.showJoins/*BOOLSETTING(SHOW_JOINS)*/  )
		{
			// Show parts in chat by default
			string message = nick + _(" has quit hub ") + client->getHubName();
			addStatusMessage_gui(nick + _(" has quit"), Msg::STATUS, order[0] == 'C' ? Sound::FAVORITE_USER_QUIT : Sound::NONE);
			WulforManager::get()->getMainWindow()->addPrivateStatusMessage_gui(Msg::STATUS, cid, message);

			if (order[0] == 'C')//f
				Notify::get()->showNotify("", message, Notify::FAVORITE_USER_QUIT);
		}
		else if (client->settings.favShowJoins/*BOOLSETTING(FAV_SHOW_JOINS)*/ && order[0] == 'C')//f
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
	//BMDC++
	userOpMap.clear();
	userPasiveMap.clear();
	userIgnoreMap.clear();
	userFavoriteMap.clear();
	userProtectMap.clear();
	//END
	userIters.clear();
	totalShared = 0;
	setStatus_gui("statusUsers", _("0 Users"));
	setStatus_gui("statusShared", "0 B");
}

void Hub::popupNickMenu_gui()
{
	// Build user command menu
	userCommandMenu->cleanMenu_gui();

	GtkTreeIter iter;
	GList *list = gtk_tree_selection_get_selected_rows(nickSelection, NULL);
	string nick;

	for (GList *i = list; i; i = i->next)
	{
		GtkTreePath *path = (GtkTreePath *)i->data;
		if (gtk_tree_model_get_iter(GTK_TREE_MODEL(nickStore), &iter, path))
		{
			userCommandMenu->addUser(nickView.getString(&iter, "CID"));
			nick += " " + nickView.getString(&iter, _("Nick"));
		}
		gtk_tree_path_free(path);
	}
	g_list_free(list);

	userCommandMenu->addHub(client->getHubUrl());
	userCommandMenu->buildMenu_gui();
	gchar *markup;
    markup = g_markup_printf_escaped ("<span fgcolor=\"blue\" ><b>%s</b></span>", nick.c_str());
    GtkMenuItem *item = GTK_MENU_ITEM(getWidget("nickItem"));
    WulforUtil::remove_signals_from_widget(GTK_WIDGET(item),GDK_ALL_EVENTS_MASK);
    GtkWidget *label = gtk_bin_get_child(GTK_BIN(item));
	gtk_label_set_markup (GTK_LABEL (label), markup);
	g_free(markup);

	gtk_menu_popup(GTK_MENU(getWidget("nickMenu")), NULL, NULL, NULL, NULL, 0, gtk_get_current_event_time());
	gtk_widget_show_all(getWidget("nickMenu"));
}

void Hub::getPassword_gui()
{
	if(!BOOLSETTING(PROMPT_PASSWORD))
	{
		addStatusMessage_gui(_("Waiting for input password (don't remove /password before your password)"), Msg::STATUS, Sound::NONE);

		gint pos = 0;
		GtkWidget *chatEntry = getWidget("chatEntry");
		gtk_editable_delete_text(GTK_EDITABLE(chatEntry), pos, -1);
		gtk_editable_insert_text(GTK_EDITABLE(chatEntry), "/password ", -1, &pos);
		gtk_editable_set_position(GTK_EDITABLE(chatEntry), pos);

		if (!WaitingPassword)
			WaitingPassword = TRUE;
		return;
	}

	if (PasswordDialog)
		return;

	// Create password dialog
	string title = client->getHubUrl(); //_("Enter hub password")
	GtkWidget *dialog = gtk_dialog_new_with_buttons(title.c_str(),
		GTK_WINDOW(WulforManager::get()->getMainWindow()->getContainer()),
		GTK_DIALOG_DESTROY_WITH_PARENT,
		GTK_STOCK_OK,
		GTK_RESPONSE_OK,
		GTK_STOCK_CANCEL,
		GTK_RESPONSE_CANCEL,
		NULL);
	gtk_window_set_modal(GTK_WINDOW(dialog), TRUE);
	gtk_dialog_set_default_response(GTK_DIALOG(dialog), GTK_RESPONSE_OK);

	GtkWidget *box = gtk_vbox_new(TRUE, 0);
	GtkWidget *entry = gtk_entry_new();
	g_object_set(entry, "can-focus", TRUE, "visibility", FALSE, "activates-default", TRUE, NULL);

	gtk_box_pack_start(GTK_BOX(box), entry, FALSE, FALSE, 8);

	GtkWidget *frame = gtk_frame_new(NULL);
	g_object_set(frame, "border-width", 8, NULL);

	GtkWidget *label = gtk_label_new(NULL);
	gtk_label_set_markup(GTK_LABEL(label), _("<b>Enter your password</b>"));
	gtk_frame_set_label_widget(GTK_FRAME(frame), label);

	gtk_container_add(GTK_CONTAINER(frame), box);
	gtk_container_add(GTK_CONTAINER(GTK_DIALOG(dialog)->vbox), frame);

	g_object_set_data(G_OBJECT(dialog), "password-entry", (gpointer) entry);

	g_signal_connect(dialog, "response", G_CALLBACK(onPasswordDialog), (gpointer) this);
	gtk_widget_show_all(dialog);

	PasswordDialog = TRUE;
	WaitingPassword = TRUE;
}

void Hub::onPasswordDialog(GtkWidget *dialog, gint response, gpointer data)
{
	Hub *hub = (Hub *) data;
	GtkWidget *entry = (GtkWidget *) g_object_get_data(G_OBJECT(dialog), "password-entry");

	if (response == GTK_RESPONSE_OK)
	{
		string password = gtk_entry_get_text(GTK_ENTRY(entry));
		typedef Func1<Hub, string> F1;
		F1 *func = new F1(hub, &Hub::setPassword_client, password);
		WulforManager::get()->dispatchClientFunc(func);
	}
	else
		hub->client->disconnect(TRUE);

	gtk_widget_destroy(dialog);
	hub->PasswordDialog = FALSE;
	hub->WaitingPassword = FALSE;
}

void Hub::addStatusMessage_gui(string message, Msg::TypeMsg typemsg, Sound::TypeSound sound)
{
	if (!message.empty())
	{
		if (sound != Sound::NONE)
			Sound::get()->playSound(sound);

		setStatus_gui("statusMain", message);

		if (BOOLSETTING(STATUS_IN_CHAT))
		{
			string line = "*** " + message;
			addMessage_gui("", line, typemsg);
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

void Hub::addMessage_gui(string cid, string message, Msg::TypeMsg typemsg)
{
	PluginManager::getInstance()->onChatDisplay(client, message);

	if (message.empty())
		return;

	GtkTextIter iter;
	string line = "";

	if (BOOLSETTING(TIME_STAMPS))
		line += "[" + Util::getShortTimeString() + "] ";

	line += message + "\n";

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
			tagMsg = Tag::TAG_GENERAL;
	}

	totalEmoticons = 0;

	applyTags_gui(cid, line);

	gtk_text_buffer_get_end_iter(chatBuffer, &iter);

	// Limit size of chat text
	if (gtk_text_buffer_get_line_count(chatBuffer) > maxLines + 1)
	{
		GtkTextIter next;
		gtk_text_buffer_get_start_iter(chatBuffer, &iter);
		gtk_text_buffer_get_iter_at_line(chatBuffer, &next, 1);
		gtk_text_buffer_delete(chatBuffer, &iter, &next);
	}
}
/* Inspired by StrongDC catch code ips */
gboolean Hub::HitIP(string& name, string &sIp)
{
    string re = "^\s*((([0-9A-Fa-f]{1,4}:){7}([0-9A-Fa-f]{1,4}|:))|(([0-9A-Fa-f]{1,4}:){6}(:[0-9A-Fa-f]{1,4}|((25[0-5]|2[0-4][0-9]|1[0-9][0-9]|[1-9]?[0-9])(.(25[0-5]|2[0-4][0-9]|1[0-9][0-9]|[1-9]?[0-9])){3})|:))|(([0-9A-Fa-f]{1,4}:){5}(((:[0-9A-Fa-f]{1,4}){1,2})|:((25[0-5]|2[0-4][0-9]|1[0-9][0-9]|[1-9]?[0-9])(.(25[0-5]|2[0-4][0-9]|1[0-9][0-9]|[1-9]?[0-9])){3})|:))|(([0-9A-Fa-f]{1,4}:){4}(((:[0-9A-Fa-f]{1,4}){1,3})|((:[0-9A-Fa-f]{1,4})?:((25[0-5]|2[0-4][0-9]|1[0-9][0-9]|[1-9]?[0-9])(.(25[0-5]|2[0-4][0-9]|1[0-9][0-9]|[1-9]?[0-9])){3}))|:))|(([0-9A-Fa-f]{1,4}:){3}(((:[0-9A-Fa-f]{1,4}){1,4})|((:[0-9A-Fa-f]{1,4}){0,2}:((25[0-5]|2[0-4][0-9]|1[0-9][0-9]|[1-9]?[0-9])(.(25[0-5]|2[0-4][0-9]|1[0-9][0-9]|[1-9]?[0-9])){3}))|:))|(([0-9A-Fa-f]{1,4}:){2}(((:[0-9A-Fa-f]{1,4}){1,5})|((:[0-9A-Fa-f]{1,4}){0,3}:((25[0-5]|2[0-4][0-9]|1[0-9][0-9]|[1-9]?[0-9])(.(25[0-5]|2[0-4][0-9]|1[0-9][0-9]|[1-9]?[0-9])){3}))|:))|(([0-9A-Fa-f]{1,4}:){1}(((:[0-9A-Fa-f]{1,4}){1,6})|((:[0-9A-Fa-f]{1,4}){0,4}:((25[0-5]|2[0-4][0-9]|1[0-9][0-9]|[1-9]?[0-9])(.(25[0-5]|2[0-4][0-9]|1[0-9][0-9]|[1-9]?[0-9])){3}))|:))|(:(((:[0-9A-Fa-f]{1,4}){1,7})|((:[0-9A-Fa-f]{1,4}){0,5}:((25[0-5]|2[0-4][0-9]|1[0-9][0-9]|[1-9]?[0-9])(.(25[0-5]|2[0-4][0-9]|1[0-9][0-9]|[1-9]?[0-9])){3}))|:)))(%.+)?\s*$";
	bool isOkIpV6 = RegEx::match<string>(name,re);
	if(isOkIpV6)
	{	
		sIp = name;
		return isOkIpV6;
	}
	
	for(uint32_t i = 0;i < name.length(); i++)
	{
		if(!((name[i] == 0) || (name[i] == '.') || ((name[i] >= '0') && (name[i] <= '9')))) {
			return FALSE;
		}
	}

	name += ".";
	size_t begin = 0, pos = string::npos, end = 0;
	bool isOk = true;
	for(int i = 0; i < 4; i++) {
		pos = name.find('.', begin);
		if(pos == string::npos) {
			isOk = false;
			break;
		}
		end = atoi(Text::fromT(name.substr(begin)).c_str());
		if((end < 0) || (end > 255)) {
			isOk = false;
			break;
		}
		begin = pos + 1;
	}

	if(isOk)
	{
		sIp = name.substr(0,pos-1);
	}
	return isOk;

}

void Hub::applyTags_gui(const string cid, const string &line)
{
	GtkTextIter start_iter;
	gtk_text_buffer_get_end_iter(chatBuffer, &start_iter);

	// apply timestamp tag
	if (BOOLSETTING(TIME_STAMPS))
	{
		gtk_text_iter_backward_chars(&start_iter,
			g_utf8_strlen(line.c_str(), -1) - g_utf8_strlen(Util::getShortTimeString().c_str(), -1) - 2);

		GtkTextIter ts_start_iter, ts_end_iter;
		ts_end_iter = start_iter;

		gtk_text_buffer_get_end_iter(chatBuffer, &ts_start_iter);
		gtk_text_iter_backward_chars(&ts_start_iter, g_utf8_strlen(line.c_str(), -1));

		gtk_text_buffer_apply_tag(chatBuffer, TagsMap[Tag::TAG_TIMESTAMP], &ts_start_iter, &ts_end_iter);
	}
	else
		gtk_text_iter_backward_chars(&start_iter, g_utf8_strlen(line.c_str(), -1));

	// apply tags: nick, link, hub-url, magnet
	GtkTextIter tag_start_iter, tag_end_iter;

	gtk_text_buffer_move_mark(chatBuffer, start_mark, &start_iter);
	gtk_text_buffer_move_mark(chatBuffer, end_mark, &start_iter);

	string tagName;
	Tag::TypeTag tagStyle = Tag::TAG_GENERAL;

	bool firstNick = FALSE;
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
			gtk_text_buffer_move_mark(chatBuffer, start_mark, &start_iter);
			gtk_text_buffer_move_mark(chatBuffer, end_mark, &start_iter);

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
		bool isNick = FALSE;
		bool image_tag = FALSE;
		bool bold_tag = FALSE;
		bool italic_tag = FALSE;
		bool underline_tag = FALSE;
		bool countryTag = FALSE;
		string image_magnet, bold_text, italic_text, underline_text, country_text;
		gchar *temp = gtk_text_iter_get_text(&tag_start_iter, &tag_end_iter);

		if(WGETB("use-highlighting"))
		{
			GtkTextTag *tag = gtk_text_tag_table_lookup(gtk_text_buffer_get_tag_table(chatBuffer), temp);
			bool isTab = false;
			if(WulforUtil::isHighlightingWorld(chatBuffer,tag,string(temp),isTab,(gpointer)this, TagsMap))
			{
				gtk_text_buffer_apply_tag(chatBuffer, TagsMap[Tag::TAG_HIGHL], &tag_start_iter, &tag_end_iter);
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
				callback = G_CALLBACK(onNickTagEvent_gui);
				string order = nickView.getString(&iter, "Client Type");

				if (tagName == client->getMyNick())
					tagStyle = Tag::TAG_MYNICK;
				else if (order[0] == 'C')
					tagStyle = Tag::TAG_FAVORITE;
				else if (order[0] == 'B')
					tagStyle = Tag::TAG_OPERATOR;
				else if (order[0] == 'U' || order[0] == 'A')//bot fix (for now)
					tagStyle = Tag::TAG_NICK;

				tagName = tagPrefix + tagName;
			}
			else
			{
			   bool notlink = FALSE;
			   bool country = FALSE;

                if(g_ascii_strncasecmp(tagName.c_str(), "[ccc]", 5) == 0)
                {
                    string::size_type i = tagName.rfind("[/ccc]");
                    if (i != string::npos)
                    {
						country_text = tagName.substr(5, i - 5);
						if(country_text.length() == 2 )
						{
							country = notlink = countryTag = TRUE;

						}
                    }
                }
			// Support bbCode: [i]italic-text[/i], [u]underline-text[/u]
			// [img]magnet-link[/img]
            if(!country) {
				if (g_ascii_strncasecmp(tagName.c_str(), "[img]", 5) == 0)
				{
					string::size_type i = tagName.rfind("[/img]");
					if (i != string::npos)
					{
						image_magnet = tagName.substr(5, i - 5);
						if (WulforUtil::isMagnet(image_magnet))
							notlink = image_tag = TRUE;
					}
				}
				else if (g_ascii_strncasecmp(tagName.c_str(), "[b]", 3) == 0)
				{
					string::size_type i = tagName.rfind("[/b]");
					if (i != string::npos)
					{
						bold_text = tagName.substr(3, i - 3);
						notlink = bold_tag = TRUE;
					}
				}
				else if (g_ascii_strncasecmp(tagName.c_str(), "[i]", 3) == 0)
				{
					string::size_type i = tagName.rfind("[/i]");
					if (i != string::npos)
					{
						italic_text = tagName.substr(3, i - 3);
						notlink = italic_tag = TRUE;
					}
				}
				else if (g_ascii_strncasecmp(tagName.c_str(), "[u]", 3) == 0)
				{
					string::size_type i = tagName.rfind("[/u]");
					if (i != string::npos)
					{
						underline_text = tagName.substr(3, i - 3);
						notlink = underline_tag = TRUE;
					}
				}
            }

				if (!notlink)
				{
					if (WulforUtil::isLink(tagName))
						callback = G_CALLBACK(onLinkTagEvent_gui);
					else if (WulforUtil::isHubURL(tagName))
						callback = G_CALLBACK(onHubTagEvent_gui);
					else if (WulforUtil::isMagnet(tagName))
						callback = G_CALLBACK(onMagnetTagEvent_gui);

					tagStyle = Tag::TAG_URL;
				}

				if(HitIP(tagName,ip))
				{
					callback = G_CALLBACK(onIpTagEvent_gui);
					tagStyle = Tag::TAG_IPADR;
					isIp = true;
					userCommandMenu2->cleanMenu_gui();
					userCommandMenu2->addIp(ip);
					userCommandMenu2->addHub(address);
					userCommandMenu2->buildMenu_gui();
					gtk_widget_show_all(userCommandMenu2->getContainer());
				}
			}
		}

		g_free(temp);

		if(countryTag)
		{
            gtk_text_buffer_move_mark(chatBuffer, tag_mark, &tag_end_iter);
            if(country_text.length() == 2)
            {
                GdkPixbuf *buffer = WulforUtil::LoadCountryPixbuf(country_text);
                if(buffer != NULL)
				{	
				gtk_text_buffer_delete(chatBuffer, &tag_start_iter, &tag_end_iter);
                GtkTextChildAnchor *anchor = gtk_text_buffer_create_child_anchor(chatBuffer, &tag_start_iter);
                GtkWidget *event_box = gtk_event_box_new();
                // Creating a visible window may cause artifacts that are visible to the user.
                gtk_event_box_set_visible_window(GTK_EVENT_BOX(event_box), FALSE);
                GtkWidget *image = gtk_image_new_from_pixbuf(buffer);
                gtk_container_add(GTK_CONTAINER(event_box),image);
                gtk_text_view_add_child_at_anchor(GTK_TEXT_VIEW(getWidget("chatText")), event_box, anchor);
                g_signal_connect(G_OBJECT(image), "expose-event", G_CALLBACK(expose), NULL);

                gtk_widget_show_all(event_box);
                #if GTK_CHECK_VERSION(2, 12, 0)
                    gtk_widget_set_tooltip_text(event_box, country_text.c_str());
                #else
                    gtk_tooltips_set_tip(tips, event_box, country_text.c_str(), country_text.c_str());
                #endif
				
				}
            }

		}

		if (image_tag)
		{
			gtk_text_buffer_move_mark(chatBuffer, tag_mark, &tag_end_iter);
			string name, tth, target;
			int64_t size;

			if (WulforUtil::splitMagnet(image_magnet, name, size, tth))
			{
				gtk_text_buffer_delete(chatBuffer, &tag_start_iter, &tag_end_iter);

				GtkTextChildAnchor *anchor = gtk_text_buffer_create_child_anchor(chatBuffer, &tag_start_iter);
				GtkWidget *event_box = gtk_event_box_new();

				// Creating a visible window may cause artifacts that are visible to the user.
				gtk_event_box_set_visible_window(GTK_EVENT_BOX(event_box), FALSE);

				GtkWidget *image = gtk_image_new_from_stock(GTK_STOCK_FILE, GTK_ICON_SIZE_BUTTON);
				gtk_container_add(GTK_CONTAINER(event_box), image);
				gtk_text_view_add_child_at_anchor(GTK_TEXT_VIEW(getWidget("chatText")), event_box, anchor);
				g_object_set_data_full(G_OBJECT(event_box), "magnet", g_strdup(image_magnet.c_str()), g_free);
				g_object_set_data_full(G_OBJECT(event_box), "cid", g_strdup(cid.c_str()), g_free);
				g_signal_connect(G_OBJECT(image), "expose-event", G_CALLBACK(expose), NULL);
				g_signal_connect(G_OBJECT(event_box), "event", G_CALLBACK(onImageEvent_gui), (gpointer)this);
				gtk_widget_show_all(event_box);
				imageList.insert(ImageList::value_type(image, tth));
				string text = "name: " + name + "\n" + "size: " + Util::formatBytes(size);
#if GTK_CHECK_VERSION(2, 12, 0)
				gtk_widget_set_tooltip_text(event_box, text.c_str());
#else
				gtk_tooltips_set_tip(tips, event_box, text.c_str(), text.c_str());
#endif
				g_signal_connect(G_OBJECT(image), "destroy", G_CALLBACK(onImageDestroy_gui), (gpointer)this);

				if (ImgLimit)
				{
					if (ImgLimit > 0)
						ImgLimit--;

					typedef Func4<Hub, string, int64_t, string, string> F4;
					target = Util::getPath(Util::PATH_USER_CONFIG) + "Images" + PATH_SEPARATOR_STR + tth;
					F4 *func = new F4(this, &Hub::download_client, target, size, tth, cid);
					WulforManager::get()->dispatchClientFunc(func);
				}
			}
		}
		else if (bold_tag)
		{
			dcassert(tagMsg >= Tag::TAG_GENERAL && tagMsg < Tag::TAG_TIMESTAMP);

			gtk_text_buffer_move_mark(chatBuffer, tag_mark, &tag_end_iter);
			gtk_text_buffer_delete(chatBuffer, &tag_start_iter, &tag_end_iter);
			gtk_text_buffer_insert_with_tags(chatBuffer, &tag_start_iter,
				bold_text.c_str(), bold_text.size(), BoldTag, TagsMap[tagMsg], NULL);
		}
		else if (italic_tag)
		{
			dcassert(tagMsg >= Tag::TAG_GENERAL && tagMsg < Tag::TAG_TIMESTAMP);

			gtk_text_buffer_move_mark(chatBuffer, tag_mark, &tag_end_iter);
			gtk_text_buffer_delete(chatBuffer, &tag_start_iter, &tag_end_iter);
			gtk_text_buffer_insert_with_tags(chatBuffer, &tag_start_iter,
				italic_text.c_str(), italic_text.size(), ItalicTag, TagsMap[tagMsg], NULL);
		}
		else if (underline_tag)
		{
			dcassert(tagMsg >= Tag::TAG_GENERAL && tagMsg < Tag::TAG_TIMESTAMP);

			gtk_text_buffer_move_mark(chatBuffer, tag_mark, &tag_end_iter);
			gtk_text_buffer_delete(chatBuffer, &tag_start_iter, &tag_end_iter);
			gtk_text_buffer_insert_with_tags(chatBuffer, &tag_start_iter,
				underline_text.c_str(), underline_text.size(), UnderlineTag, TagsMap[tagMsg], NULL);
		}

		if (image_tag || bold_tag || italic_tag || underline_tag || countryTag)
		{
			applyEmoticons_gui();

			gtk_text_buffer_get_iter_at_mark(chatBuffer, &start_iter, tag_mark);

			if (gtk_text_iter_is_end(&start_iter))
				return;

			start = FALSE;

			continue;
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
			if (callback == G_CALLBACK(onMagnetTagEvent_gui) && WGETB("use-magnet-split"))
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

			start = FALSE;
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
	else if (!Emoticons::get()->useEmoticons_gui())
	{
		if (WGETB("emoticons-use"))
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
	Emot::List &list = Emoticons::get()->getPack_gui();

	/* set start mark */
	gtk_text_buffer_move_mark(chatBuffer, emot_mark, &start_iter);

	for (;;)
	{
		/* get start and end iter positions at marks */
		gtk_text_buffer_get_iter_at_mark(chatBuffer, &start_iter, emot_mark);
		gtk_text_buffer_get_iter_at_mark(chatBuffer, &end_iter, end_mark);

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
						gtk_text_buffer_get_iter_at_mark(chatBuffer, &tmp_end_iter, end_mark);
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
				setStatus_gui("statusMain", _(" *** Emoticons limit"));
				return;
			}

			/* delete text-emoticon and insert pixbuf-emoticon */
			gtk_text_buffer_delete(chatBuffer, &p_start, &p_end);
			gtk_text_buffer_insert_pixbuf(chatBuffer, &p_start, (*p_it)->getPixbuf());

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
	GSList *tagList;
	GtkTextTag *newTag = NULL;

	gdk_window_get_pointer(widget->window, &x, &y, NULL);

	// Check for tags under the cursor, and change mouse cursor appropriately
	gtk_text_view_window_to_buffer_coords(GTK_TEXT_VIEW(widget), GTK_TEXT_WINDOW_WIDGET, x, y, &buf_x, &buf_y);
	gtk_text_view_get_iter_at_location(GTK_TEXT_VIEW(widget), &iter, buf_x, buf_y);
	tagList = gtk_text_iter_get_tags(&iter);

	if (tagList != NULL)
	{
		newTag = GTK_TEXT_TAG(tagList->data);

		if (find(TagsMap + Tag::TAG_MYNICK, TagsMap + Tag::TAG_LAST, newTag) != TagsMap + Tag::TAG_LAST)
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
			selectedTagStr = newTag->name;

			if (find(TagsMap, TagsMap + Tag::TAG_MYNICK, newTag) == TagsMap + Tag::TAG_MYNICK)
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

void Hub::preferences_gui()
{
	WulforSettingsManager *wsm = WulforSettingsManager::getInstance();
	string fore, back;
	int bold, italic;

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

	gtk_widget_queue_draw(getWidget("chatText"));
	gtk_widget_queue_draw(getWidget("nickView"));
	gtk_widget_queue_draw(getWidget("emotButton"));

	if (!WGETB("emoticons-use"))
	{
		if (GTK_WIDGET_IS_SENSITIVE(getWidget("emotButton")))
			gtk_widget_set_sensitive(getWidget("emotButton"), FALSE);
	}
	else if (!GTK_WIDGET_IS_SENSITIVE(getWidget("emotButton")))
	{
		gtk_widget_set_sensitive(getWidget("emotButton"), TRUE);
	}

	// resort users
	string sort = BOOLSETTING(SORT_FAVUSERS_FIRST) ? "Client Type" : "Nick Order";
	nickView.setSortColumn_gui(_("Nick"), sort);
	gtk_tree_sortable_set_sort_column_id(GTK_TREE_SORTABLE(nickStore), nickView.col(sort), GTK_SORT_ASCENDING);

	string strcolor = WGETS("background-color-chat");
	GdkColor color;
	gdk_color_parse(strcolor.c_str(),&color);
	gtk_widget_modify_base(getWidget("chatText"),GTK_STATE_NORMAL,&color);
	gtk_widget_modify_base(getWidget("chatText"),GTK_STATE_PRELIGHT,&color);
	gtk_widget_modify_base(getWidget("chatText"),GTK_STATE_ACTIVE,&color);
	gtk_widget_modify_base(getWidget("chatText"),GTK_STATE_INSENSITIVE,&color);
	setColorsRows();
}

void Hub::getSettingTag_gui(WulforSettingsManager *wsm, Tag::TypeTag type, string &fore, string &back, int &bold, int &italic)
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

		case Tag::TAG_CHEAT:

			fore = wsm->getString("text-cheat-fore-color");
			back = wsm->getString("text-cheat-back-color");
			bold = wsm->getInt("text-cheat-bold");
			italic = wsm->getInt("text-cheat-italic");
		break;

		case Tag::TAG_HIGHL:

			fore = wsm->getString("text-high-fore-color");
			back = wsm->getString("text-high-back-color");
			bold = wsm->getInt("text-high-bold");
			italic = wsm->getInt("text-high-italic");
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

		case Tag::TAG_URL:

			fore = wsm->getString("text-url-fore-color");
			back = wsm->getString("text-url-back-color");
			bold = wsm->getInt("text-url-bold");
			italic = wsm->getInt("text-url-italic");
		break;

		case Tag::TAG_NICK:

			fore = wsm->getString("text-general-fore-color");
			back = wsm->getString("text-general-back-color");
			italic = wsm->getInt("text-general-italic");

			if (wsm->getBool("text-bold-autors"))
				bold = 1;
			else
				bold = 0;
		break;

		case Tag::TAG_GENERAL:

		default:
			fore = wsm->getString("text-general-fore-color");
			back = wsm->getString("text-general-back-color");
			bold = wsm->getInt("text-general-bold");
			italic = wsm->getInt("text-general-italic");
	}
}

GtkTextTag* Hub::createTag_gui(const string &tagname, Tag::TypeTag type)
{
	WulforSettingsManager *wsm = WulforSettingsManager::getInstance();
	GtkTextTag *tag = gtk_text_tag_table_lookup(gtk_text_buffer_get_tag_table(chatBuffer), tagname.c_str());

	if (!tag)
	{
		string fore, back;
		int bold, italic;

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

gboolean Hub::onFocusIn_gui(GtkWidget *widget, GdkEventFocus *event, gpointer data)
{
	Hub *hub = (Hub *)data;

	gtk_widget_grab_focus(hub->getWidget("chatEntry"));

	// fix select text
	gtk_editable_set_position(GTK_EDITABLE(hub->getWidget("chatEntry")), -1);

	return TRUE;
}

gboolean Hub::onNickListButtonPress_gui(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
	Hub *hub = (Hub *)data;

	if (event->type == GDK_BUTTON_PRESS || event->type == GDK_2BUTTON_PRESS)
		hub->oldType = event->type;

	if (event->button == 3)
	{
		GtkTreePath *path;
		if (gtk_tree_view_get_path_at_pos(hub->nickView.get(), (gint)event->x, (gint)event->y, &path, NULL, NULL, NULL))
		{
			bool selected = gtk_tree_selection_path_is_selected(hub->nickSelection, path);
			gtk_tree_path_free(path);

			if (selected)
				return TRUE;
		}
	}

	return FALSE;
}

gboolean Hub::onNickListButtonRelease_gui(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
	Hub *hub = (Hub *)data;

	if (gtk_tree_selection_count_selected_rows(hub->nickSelection) > 0)
	{
		if (event->button == 1 && hub->oldType == GDK_2BUTTON_PRESS)
		{
			if (WGETB("pm"))
				hub->onMsgItemClicked_gui(NULL, data);
			else
				hub->onBrowseItemClicked_gui(NULL, data);
		}
		else if (event->button == 2 && event->type == GDK_BUTTON_RELEASE)
		{
			if (WGETB("pm"))
				hub->onBrowseItemClicked_gui(NULL, data);
			else
				hub->onMsgItemClicked_gui(NULL, data);
		}
		else if (event->button == 3 && event->type == GDK_BUTTON_RELEASE)
		{
			hub->popupNickMenu_gui();
		}
	}

	return FALSE;
}

gboolean Hub::onNickListKeyRelease_gui(GtkWidget *widget, GdkEventKey *event, gpointer data)
{
	Hub *hub = (Hub *)data;

	if (gtk_tree_selection_count_selected_rows(hub->nickSelection) > 0)
	{
		if (event->keyval == GDK_Menu || (event->keyval == GDK_F10 && event->state & GDK_SHIFT_MASK))
		{
			hub->popupNickMenu_gui();
		}
		else if (event->keyval == GDK_Return || event->keyval == GDK_KP_Enter)
		{
			hub->onBrowseItemClicked_gui(NULL, data);
		}
	}

	return FALSE;
}

gboolean Hub::onEntryKeyPress_gui(GtkWidget *entry, GdkEventKey *event, gpointer data)
{
	Hub *hub = (Hub *)data;

	if (event->keyval == GDK_Up || event->keyval == GDK_KP_Up)
	{
		size_t index = hub->historyIndex - 1;
		if (index >= 0 && index < hub->history.size())
		{
			hub->historyIndex = index;
			gtk_entry_set_text(GTK_ENTRY(entry), hub->history[index].c_str());
		}
		return TRUE;
	}
	else if (event->keyval == GDK_Down || event->keyval == GDK_KP_Down)
	{
		size_t index = hub->historyIndex + 1;
		if (index >= 0 && index < hub->history.size())
		{
			hub->historyIndex = index;
			gtk_entry_set_text(GTK_ENTRY(entry), hub->history[index].c_str());
		}
		return TRUE;
	}
	else if (event->keyval == GDK_Tab || event->keyval == GDK_ISO_Left_Tab)
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

gboolean Hub::onNickTagEvent_gui(GtkTextTag *tag, GObject *textView, GdkEvent *event, GtkTextIter *iter, gpointer data)
{
	Hub *hub = (Hub *)data;

	if (event->type == GDK_2BUTTON_PRESS)
	{
		string tagName = tag->name;
		hub->nickToChat_gui(tagName.substr(tagPrefix.size()));

		return TRUE;
	}
	else if (event->type == GDK_BUTTON_PRESS)
	{
		GtkTreeIter nickIter;
		string tagName = tag->name;

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

gboolean Hub::onLinkTagEvent_gui(GtkTextTag *tag, GObject *textView, GdkEvent *event, GtkTextIter *iter, gpointer data)
{
	Hub *hub = (Hub *)data;

	if (event->type == GDK_BUTTON_PRESS)
	{
		switch (event->button.button)
		{
			case 1:
				onOpenLinkClicked_gui(NULL, data);
				break;
			case 3:
				// Popup uri context menu
				gtk_widget_show_all(hub->getWidget("linkMenu"));
				gtk_menu_popup(GTK_MENU(hub->getWidget("linkMenu")), NULL, NULL, NULL, NULL, 0, gtk_get_current_event_time());
				break;
		}
		return TRUE;
	}
	return FALSE;
}

gboolean Hub::onHubTagEvent_gui(GtkTextTag *tag, GObject *textView, GdkEvent *event, GtkTextIter *iter, gpointer data)
{
	Hub *hub = (Hub *)data;

	if (event->type == GDK_BUTTON_PRESS)
	{
		switch (event->button.button)
		{
			case 1:
				onOpenHubClicked_gui(NULL, data);
				break;
			case 3:
				// Popup uri context menu
				gtk_widget_show_all(hub->getWidget("hubMenu"));
				gtk_menu_popup(GTK_MENU(hub->getWidget("hubMenu")), NULL, NULL, NULL, NULL, 0, gtk_get_current_event_time());
				break;
		}
		return TRUE;
	}
	return FALSE;
}

gboolean Hub::onIpTagEvent_gui(GtkTextTag *tag, GObject *textView, GdkEvent *event , GtkTextIter *iter, gpointer data)
{
	Hub *hub = (Hub *)data;
	hub->ip = tag->name; //think

	if(event->type == GDK_BUTTON_PRESS)
	{
		if(event->button.button == 3)
		{
			gtk_widget_show_all(hub->getWidget("ipMenu"));
			gtk_menu_popup(GTK_MENU(hub->getWidget("ipMenu")), NULL,NULL,NULL,NULL, 0,gtk_get_current_event_time());
			return TRUE;
		}
	}
	return FALSE;
}

void Hub::onCopyIpItem_gui(GtkWidget *wid, gpointer data)
{
	Hub *hub = (Hub *)data;
	gtk_clipboard_set_text(gtk_clipboard_get(GDK_SELECTION_CLIPBOARD), hub->ip.c_str(), hub->ip.length());
}

void Hub::onRipeDbItem_gui(GtkWidget *wid, gpointer data)
{
	Hub *hub = (Hub *)data;
	string error;
	WulforUtil::openURI("http://www.db.ripe.net/whois?searchtext=" + hub->ip + "&searchSubmit=search", error);
	hub->setStatus_gui("statusMain",error);
}

gboolean Hub::onMagnetTagEvent_gui(GtkTextTag *tag, GObject *textView, GdkEvent *event, GtkTextIter *iter, gpointer data)
{
	Hub *hub = (Hub *)data;

	if (event->type == GDK_BUTTON_PRESS)
	{
		switch (event->button.button)
		{
			case 1:
				// Search for magnet
				WulforManager::get()->getMainWindow()->actionMagnet_gui(hub->selectedTagStr);
				break;
			case 3:
				// Popup magnet context menu
				gtk_widget_show_all(hub->getWidget("magnetMenu"));
				gtk_menu_popup(GTK_MENU(hub->getWidget("magnetMenu")), NULL, NULL, NULL, NULL, 0, gtk_get_current_event_time());
				break;
		}
		return TRUE;
	}
	return FALSE;
}

gboolean Hub::onChatPointerMoved_gui(GtkWidget *widget, GdkEventMotion *event, gpointer data)
{
	Hub *hub = (Hub *)data;

	hub->updateCursor_gui(widget);

	return FALSE;
}

gboolean Hub::onChatVisibilityChanged_gui(GtkWidget *widget, GdkEventVisibility *event, gpointer data)
{
	Hub *hub = (Hub *)data;

	hub->updateCursor_gui(widget);

	return FALSE;
}

gboolean Hub::onEmotButtonRelease_gui(GtkWidget *widget, GdkEventButton *event, gpointer data)
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
			gtk_menu_popup(GTK_MENU(emot_menu), NULL, NULL, NULL, NULL, 0, gtk_get_current_event_time());
		break;
	}

	return FALSE;
}

void Hub::onChatScroll_gui(GtkAdjustment *adjustment, gpointer data)
{
	Hub *hub = (Hub *)data;
	gdouble value = gtk_adjustment_get_value(adjustment);
	hub->scrollToBottom = value >= (adjustment->upper - adjustment->page_size);
}

void Hub::onChatResize_gui(GtkAdjustment *adjustment, gpointer data)
{
	Hub *hub = (Hub *)data;
	gdouble value = gtk_adjustment_get_value(adjustment);

	if (hub->scrollToBottom && value < (adjustment->upper - adjustment->page_size))
	{
		GtkTextIter iter;

		gtk_text_buffer_get_end_iter(hub->chatBuffer, &iter);
		gtk_text_buffer_move_mark(hub->chatBuffer, hub->chatMark, &iter);
		gtk_text_view_scroll_to_mark(GTK_TEXT_VIEW(hub->getWidget("chatText")), hub->chatMark, 0, FALSE, 0, 0);
	}
}

void Hub::onSendMessage_gui(GtkEntry *entry, gpointer data)
{
	string text = gtk_entry_get_text(entry);
	if (text.empty())
		return;

	gtk_entry_set_text(entry, "");
	Hub *hub = (Hub *)data;
	typedef Func1<Hub, string> F1;
	F1 *func;
	typedef Func2<Hub, string, bool> F2;
	F2 *func2;

	// Store line in chat history
	hub->history.pop_back();
	hub->history.push_back(text);
	hub->history.push_back("");
	hub->historyIndex = hub->history.size() - 1;
	if (hub->history.size() > maxHistory + 1)
		hub->history.erase(hub->history.begin());

	#ifdef _USELUA
    bool dropMessage = hub->client->onHubFrameEnter(hub->client, Text::fromT(text));
	#endif

	// Process special commands
	if (text[0] == '/')
	{
		string command = text, param ,mess = Util::emptyString, status = Util::emptyString, params;
		bool thirdPerson = false;
		string::size_type separator = text.find_first_of(' ');
		if (separator != string::npos && text.size() > separator + 1)
		{
			params = text.substr(separator + 1);
		}

	   if(PluginManager::getInstance()->onChatCommand(hub->client, command )) {
			// Plugins, chat commands
		  return;
	    }

        if(WulforUtil::checkCommand(command, param, mess, status, thirdPerson))
        {
            if(!mess.empty())
				hub->sendMessage_client(mess, thirdPerson);

			if(!status.empty())
				hub->addStatusMessage_gui(status, Msg::SYSTEM, Sound::NONE);
        }
		else if (command == "clear")
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
			WulforManager::get()->dispatchClientFunc(new Func0<Hub>(hub, &Hub::addAsFavorite_client));
		}
		else if (command == "fuser" || command == "fu")
		{
			if (hub->client->getMyNick() == params)
				return;

			if (hub->userMap.find(param) != hub->userMap.end())
			{
				string &cid = hub->userMap[params];
				if (hub->userFavoriteMap.find(cid) == hub->userFavoriteMap.end())
				{
					Func1<Hub, string> *func = new Func1<Hub, string>(hub, &Hub::addFavoriteUser_client, cid);
					WulforManager::get()->dispatchClientFunc(func);
				} else
					hub->addStatusMessage_gui(param + _(" is favorite user"), Msg::STATUS, Sound::NONE);
			} else
				hub->addStatusMessage_gui(_("Not found user: ") + params, Msg::SYSTEM, Sound::NONE);
		}
		else if (command == "removefu" || command == "rmfu")
		{
			if (hub->client->getMyNick() == params)
				return;

			UserMap::const_iterator it = find_if(hub->userFavoriteMap.begin(), hub->userFavoriteMap.end(),
				CompareSecond<string, string>(params));

			if (it != hub->userFavoriteMap.end())
			{
				Func1<Hub, string> *func = new Func1<Hub, string>(hub, &Hub::removeFavoriteUser_client, it->first);
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
			hub->addMessage_gui("", _("User favorite list:") + (list.empty()? list = _(" empty...") : list), Msg::SYSTEM);
		}
		else if (command == "getlist")
		{
			if (hub->userMap.find(params) != hub->userMap.end())
			{
				func2 = new F2(hub, &Hub::getFileList_client, hub->userMap[params], FALSE);
				WulforManager::get()->dispatchClientFunc(func2);
			}
			else
				hub->addStatusMessage_gui(_("Not found user: ") + param, Msg::SYSTEM, Sound::NONE);
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
		else if (command == "emoticons" || command == "emot")
		{
			if (hub->useEmoticons)
			{
				hub->useEmoticons = FALSE;
				hub->addStatusMessage_gui(_("Emoticons mode off"), Msg::SYSTEM, Sound::NONE);
			}
			else
			{
				hub->useEmoticons = TRUE;
				hub->addStatusMessage_gui(_("Emoticons mode on"), Msg::SYSTEM, Sound::NONE);
			}
		}
		else if (command == "limitimg" || command == "limg")
		{
			int n;
			string text;
			if (params.empty())
			{
				n = hub->ImgLimit;
				if (n == 0)
					text = _("Download image: disable");
				else if (n < 0)
					text = _("Download image: unlimit");
				else
					text = _("Download limit image: ") + Util::toString(n);
				hub->addMessage_gui("", text.c_str(), Msg::SYSTEM);
				return;
			}
			n = Util::toInt(params);
			hub->ImgLimit = n;
			text = _("Set download limit image: ") + Util::toString(n);
			hub->addStatusMessage_gui(text, Msg::SYSTEM, Sound::NONE);
		}
		else if (command == "help")
		{
			hub->addMessage_gui("", string(_("*** Available commands:")) + "\n\n" +
			"/clear\t\t\t\t - " + _("Clear chat") + "\n" +
			"/close\t\t\t\t - " + _("Close chat") + "\n" +
			"/favorite, /fav\t\t\t - " + _("Add a hub to favorites") + "\n" +
			"/fuser, /fu <nick>\t\t - " + _("Add user to favorites list") + "\n" +
			"/removefu, /rmfu <nick>\t - " + _("Remove user favorite") + "\n" +
			"/listfu, /lsfu\t\t\t - " + _("Show favorites list") + "\n" +
			"/getlist <nick>\t\t\t - " + _("Get file list") + "\n" +
			"/grant <nick>\t\t\t - " + _("Grant extra slot") + "\n" +
			"/help\t\t\t\t - " + _("Show help") + "\n" +
			"/join <address>\t\t - " + _("Connect to the hub") + "\n" +
			"/me <message>\t\t - " + _("Say a third person") + "\n" +
			"/pm <nick>\t\t\t - " + _("Private message") + "\n" +
			#ifdef _USELUA
			"/luafile <file>\t\t - " 	+ _("Load Lua file") + "\n" +
			"/lua <chunk>\t\t\t -  " 	+ _("Execute Lua Chunk") + "\n" +
			#endif
			"/exec <cmd>-\t\t\t    "    + _("Execute Bash chunk") + "\n"+
			"/userlist\t\t\t\t - " + _("User list show/hide") + "\n" +
			"/limitimg <n>, limg <n>\t - " + _("Download limit image: 0 - disable, n < 0 - unlimit, empty - info") + "\n" +
			"/emoticons, /emot\t\t - " + _("Emoticons on/off") + "\n" +
			"/sc\t\t\t" + _("Start/Stop checkers") + "\n" +
			"/scmyinfo\t\t\t" + _("Start/Stop checkers myinfo") + "\n" +
			"/showjoins\t\t\t" + _("Join/Parts on/off") + "\n" +
			"/showfavjoins\t\t\t" + _("Fav Joins/Parts on/off") + "\n" +
			"/plgadd\t\t\t" + _("Add Plugin") + "\n" +
			"/plist\t\t\t" + _("List Plugins") + "\n" +
			"/addfavorite\t\t\t" + _("Add Idepent Fav") + "\n" +
			"/topic\t\t\t" + _("Show topic") + "\n" +
             WulforUtil::commands
             , Msg::SYSTEM);
		}
		else if(command == "plgadd")
		{
			size_t idx = PluginManager::getInstance()->getPluginList().size();
			if(PluginManager::getInstance()->loadPlugin(Text::fromT(param), true)) {
				const MetaData& info = PluginManager::getInstance()->getPlugin(idx)->getInfo();
			hub->addMessage_gui("",string("Done, Info **\nName")+string(info.name)+string("\nDesc")+string(info.description)+string("\nVersion")+Util::toString(info.version)+string("\n"),Msg::SYSTEM);
			}
		}
		else if(command == "plist") {
			size_t idx = 0;
			string status = string(_("Loaded plugins:")) + _("\n");
			const PluginManager::pluginList& list = PluginManager::getInstance()->getPluginList();
			for(PluginManager::pluginList::const_iterator i = list.begin(); i != list.end(); ++i, ++idx) {
				const MetaData& info = (*i)->getInfo();
			status += Util::toString(idx) + " - " + string(info.name) + " - " + Util::toString(info.version) + "\n";
			}
			hub->addMessage_gui("",status,Msg::SYSTEM);
		}
		else if (command == "join" && !param.empty())
		{
			if (BOOLSETTING(JOIN_OPEN_NEW_WINDOW))
			{
				// Assumption: new hub is same encoding as current hub.
				WulforManager::get()->getMainWindow()->showHub_gui(param, hub->encoding);
			}
			else
			{
				typedef Func2<Hub, string, bool> F2;
				F2 *func = new F2(hub, &Hub::redirect_client, param, TRUE);
				WulforManager::get()->dispatchClientFunc(func);
			}
		}
		else if (command == "me")
		{
			func2 = new F2(hub, &Hub::sendMessage_client, params, true);
			WulforManager::get()->dispatchClientFunc(func2);
		}
		else if (command == "topic")
		{
			hub->addMessage_gui("",_("Topic: ")+hub->client->getHubDescription(), Msg::SYSTEM);
		}
		else if (command == "pm")
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
				hub->addStatusMessage_gui(_("Not found user: ") + param, Msg::SYSTEM, Sound::NONE);
		}
		else if (command == "userlist")
		{
			if (GTK_WIDGET_VISIBLE(hub->getWidget("scrolledwindow2")))
				gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(hub->getWidget("userListCheckButton")), FALSE);
			else
				gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(hub->getWidget("userListCheckButton")), TRUE);
		}
		#ifdef _USELUA
		else if (command == "lua" ) {
			ScriptManager::getInstance()->EvaluateChunk(Text::fromT(params));
		}
		else if( command == "luafile") {
			ScriptManager::getInstance()->EvaluateFile(Text::fromT(params));
		}
		#endif
		///
		else if (command == "exec")
		{
			FILE *pipe = popen( param.c_str(), "r" );
			gchar *command_res;
			gsize command_length;
			GIOChannel* gio_chanel = g_io_channel_unix_new( fileno( pipe ) );
			GIOStatus gio_status = g_io_channel_read_to_end( gio_chanel, &command_res, &command_length, NULL );
			if( gio_status == G_IO_STATUS_NORMAL )
			{
				F2 *func = new F2( hub, &Hub::sendMessage_client, string(command_res), false );
				WulforManager::get()->dispatchClientFunc(func);
			}
			g_io_channel_close( gio_chanel );
			g_free( command_res );
			pclose( pipe );
		}
		else if (command == "sc")
		{
			hub->addStatusMessage_gui(hub->client->startCheck(params),Msg::SYSTEM,Sound::NONE);
		}
		else if(command == "addfavorite")
		{
			FavoriteManager::getInstance()->addFavoriteIUser(params);
		}	
		else if (command == "scmyinfo")
		{
			if(params == "stop") {
				hub->client->stopMyInfoCheck();
				hub->addStatusMessage_gui("Stop MyInfo check", Msg::SYSTEM, Sound::NONE);
			} else	{
				hub->client->startMyInfoCheck();
				hub->addStatusMessage_gui("Started MyInfo check", Msg::SYSTEM, Sound::NONE);
			}
		} else if ( command == "showjoins")
		{
			hub->client->settings.showJoins = !hub->client->settings.showJoins;
            if(hub->client->settings.showJoins) {
                 hub->addStatusMessage_gui("Join/part showing on", Msg::SYSTEM, Sound::NONE);
            } else {
                 hub->addStatusMessage_gui("Join/part showing off", Msg::SYSTEM, Sound::NONE);
            }		
		}
		else if ( command == "showfavjoins")
		{
			hub->client->settings.favShowJoins = !hub->client->settings.favShowJoins;
            if(hub->client->settings.favShowJoins) {
                 hub->addStatusMessage_gui("Join/part for Fav showing on", Msg::SYSTEM, Sound::NONE);
            } else {
                 hub->addStatusMessage_gui("Join/part for fav showing off", Msg::SYSTEM, Sound::NONE);
            }		
		}
		// protect command
		else if (command == "password")
		{
			if (!hub->WaitingPassword)
				return;

			F1 *func = new F1(hub, &Hub::setPassword_client, params);
			WulforManager::get()->dispatchClientFunc(func);
			hub->WaitingPassword = FALSE;
		}
		else
		{
			#ifdef _USELUA
				if(!dropMessage) {

					if (BOOLSETTING(SEND_UNKNOWN_COMMANDS))
					{
						func2 = new F2(hub, &Hub::sendMessage_client, text, false);
						WulforManager::get()->dispatchClientFunc(func2);
					}
					else {
			#endif
					hub->addStatusMessage_gui(_("Unknown command '") + text + _("': type /help for a list of available commands"), Msg::SYSTEM, Sound::NONE);
			#ifdef _USELUA
				}
			}
			#endif
		}

	}
	else
	{
		#ifdef _USELUA
		if(!dropMessage)
         {
		#endif
            func2 = new F2(hub, &Hub::sendMessage_client, text, false);
            WulforManager::get()->dispatchClientFunc(func2);
        #ifdef _USELUA
         }
        #endif
	}
}

void Hub::onNickToChat_gui(GtkMenuItem *item, gpointer data)
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

void Hub::onCopyNickItemClicked_gui(GtkMenuItem *item, gpointer data)
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
			{
				nicks += hub->nickView.getString(&iter, _("Nick")) + ' ';
			}
			gtk_tree_path_free(path);
		}
		g_list_free(list);

		if (!nicks.empty())
		{
			nicks.erase(nicks.length() - 1);
			gtk_clipboard_set_text(gtk_clipboard_get(GDK_SELECTION_CLIPBOARD), nicks.c_str(), nicks.length());
		}
	}
}

void Hub::onBrowseItemClicked_gui(GtkMenuItem *item, gpointer data)
{
	Hub *hub = (Hub *)data;

	if (gtk_tree_selection_count_selected_rows(hub->nickSelection) > 0)
	{
		string cid;
		GtkTreeIter iter;
		GtkTreePath *path;
		typedef Func2<Hub, string, bool> F2;
		F2 *func;
		GList *list = gtk_tree_selection_get_selected_rows(hub->nickSelection, NULL);

		for (GList *i = list; i; i = i->next)
		{
			path = (GtkTreePath *)i->data;
			if (gtk_tree_model_get_iter(GTK_TREE_MODEL(hub->nickStore), &iter, path))
			{
				cid = hub->nickView.getString(&iter, "CID");
				func = new F2(hub, &Hub::getFileList_client, cid, FALSE);
				WulforManager::get()->dispatchClientFunc(func);
			}
			gtk_tree_path_free(path);
		}
		g_list_free(list);
	}
}

void Hub::onMatchItemClicked_gui(GtkMenuItem *item, gpointer data)
{
	Hub *hub = (Hub *)data;

	if (gtk_tree_selection_count_selected_rows(hub->nickSelection) > 0)
	{
		string cid;
		GtkTreeIter iter;
		GtkTreePath *path;
		typedef Func2<Hub, string, bool> F2;
		F2 *func;
		GList *list = gtk_tree_selection_get_selected_rows(hub->nickSelection, NULL);

		for (GList *i = list; i; i = i->next)
		{
			path = (GtkTreePath *)i->data;
			if (gtk_tree_model_get_iter(GTK_TREE_MODEL(hub->nickStore), &iter, path))
			{
				cid = hub->nickView.getString(&iter, "CID");
				func = new F2(hub, &Hub::getFileList_client, cid, TRUE);
				WulforManager::get()->dispatchClientFunc(func);
			}
			gtk_tree_path_free(path);
		}
		g_list_free(list);
	}
}

void Hub::onMsgItemClicked_gui(GtkMenuItem *item, gpointer data)
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

void Hub::onGrantItemClicked_gui(GtkMenuItem *item, gpointer data)
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

void Hub::onRemoveUserItemClicked_gui(GtkMenuItem *item, gpointer data)
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

void Hub::onCopyURIClicked_gui(GtkMenuItem *item, gpointer data)
{
	Hub *hub = (Hub *)data;

	gtk_clipboard_set_text(gtk_clipboard_get(GDK_SELECTION_CLIPBOARD), hub->selectedTagStr.c_str(), hub->selectedTagStr.length());
}

void Hub::onOpenLinkClicked_gui(GtkMenuItem *item, gpointer data)
{
	Hub *hub = (Hub *)data;
	string error = Util::emptyString;
	WulforUtil::openURI(hub->selectedTagStr,error);
	if(!error.empty())
		hub->setStatus_gui("statusMain", error);
}

void Hub::onOpenHubClicked_gui(GtkMenuItem *item, gpointer data)
{
	Hub *hub = (Hub *)data;

	WulforManager::get()->getMainWindow()->showHub_gui(hub->selectedTagStr);
}

void Hub::onSearchMagnetClicked_gui(GtkMenuItem *item, gpointer data)
{
	Hub *hub = (Hub *)data;

	WulforManager::get()->getMainWindow()->addSearch_gui(hub->selectedTagStr);
}

void Hub::onDownloadClicked_gui(GtkMenuItem *item, gpointer data)
{
	Hub *hub = (Hub *)data;
	WulforManager::get()->getMainWindow()->fileToDownload_gui(hub->selectedTagStr, SETTING(DOWNLOAD_DIRECTORY));
}

gboolean Hub::onChatCommandButtonRelease_gui(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
	Hub *hub = (Hub *)data;

	if (event->button == 1)
	{
		gtk_menu_popup(GTK_MENU(hub->getWidget("chatCommandsMenu")), NULL, NULL, NULL, NULL, 0, gtk_get_current_event_time());
	}

	return FALSE;
}

void Hub::onCommandClicked_gui(GtkWidget *widget, gpointer data)
{
	Hub *hub = (Hub *)data;

	string command = (gchar*) g_object_get_data(G_OBJECT(widget), "command");

	gint pos = 0;
	GtkWidget *chatEntry = hub->getWidget("chatEntry");
	if (!gtk_widget_is_focus(chatEntry))
		gtk_widget_grab_focus(chatEntry);
	gtk_editable_delete_text(GTK_EDITABLE(chatEntry), pos, -1);
	gtk_editable_insert_text(GTK_EDITABLE(chatEntry), command.c_str(), -1, &pos);
	gtk_editable_set_position(GTK_EDITABLE(chatEntry), pos);
}

void Hub::onUseEmoticons_gui(GtkWidget *widget, gpointer data)
{
	Hub *hub = (Hub *)data;

	hub->useEmoticons = !hub->useEmoticons;
}

void Hub::onDownloadToClicked_gui(GtkMenuItem *item, gpointer data)
{
	Hub *hub = (Hub *)data;

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

			WulforManager::get()->getMainWindow()->fileToDownload_gui(hub->selectedTagStr, path);
		}
	}
	gtk_widget_hide(dialog);
}

void Hub::onMagnetPropertiesClicked_gui(GtkMenuItem *item, gpointer data)
{
	Hub *hub = (Hub *)data;

	WulforManager::get()->getMainWindow()->propertiesMagnetDialog_gui(hub->selectedTagStr);
}

void Hub::onUserListToggled_gui(GtkWidget *widget, gpointer data)
{
	Hub *hub = (Hub *)data;

	if (GTK_WIDGET_VISIBLE(hub->getWidget("scrolledwindow2")))
		gtk_widget_hide(hub->getWidget("scrolledwindow2"));
	else
		gtk_widget_show_all(hub->getWidget("scrolledwindow2"));
}

void Hub::onAddFavoriteUserClicked_gui(GtkMenuItem *item, gpointer data)
{
	Hub *hub = (Hub *)data;

	if (gtk_tree_selection_count_selected_rows(hub->nickSelection) > 0)
	{
		string cid, nick, order;
		GtkTreeIter iter;
		GtkTreePath *path;
		typedef Func1<Hub, string> F1;
		GList *list = gtk_tree_selection_get_selected_rows(hub->nickSelection, NULL);

		for (GList *i = list; i; i = i->next)
		{
			path = (GtkTreePath *)i->data;

			if (gtk_tree_model_get_iter(GTK_TREE_MODEL(hub->nickStore), &iter, path))
			{
				cid = hub->nickView.getString(&iter, "CID");
				nick = hub->nickView.getString(&iter, _("Nick"));
				order = hub->nickView.getString(&iter, "Client Type");//F

				if (!cid.empty() && nick != hub->client->getMyNick())
				{
					if (!(order[0] == 'C'))
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

void Hub::onRemoveFavoriteUserClicked_gui(GtkMenuItem *item, gpointer data)
{
	Hub *hub = (Hub *)data;

	if (gtk_tree_selection_count_selected_rows(hub->nickSelection) > 0)
	{
		string cid, nick, order;
		GtkTreeIter iter;
		GtkTreePath *path;
		typedef Func1<Hub, string> F1;
		GList *list = gtk_tree_selection_get_selected_rows(hub->nickSelection, NULL);

		for (GList *i = list; i; i = i->next)
		{
			path = (GtkTreePath *)i->data;

			if (gtk_tree_model_get_iter(GTK_TREE_MODEL(hub->nickStore), &iter, path))
			{
				cid = hub->nickView.getString(&iter, "CID");
				nick = hub->nickView.getString(&iter, _("Nick"));
				order = hub->nickView.getString(&iter, "Client Type");//F

				if (!cid.empty() && nick != hub->client->getMyNick())
				{
					if (order[0] == 'C')//f
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

void Hub::onAddIgnoreUserItemClicked_gui(GtkMenuItem *item, gpointer data)
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
				UserPtr user = ClientManager::getInstance()->findUser(CID(cid));

				if (user)
				{
					FavoriteManager::getInstance()->addIgnoredUser(user);
				}
				else
				{
					string message = _("User Ignored ");
					message += WulforUtil::getNicks(user, Util::emptyString);
					hub->addStatusMessage_gui(message, Msg::SYSTEM, Sound::NONE);
				}
			}
		  gtk_tree_path_free(path);
		  }
		g_list_free(list);
    }
}

void Hub::onRemoveIgnoreUserItemClicked_gui(GtkMenuItem *item, gpointer data)
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
				UserPtr user = ClientManager::getInstance()->findUser(CID(cid));

				if (user)
				{
					if(FavoriteManager::getInstance()->isIgnoredUser(user))
					{
						FavoriteManager::getInstance()->removeIgnoredUser(user);
					}
				}
				else
				{
					string message = _("User unIgnored ");
					message += WulforUtil::getNicks(user, Util::emptyString);
					hub->addStatusMessage_gui(message, Msg::SYSTEM, Sound::NONE);

				}
			}
		  gtk_tree_path_free(path);
		  }
		g_list_free(list);
    }
}

//Protect Users
void Hub::onProtectUserClicked_gui(GtkMenuItem *item , gpointer data)
{
	Hub *hub =(Hub *)data;
	if (gtk_tree_selection_count_selected_rows(hub->nickSelection) == 1)
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
			OnlineUser *ou = ClientManager::getInstance()->findOnlineUser(CID(cid), Util::emptyString, false);
			if(ou->getUser() && !ou->getUser()->isSet(User::PROTECT))
				const_cast<UserPtr&>(ou->getUser())->setFlag(User::PROTECT);
			ParamMap params;
			hub->getParams_client(params, ou->getIdentity());
			hub->addProtected_gui(params);
		}
	}
}

void Hub::onUnProtectUserClicked_gui(GtkMenuItem *item , gpointer data)
{
	Hub *hub =(Hub *)data;
	if (gtk_tree_selection_count_selected_rows(hub->nickSelection) == 1)
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
			OnlineUser *ou = ClientManager::getInstance()->findOnlineUser(CID(cid), Util::emptyString, false);
			if(ou->getUser() && !ou->getUser()->isSet(User::PROTECT))
				const_cast<UserPtr&>(ou->getUser())->unsetFlag(User::PROTECT);
			ParamMap params;
			hub->getParams_client(params, ou->getIdentity());
		}
	}
}

void Hub::onShowReportClicked_gui(GtkMenuItem *item , gpointer data)
{
    Hub *hub = (Hub *)data;
	string cid;

	if (gtk_tree_selection_count_selected_rows(hub->nickSelection) == 1)
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
			}
			gtk_tree_path_free(path);
		}
		g_list_free(list);

		OnlineUser *ou = ClientManager::getInstance()->findOnlineUser(CID(cid), hub->client->getHubUrl(), false);
		Identity id = ou->getIdentity();

		hub->addMessage_gui("", WulforUtil::formatReport(id), Msg::CHEAT);

	}
}
//Test SUR
void Hub::onTestSURItemClicked_gui(GtkMenuItem *item, gpointer data)
{
	Hub *hub = (Hub *)data;


	if (gtk_tree_selection_count_selected_rows(hub->nickSelection) >= 1)
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
		    string nick = Util::emptyString;
			nick += nicks.front();
			OnlineUser *ou = ClientManager::getInstance()->findOnlineUser( CID(nick),hub->address,false);
			if(ou != NULL)
			{
				try {
					HintedUser hintedUser(ou->getUser(), hub->address);
					ClientManager::getInstance()->addCheckToQueue(hintedUser, false);
				}catch(...)
				{ }
			}
			nicks.pop();
		}
	}
}
//check FL
void Hub::onCheckFLItemClicked_gui(GtkMenuItem *item , gpointer data)
{
	Hub *hub = (Hub *)data;

	if (gtk_tree_selection_count_selected_rows(hub->nickSelection) == 1)
	{
		string nick;
		GtkTreeIter iter;
		GtkTreePath *path;
		GList *list = gtk_tree_selection_get_selected_rows(hub->nickSelection, NULL);

		for (GList *i = list; i; i = i->next)
		{
			path = (GtkTreePath *)i->data;
			if (gtk_tree_model_get_iter(GTK_TREE_MODEL(hub->nickStore), &iter, path))
			{
				nick = hub->nickView.getString(&iter, _("CID"));
			}
			gtk_tree_path_free(path);
		}
		g_list_free(list);

		if (!nick.empty())
		{
			OnlineUser *ou = ClientManager::getInstance()->findOnlineUser(CID(nick),hub->address,false);
			if(ou != NULL)
			{
				try {
					 HintedUser hintedUser(ou->getUser(), hub->address);
					 ClientManager::getInstance()->addCheckToQueue(hintedUser, true);
				}
				 catch(...)
				{ }
			}
		}
	}
}

void Hub::onRefreshUserListClicked_gui(GtkWidget *wid, gpointer data)
{
	Hub *hub = (Hub *)data;
	hub->clearNickList_gui();
	hub->client->refreshuserlist(true);
}

void Hub::addFavoriteUser_gui(ParamMap params)
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
				nickView.col("Client Type"), params["Type"].c_str(),
				nickView.col("NickColor"), WGETS("userlist-text-favorite").c_str(),
				-1);
			removeTag_gui(nick);
		}

		string message = nick + _(" added to favorites list");
		addStatusMessage_gui(message, Msg::STATUS, Sound::NONE);
		WulforManager::get()->getMainWindow()->addPrivateStatusMessage_gui(Msg::STATUS, cid, message);
	
	}
}

void Hub::removeFavoriteUser_gui(ParamMap params)
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

void Hub::addPasive_gui(ParamMap params)
{
	const string &cid = params["CID"];

	if (userPasiveMap.find(cid) == userPasiveMap.end())
	{
		GtkTreeIter iter;
		const string &nick = params["Nick"];
		userPasiveMap.insert(UserMap::value_type(cid, nick));

		// resort users
		if (findUser_gui(cid, &iter))
		{
			gtk_list_store_set(nickStore, &iter,
                    nickView.col("NickColor"), WGETS("userlist-text-pasive").c_str(),
                    nickView.col("Client Type"), params["Type"].c_str(),
				-1);
			removeTag_gui(nick);
		}
	}
}

void Hub::addOperator_gui(ParamMap params)
{
	const string &cid = params["CID"];

	if (userOpMap.find(cid) == userOpMap.end())
	{
		GtkTreeIter iter;
		const string &nick = params["Nick"];
		userOpMap.insert(UserMap::value_type(cid, nick));

		// resort users
		if (findUser_gui(cid, &iter))
		{
			gtk_list_store_set(nickStore, &iter,
                    nickView.col("NickColor"), WGETS("userlist-text-operator").c_str(),
                    nickView.col("Client Type"), params["Type"].c_str(),
				-1);
			removeTag_gui(nick);
		}
	}
}

void Hub::addProtected_gui(ParamMap params)
{
	const string &cid = params["CID"];

	if (userProtectMap.find(cid) == userProtectMap.end())
	{
		GtkTreeIter iter;
		const string &nick = params["Nick"];
		userProtectMap.insert(UserMap::value_type(cid, nick));

		// resort users
		if (findUser_gui(cid, &iter))
		{
			gtk_list_store_set(nickStore, &iter,
                    nickView.col("NickColor"), WGETS("userlist-text-protected").c_str(),
                    nickView.col("Client Type"), params["Type"].c_str(),
				-1);
			removeTag_gui(nick);
		}
	}
}

void Hub::addIgnore_gui(ParamMap params)
{
	const string &cid = params["CID"];

	if (userIgnoreMap.find(cid) == userIgnoreMap.end())
	{
		GtkTreeIter iter;
		const string &nick = params["Nick"];
		userIgnoreMap.insert(UserMap::value_type(cid, nick));

		// resort users
		if (findUser_gui(cid, &iter))
		{
			gtk_list_store_set(nickStore, &iter,
                    nickView.col("NickColor"), WGETS("userlist-text-ignored").c_str(),
                    nickView.col("Client Type"), params["Type"].c_str(),
				-1);
			removeTag_gui(nick);
		}
	}
}

void Hub::removeIgnore_gui(ParamMap params)
{
    const string &cid = params["CID"];
    if(userIgnoreMap.find(cid)!= userIgnoreMap.end())
    {
       GtkTreeIter iter;
        if(findUser_gui(cid,&iter))
        {
            userIgnoreMap.erase(cid);
            gtk_list_store_set(nickStore,&iter,
                               nickView.col("NickColor"), WGETS("userlist-text-normal").c_str(),
                               nickView.col("Client Type"), ("U"+params["nick"]).c_str(),
                               -1);
        }
    }
	
}
void Hub::addPrivateMessage_gui(Msg::TypeMsg typemsg, string CID, string cid, string url, string message, bool useSetting)
{
	if (userFavoriteMap.find(CID) != userFavoriteMap.end())
		typemsg = Msg::FAVORITE;

	WulforManager::get()->getMainWindow()->addPrivateMessage_gui(typemsg, cid, url, message, useSetting);
}

void Hub::addFavoriteUser_client(const string cid)
{
	UserPtr user = ClientManager::getInstance()->findUser(CID(cid));

	if (user)
	{
		FavoriteManager::getInstance()->addFavoriteUser(user);
	}
}

void Hub::removeFavoriteUser_client(const string cid)
{
	UserPtr user = ClientManager::getInstance()->findUser(CID(cid));

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

void Hub::disconnect_client()
{
	if (client)
	{
		FavoriteManager::getInstance()->removeListener(this);
		QueueManager::getInstance()->removeListener(this);
		client->removeListener(this);
		client->disconnect(TRUE);
		ClientManager::getInstance()->putClient(client);
		client = NULL;
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

void Hub::getFileList_client(string cid, bool match)
{
	string message;

	if (!cid.empty())
	{
		try
		{
			UserPtr user = ClientManager::getInstance()->findUser(CID(cid));
			if (user)
			{
				const HintedUser hintedUser(user, client->getHubUrl());//NOTE: core 0.762

				if (user == ClientManager::getInstance()->getMe())
				{
					// Don't download file list, open locally instead
					WulforManager::get()->getMainWindow()->openOwnList_client(TRUE);
				}
				else if (match)
				{
					QueueManager::getInstance()->addList(hintedUser, QueueItem::FLAG_MATCH_QUEUE);//NOTE: core 0.762
				}
				else
				{
					//QueueManager::getInstance()->addList(hintedUser, QueueItem::FLAG_CLIENT_VIEW | QueueItem::FLAG_PARTIAL_LIST);
					QueueManager::getInstance()->addList(hintedUser, QueueItem::FLAG_CLIENT_VIEW);//NOTE: core 0.762
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
		UserPtr user = ClientManager::getInstance()->findUser(CID(cid));
		if (user)
		{
			const string hubUrl = client->getHubUrl();//NOTE: core 0.762
			UploadManager::getInstance()->reserveSlot(HintedUser(user, hubUrl));//NOTE: core 0.762
			message = _("Slot granted to ") + WulforUtil::getNicks(user, hubUrl);//NOTE: core 0.762
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
		UserPtr user = ClientManager::getInstance()->findUser(CID(cid));
		if (user)
			QueueManager::getInstance()->removeSource(user, QueueItem::Source::FLAG_REMOVED);
	}
}

void Hub::redirect_client(string address, bool follow)
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

		if (follow)
		{
			// the client is dead, long live the client!
			disconnect_client();

			Func0<Hub> *func = new Func0<Hub>(this, &Hub::clearNickList_gui);
			WulforManager::get()->dispatchGuiFunc(func);

			connectClient_client(address, encoding);
		}
	}
}

void Hub::rebuildHashData_client()
{
	HashManager::getInstance()->rebuild();
}

void Hub::refreshFileList_client()
{
	try
	{
		ShareManager::getInstance()->setDirty();
		ShareManager::getInstance()->refresh(true);
	}
	catch (const ShareException& e)
	{
	}
}

void Hub::addAsFavorite_client()
{
	typedef Func3<Hub, string, Msg::TypeMsg, Sound::TypeSound> F3;
	F3 *func;

	FavoriteHubEntry *existingHub = FavoriteManager::getInstance()->getFavoriteHubEntry(client->getHubUrl());

	if (!existingHub)
	{
		FavoriteHubEntry entry;
		entry.setServer(client->getHubUrl());
		entry.setName(client->getHubName());
		entry.setDescription(client->getHubDescription());
		entry.setNick(client->getMyNick());
		entry.setEncoding(encoding);
		if(!client->getPassword().empty())
			entry.setPassword(client->getPassword());
		else
			entry.setPassword(Util::emptyString);
		entry.setGroup(Util::emptyString);

		FavoriteManager::getInstance()->addFavorite(entry);

		const FavoriteHubEntryList &fh = FavoriteManager::getInstance()->getFavoriteHubs();
		WulforManager::get()->getMainWindow()->updateFavoriteHubMenu_client(fh);

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

/*Inspipred by code UserInfoBase getImageIndex*/
string Hub::getIcons(const Identity& id)
{
	string tmp = "other";
	if(id.isOp()) {
		 tmp = "op";
	} else /*status*/
	{
		string conn = id.getConnection();
		if(	(conn == "28.8Kbps") || (conn == "33.6Kbps") ||	(conn == "56Kbps") || (conn == "Modem") ||	(conn == "ISDN")) {
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
				if( (n =conn.find("/s")) != string::npos)
					conn.erase(n, string::npos);

				double us = conn.empty() ? (8 * Util::toInt64(id.get("US")) / 1024 / 1024): Util::toDouble(conn);
				if(us >= 10) {
					tmp = "ten";
				} else if(us > 0.1) {
					tmp = "zeroone";
				} else if(us >= 0.01) {
					tmp = "zerozeroone";
				} else if(us > 0) {
					tmp = "other";
				}
				else tmp = "other";
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
	params.insert(ParamMap::value_type("Country", (BOOLSETTING(GET_USER_COUNTRY)) ? GeoManager::getInstance()->getCountry(id.getIp(),GeoManager::V4): Util::emptyString ));
	params.insert(ParamMap::value_type("Abbrevation", (BOOLSETTING(GET_USER_COUNTRY)) ? GeoManager::getInstance()->getCountryAbbrevation(id.getIp()): Util::emptyString ));
	params.insert(ParamMap::value_type("Slots", id.get("SL")));
	const string hubs = Util::toString(Util::toInt(id.get("HN")) + Util::toInt(id.get("HR")) + Util::toInt(id.get("HO")));//hubs
	params.insert(ParamMap::value_type("Hubs", hubs));
	params.insert(ParamMap::value_type("PK", id.get("PK")));
	params.insert(ParamMap::value_type("Cheat", id.get("CS")));
	params.insert(ParamMap::value_type("Generator", id.get("GE")));
	params.insert(ParamMap::value_type("Support", id.get("SU")));

	if(id.isBot() || id.isHub()) {
        params.insert(ParamMap::value_type("Type", "A" + id.getNick()));
        addOperator_gui(params);
    } else if (id.isOp()) {
        params.insert(ParamMap::value_type("Type", "B" + id.getNick()));
        addOperator_gui(params);
    } else
        params.insert(ParamMap::value_type("Type", "U" + id.getNick()));
	
	if(id.getUser()->isSet(User::PASSIVE))
	{
		params.insert(ParamMap::value_type("Type", "P" + id.getNick()));
		addPasive_gui(params);
	}
	if(FavoriteManager::getInstance()->isFavoriteUser(id.getUser()))
	{
		params.insert(ParamMap::value_type("Type", "C" + id.getNick()));
		addFavoriteUser_gui(params);
	}
    if(FavoriteManager::getInstance()->isFavoriteIUser(id.getNick()))
	{
		params.insert(ParamMap::value_type("Type", "C" + id.getNick()));
		addFavoriteUser_gui(params);
	}	
	
	if(FavoriteManager::getInstance()->isIgnoredUser(id.getUser()))
     {
		params.insert(ParamMap::value_type("Type", "I" + id.getNick()));
		addIgnore_gui(params);
	}

	if(id.getUser()->isSet(User::PROTECT))
	{
		params.insert(ParamMap::value_type("Type", "R" + id.getNick()));
		addProtected_gui(params);
	}

}

void Hub::download_client(string target, int64_t size, string tth, string cid)
{
	string real = realFile_client(tth);
	if (!real.empty())
	{
		typedef Func2<Hub, string, string> F2;
		F2 *f2 = new F2(this, &Hub::loadImage_gui, real, tth);
		WulforManager::get()->dispatchGuiFunc(f2);

		return;
	}

	try
	{
		UserPtr user = ClientManager::getInstance()->findUser(CID(cid));
		if (user == NULL)
			return;

		string hubUrl = client->getHubUrl();
		QueueManager::getInstance()->add(target, size, TTHValue(tth), HintedUser(user, hubUrl));
	}
	catch (const Exception&)
	{
		typedef Func2<Hub, string, string> F2;
		F2 *f2 = new F2(this, &Hub::loadImage_gui, target, tth);
		WulforManager::get()->dispatchGuiFunc(f2);
	}
}

string Hub::realFile_client(string tth)
{
	try
	{
		string virt = ShareManager::getInstance()->toVirtual(TTHValue(tth));
		string real = ShareManager::getInstance()->toReal(virt,false);

		return real;
	}
	catch (const Exception&)
	{
	}
	return "";
}

void Hub::on(QueueManagerListener::Finished, QueueItem *item, const string& dir, int64_t avSpeed) throw()
{
	typedef Func2<Hub, string, string> F2;
	string tth = item->getTTH().toBase32();

	if (!item->isSet(QueueItem::FLAG_CLIENT_VIEW | QueueItem::FLAG_USER_LIST) && !item->isSet(QueueItem::FLAG_XML_BZLIST))
	{
		F2 *f2 = new F2(this, &Hub::loadImage_gui, item->getTarget(), tth);
		WulforManager::get()->dispatchGuiFunc(f2);
	}
}

void Hub::loadImage_gui(string target, string tth)
{
	if (imageLoad.first != tth)
		return;

	if (imageLoad.second == NULL)
		return;

	if (!g_file_test(target.c_str(), G_FILE_TEST_EXISTS))
	{
		string text = _("loading error");
#if GTK_CHECK_VERSION(2, 12, 0)
		gtk_widget_set_tooltip_text(imageLoad.second, text.c_str());
#else
		gtk_tooltips_set_tip(tips, imageLoad.second, text.c_str(), text.c_str());
#endif
		return;
	}

	GdkPixbuf *source = gdk_pixbuf_new_from_file(target.c_str(), NULL);

	if (!source)
	{
		string text = _("bad image");
#if GTK_CHECK_VERSION(2, 12, 0)
		gtk_widget_set_tooltip_text(imageLoad.second, text.c_str());
#else
		gtk_tooltips_set_tip(tips, imageLoad.second, text.c_str(), text.c_str());
#endif
		return;
	}

	int width = gdk_pixbuf_get_width(source);
	int height = gdk_pixbuf_get_height(source);
	GdkPixbuf *pixbuf = NULL;
	int set_w = 200, set_h = 200;
	double w, h, k;
	w = (double) set_w / width;
	h = (double) set_h / height ;
	k = MIN(w, h);

	if (k >= 1)
		pixbuf = source;
	else
	{
		pixbuf = WulforUtil::scalePixbuf(source, set_w, set_h);
		g_object_unref(source);
	}
	gtk_image_set_from_pixbuf(GTK_IMAGE(imageLoad.second), pixbuf);
	g_object_unref(pixbuf);

	// reset tips
	string name, magnet = imageMagnet.first;
	int64_t size;
	WulforUtil::splitMagnet(magnet, name, size, tth);
	string text = "name: " + name + "\n" + "size: " + Util::formatBytes(size);
#if GTK_CHECK_VERSION(2, 12, 0)
	gtk_widget_set_tooltip_text(imageLoad.second, text.c_str());
#else
	gtk_tooltips_set_tip(tips, imageLoad.second, text.c_str(), t.c_str());
#endif
	imageLoad.first = "";
	imageLoad.second = NULL;
}

void Hub::onImageDestroy_gui(GtkWidget *widget, gpointer data)
{
	Hub *hub = (Hub*) data;

	// fix crash, if entry delete...
	if (!WulforManager::get()->isEntry_gui(hub))
		return;

	ImageList::const_iterator j = hub->imageList.find(widget);

	if (j != hub->imageList.end())
	{
		// fix crash, if image menu active...
		string k = hub->imageMagnet.first;
		string l = j->second;
		string name, tth;
		int64_t size;

		WulforUtil::splitMagnet(k, name, size, tth);

		if (l == tth)
		{
			g_object_set_data(G_OBJECT(hub->getWidget("downloadImageItem")), "container", NULL);
			g_object_set_data(G_OBJECT(hub->getWidget("removeImageItem")), "container", NULL);
		}

		// erase image...
		hub->imageList.erase(j);
	}

	// fix crash...
	if (hub->imageLoad.second == widget)
	{
		hub->imageLoad.first = "";
		hub->imageLoad.second = NULL;
	}
}

gboolean Hub::onImageEvent_gui(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
	Hub *hub = (Hub*) data;

	if (event->button == 3 && event->type == GDK_BUTTON_RELEASE)
	{
		hub->imageMagnet.first = (gchar*) g_object_get_data(G_OBJECT(widget), "magnet");
		hub->imageMagnet.second = (gchar*) g_object_get_data(G_OBJECT(widget), "cid");
		g_object_set_data(G_OBJECT(hub->getWidget("removeImageItem")), "container", (gpointer)widget);
		g_object_set_data(G_OBJECT(hub->getWidget("downloadImageItem")), "container", (gpointer)widget);
		gtk_menu_popup(GTK_MENU(hub->getWidget("imageMenu")), NULL, NULL, NULL, NULL, 0, gtk_get_current_event_time());
	}
	return false;
}

void Hub::onDownloadImageClicked_gui(GtkMenuItem *item, gpointer data)
{
	Hub *hub = (Hub*) data;

	string name, tth, target;
	int64_t size;
	const string magnet = hub->imageMagnet.first;
	const string cid = hub->imageMagnet.second;

	if (WulforUtil::splitMagnet(magnet, name, size, tth))
	{
		GtkWidget *container = (GtkWidget*) g_object_get_data(G_OBJECT(item), "container");

		// if image destroy
		if (container == NULL)
			return;

		GList *childs = gtk_container_get_children(GTK_CONTAINER(container));
		hub->imageLoad.first = tth;
		hub->imageLoad.second = (GtkWidget*)childs->data;
		g_list_free(childs);

		target = Util::getPath(Util::PATH_USER_CONFIG) + "Images" + PATH_SEPARATOR_STR + tth;
		typedef Func4<Hub, string, int64_t, string, string> F4;
		F4 *func = new F4(hub, &Hub::download_client, target, size, tth, cid);
		WulforManager::get()->dispatchClientFunc(func);
	}
}

void Hub::onRemoveImageClicked_gui(GtkMenuItem *item, gpointer data)
{
	Hub *hub = (Hub*) data;

	GtkWidget *container = (GtkWidget*) g_object_get_data(G_OBJECT(item), "container");

	// if image destroy
	if (container == NULL)
		return;

	GList *childs = gtk_container_get_children(GTK_CONTAINER(container));
	GtkWidget *image = (GtkWidget*)childs->data;
	g_list_free(childs);
	gtk_image_set_from_stock(GTK_IMAGE(image), GTK_STOCK_FILE, GTK_ICON_SIZE_BUTTON);

	hub->imageLoad.first = "";
	hub->imageLoad.second = NULL;
}

void Hub::onOpenImageClicked_gui(GtkMenuItem *item, gpointer data)
{
	Hub *hub = (Hub*) data;

	int64_t size;
	string name, tth;
	const string magnet = hub->imageMagnet.first;
	WulforUtil::splitMagnet(magnet, name, size, tth);
	typedef Func1<Hub, string> F1;
	F1 *f = new F1(hub, &Hub::openImage_client, tth);
	WulforManager::get()->dispatchClientFunc(f);
}

void Hub::openImage_client(string tth)
{
	string real = realFile_client(tth);
	typedef Func1<Hub, string> F1;
	F1 *f = new F1(this, &Hub::openImage_gui, real.empty() ? tth : real);
	WulforManager::get()->dispatchGuiFunc(f);
}

void Hub::openImage_gui(string target)
{
	if (!File::isAbsolute(target))
		target = Util::getPath(Util::PATH_USER_CONFIG) + "Images" + PATH_SEPARATOR_STR + target;
	WulforUtil::openURI(target);
}

gboolean Hub::expose(GtkWidget *widget, GdkEventExpose *event, gpointer data)
{
	GTK_WIDGET_CLASS(GTK_WIDGET_GET_CLASS(widget))->expose_event(widget, event);
	return true;
}

void Hub::onItalicButtonClicked_gui(GtkWidget *widget, gpointer data)
{
	Hub *hub = (Hub*) data;

	hub->insertBBcodeEntry_gui("i");
}

void Hub::onBoldButtonClicked_gui(GtkWidget *widget, gpointer data)
{
	Hub *hub = (Hub*) data;

	hub->insertBBcodeEntry_gui("b");
}

void Hub::onUnderlineButtonClicked_gui(GtkWidget *widget, gpointer data)
{
	Hub *hub = (Hub*) data;

	hub->insertBBcodeEntry_gui("u");
}

void Hub::insertBBcodeEntry_gui(string ch)
{
	gint start_pos;
	gint end_pos;
	GtkEditable *chatEntry = GTK_EDITABLE(getWidget("chatEntry"));
	if (gtk_editable_get_selection_bounds(chatEntry, &start_pos, &end_pos))
	{
		gchar *tmp = gtk_editable_get_chars(chatEntry, start_pos, end_pos);
		string text = tmp;
		g_free(tmp);
		string::size_type a = 0, b = 0;
		string res;

		for (;;)
		{
			a = text.find_first_not_of("\r\n\t ", b);

			if (a != string::npos)
			{
				b = text.find_first_of("\r\n\t ", a);

				if (b != string::npos)
				{
					res += "[" + ch + "]" + text.substr(a, b - a) + "[/" + ch + "] ";
				}
				else
				{
					res += "[" + ch + "]" + text.substr(a) + "[/" + ch + "]";
					break;
				}
			}
			else
				break;
		}

		gtk_editable_delete_text(chatEntry, start_pos, end_pos);
		gtk_editable_insert_text(chatEntry, res.c_str(), -1, &start_pos);
		gtk_editable_set_position(chatEntry, -1);
	}
	else
	{
		start_pos = gtk_editable_get_position(chatEntry);
		gtk_editable_insert_text(chatEntry, string("[" + ch + "][/" + ch + "]").c_str(), -1, &start_pos);
		gtk_editable_set_position(chatEntry, start_pos - 4);
	}
}

void Hub::on(FavoriteManagerListener::UserAdded, const FavoriteUser &user) throw()
{
	if (user.getUrl() != client->getHubUrl())
		return;

	ParamMap params;
	params.insert(ParamMap::value_type("Nick", user.getNick()));
	params.insert(ParamMap::value_type("CID", user.getUser()->getCID().toBase32()));
	params.insert(ParamMap::value_type("Order", ClientManager::getInstance()->isOp(user.getUser(), user.getUrl()) ? "O" : "U"));
	params.insert(ParamMap::value_type("Type", "C" + user.getNick()));

	Func1<Hub, ParamMap> *func = new Func1<Hub, ParamMap>(this, &Hub::addFavoriteUser_gui, params);
	WulforManager::get()->dispatchGuiFunc(func);
}

void Hub::on(FavoriteManagerListener::UserRemoved, const FavoriteUser &user) throw()
{
	if (user.getUrl() != client->getHubUrl())
		return;

	ParamMap params;
	params.insert(ParamMap::value_type("Nick", user.getNick()));
	params.insert(ParamMap::value_type("CID", user.getUser()->getCID().toBase32()));

	Func1<Hub, ParamMap> *func = new Func1<Hub, ParamMap>(this, &Hub::removeFavoriteUser_gui, params);
	WulforManager::get()->dispatchGuiFunc(func);
}

void Hub::on(FavoriteManagerListener::IgnoreUserAdded, const FavoriteUser &user) noexcept
{
    if(user.getUrl() != client->getHubUrl())
        return;
    ParamMap params;
    params.insert(ParamMap::value_type("Nick", user.getNick()));
	params.insert(ParamMap::value_type("CID", user.getUser()->getCID().toBase32()));

    Func1<Hub, ParamMap> *func = new Func1<Hub, ParamMap>(this, &Hub::addIgnore_gui, params);
    WulforManager::get()->dispatchGuiFunc(func);
}

void Hub::on(FavoriteManagerListener::IgnoreUserRemoved, const FavoriteUser &user) noexcept
{
    if (user.getUrl() != client->getHubUrl())
		return;

	ParamMap params;
	params.insert(ParamMap::value_type("Nick", user.getNick()));
	params.insert(ParamMap::value_type("CID", user.getUser()->getCID().toBase32()));

    Func1<Hub, ParamMap> * func = new Func1<Hub, ParamMap> (this, &Hub::removeIgnore_gui, params);
    WulforManager::get()->dispatchGuiFunc(func);
}
//Idenpetn Fav
void Hub::on(FavoriteManagerListener::FavoriteIAdded, const string &nick, FavoriteIUser* &user) noexcept
{
	ParamMap params;
	params.insert(ParamMap::value_type("CID", user->getCid()));	
	params.insert(ParamMap::value_type("Nick", nick));
	params.insert(ParamMap::value_type("Type", "C" + nick));
	Func1<Hub, ParamMap> *func = new Func1<Hub, ParamMap>(this,&Hub::addFavoriteUser_gui,params);
	WulforManager::get()->dispatchGuiFunc(func);
}

void Hub::on(FavoriteManagerListener::FavoriteIRemoved, const string &nick, FavoriteIUser* &user) noexcept
{
	ParamMap params;
	params.insert(ParamMap::value_type("Nick", nick));
	params.insert(ParamMap::value_type("CID", user->getCid()));
	Func1<Hub, ParamMap> *func = new Func1<Hub, ParamMap>(this,&Hub::removeFavoriteUser_gui, params);
	WulforManager::get()->dispatchGuiFunc(func);
}
//end
void Hub::on(ClientListener::Connecting, Client *) throw()
{
	typedef Func3<Hub, string, Msg::TypeMsg, Sound::TypeSound> F3;
	F3 *f3 = new F3(this, &Hub::addStatusMessage_gui, _("Connecting to ") + client->getHubUrl() + "...", Msg::STATUS, Sound::NONE);
	WulforManager::get()->dispatchGuiFunc(f3);
}

void Hub::on(ClientListener::Connected, Client *) throw()
{
	typedef Func4<Hub, string, Msg::TypeMsg, Sound::TypeSound, Notify::TypeNotify> F4;
	F4 *func = new F4(this, &Hub::addStatusMessage_gui, _("Connected"), Msg::STATUS, Sound::HUB_CONNECT, Notify::HUB_CONNECT);
	WulforManager::get()->dispatchGuiFunc(func);
}

void Hub::on(ClientListener::UserUpdated, Client *, const OnlineUser &user) throw()
{
	Identity id = user.getIdentity();

	if (!id.isHidden())
	{
		ParamMap params;
		getParams_client(params, id);
		//BMDC++
		if(user.getIdentity().getUser()->isSet(User::PASSIVE))
		{
            Func1<Hub, ParamMap> *func = new Func1<Hub, ParamMap>(this, &Hub::addPasive_gui, params);
            WulforManager::get()->dispatchGuiFunc(func);
		}
		if (user.getIdentity().isOp())
		{
		    Func1<Hub, ParamMap> *func = new Func1<Hub, ParamMap>(this, &Hub::addOperator_gui, params);
            WulforManager::get()->dispatchGuiFunc(func);
		}
		if (user.getIdentity().getUser()->isSet(User::PROTECT))
		{
		   Func1<Hub, ParamMap> *func = new Func1<Hub, ParamMap>(this, &Hub::addProtected_gui, params);
           WulforManager::get()->dispatchGuiFunc(func);
		}
        //end
		Func1<Hub, ParamMap> *func = new Func1<Hub, ParamMap>(this, &Hub::updateUser_gui, params);
		WulforManager::get()->dispatchGuiFunc(func);
	}
}

void Hub::on(ClientListener::UsersUpdated, Client *, const OnlineUserList &list) throw()
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

void Hub::on(ClientListener::UserRemoved, Client *, const OnlineUser &user) throw()
{
	Func1<Hub, string> *func = new Func1<Hub, string>(this, &Hub::removeUser_gui, user.getUser()->getCID().toBase32());
	WulforManager::get()->dispatchGuiFunc(func);
}

void Hub::on(ClientListener::Redirect, Client *, const string &address) throw()
{
	// redirect_client() crashes unless I put it into the dispatcher (why?)
	typedef Func2<Hub, string, bool> F2;
	F2 *func = new F2(this, &Hub::redirect_client, address, BOOLSETTING(AUTO_FOLLOW));
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

void Hub::on(ClientListener::GetPassword, Client *) throw()
{
	if (!client->getPassword().empty()) {
		client->password(client->getPassword());
        //Show if stored pass send..in status
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
	typedef Func1<Hub, string> F1;
	string hubName = Util::emptyString;
	string hubText = client->getTabText();
	string iconPath = client->getTabIconStr();

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
		F1 *f = new F1(this, &BookEntry::setLabel_gui, hubText);
		WulforManager::get()->dispatchGuiFunc(f);
		return;
	}

	if (client->getHubName().empty())
		hubName += client->getAddress() + ":" + client->getPort();
	else
		hubName += client->getHubName();

	if (!client->getHubDescription().empty())
		hubName += " - " + client->getHubDescription();

	F1 *func1 = new F1(this, &BookEntry::setLabel_gui, hubName);
	WulforManager::get()->dispatchGuiFunc(func1);
}

/* Inspired by code of RSX*/
string Hub::formatAdditionalInfo(const string& aIp, bool sIp, bool sCC, bool isPm) {
	string ret = Util::emptyString;

	if(!aIp.empty()) {
		string cc = (BOOLSETTING(GET_USER_COUNTRY)) ? GeoManager::getInstance()->getCountryAbbrevation(aIp) : Util::emptyString;
		string countryn = (BOOLSETTING(GET_USER_COUNTRY)) ? GeoManager::getInstance()->getCountry(aIp) : Util::emptyString;
		bool showIp = BOOLSETTING(USE_IP) || sIp;
		bool showCc = (BOOLSETTING(GET_USER_COUNTRY) || sCC) && !cc.empty();
		bool useFlagIcons = (WGETB("use-flag") && !cc.empty());

		if(showIp) {
			ret = "[ " + aIp + " ] ";
		}

		if(showCc) {
			ret += "[" + countryn + "] ";
		}

		if(useFlagIcons) {
			ret += " [ccc]" + cc + "[/ccc] ";
		}

	}
	return Text::toT(ret);
}

void Hub::on(ClientListener::Message, Client*, const ChatMessage& message) throw() //NOTE: core 0.762
{
	string txt = message.text;
	if(PluginManager::getInstance()->onChatDisplay(client, txt))
		return;

	if (message.text.empty())
		return;

	Msg::TypeMsg typemsg;
	string line;
	string tmp_text = message.text;

	if( (!message.from->getIdentity().isHub()) && (!message.from->getIdentity().isBot()) )
	{

		string info;
		string extraInfo;
		info = formatAdditionalInfo(message.from->getIdentity().getIp(), BOOLSETTING(USE_IP), BOOLSETTING(GET_USER_COUNTRY), message.to && message.replyTo);

		//Extra Info
		dcpp::ParamMap params;
		params["hubURL"] = client->getHubUrl();
		client->getHubIdentity().getParams(params, "hub", false);
		client->getMyIdentity().getParams(params, "my", true);
		message.from->getIdentity().getParams(params, "user", true);
		extraInfo = Text::toT(Util::formatParams(client->getChatExtraInfo(), params));
		if(!extraInfo.empty())
			info += " " + extraInfo + " ";

	    line += info;
	}

	bool third = false;
	string mess;

	mess = message.text;
	{
		size_t nestle = message.text.find("/me");
		if(nestle != string::npos)
		 {

			size_t nt = message.text.find_first_of(" ",nestle);
			if( message.text.compare(0,nt,"/me") == 0) {
				 	third = true;
					mess.replace(0,nt+1,"");
			}
		}
	}

	if (third || message.thirdPerson)
		line += "* " + message.from->getIdentity().getNick() + " " +  (mess.empty() ? message.text : mess);
	else
		line += "<" + message.from->getIdentity().getNick() + "> " + message.text;

    if(FavoriteManager::getInstance()->isIgnoredUser(message.from->getIdentity().getUser()))
	{
		string error = _("Ignored message from User ") + message.from->getIdentity().getNick() + " from " + ((message.to && message.replyTo) ? "PM" : "Mainchat");
		error += _("\nMessage: ") + message.text + "\n";

		dcpp::ParamMap params;
		params["message"] = error;

		if(WGETB("log-messages") )
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
		const OnlineUser *user = (message.replyTo->getUser() == ClientManager::getInstance()->getMe())?
			message.to : message.replyTo;

		if (message.from->getIdentity().isOp()) typemsg = Msg::OPERATOR;
		else if (message.from->getUser() == client->getMyIdentity().getUser()) typemsg = Msg::MYOWN;
		else typemsg = Msg::PRIVATE;

		if (user->getIdentity().isHub() && BOOLSETTING(IGNORE_HUB_PMS))
		{
			error = _("Ignored private message from hub");
			typedef Func3<Hub, string, Msg::TypeMsg, Sound::TypeSound> F3;
			F3 *func = new F3(this, &Hub::addStatusMessage_gui, error, Msg::STATUS, Sound::NONE);
			WulforManager::get()->dispatchGuiFunc(func);
		}
		else if (user->getIdentity().isBot() && BOOLSETTING(IGNORE_BOT_PMS))
		{
			error = _("Ignored private message from bot ") + user->getIdentity().getNick();
			typedef Func3<Hub, string, Msg::TypeMsg, Sound::TypeSound> F3;
			F3 *func = new F3(this, &Hub::addStatusMessage_gui, error, Msg::STATUS, Sound::NONE);
			WulforManager::get()->dispatchGuiFunc(func);
		}
		else
		{
			typedef Func6<Hub, Msg::TypeMsg, string, string, string, string, bool> F6;
			F6 *func = new F6(this, &Hub::addPrivateMessage_gui, typemsg, message.from->getUser()->getCID().toBase32(),
			user->getUser()->getCID().toBase32(), client->getHubUrl(), line, TRUE);
			WulforManager::get()->dispatchGuiFunc(func);
		}
	}
	else
	{
		 // chat message
		string cid = message.from->getIdentity().getUser()->getCID().toBase32();

		if (message.from->getIdentity().isHub()) typemsg = Msg::STATUS;
		else if (message.from->getUser() == client->getMyIdentity().getUser()) typemsg = Msg::MYOWN;
		else typemsg = Msg::GENERAL;

		if (BOOLSETTING(FILTER_MESSAGES))
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

		if (client->settings.getLogChat())
		{
			dcpp::ParamMap params;
			params["message"] = tmp_text;//line
			client->getHubIdentity().getParams(params, "hub", false);
			params["hubURL"] = client->getHubUrl();
			client->getMyIdentity().getParams(params, "my", true);
			LOG(LogManager::CHAT, params);
		}

		typedef Func3<Hub, string, string, Msg::TypeMsg> F3;
		F3 *func = new F3(this, &Hub::addMessage_gui, cid, line, typemsg);
		WulforManager::get()->dispatchGuiFunc(func);

		// Set urgency hint if message contains user's nick
		if (BOOLSETTING(BOLD_HUB) && message.from->getIdentity().getUser() != client->getMyIdentity().getUser())
		{
              if( !isActive_gui() && WGETB("bold-all-tab"))
			{
					typedef Func0<Hub> F0;
					F0 *func = new F0(this, &Hub::setUrgent_gui);
					WulforManager::get()->dispatchGuiFunc(func);
					return;//thinking
			}

			if (message.text.find(client->getMyIdentity().getNick()) != string::npos)
			{
				typedef Func0<Hub> F0;
				F0 *func = new F0(this, &Hub::setUrgent_gui);
				WulforManager::get()->dispatchGuiFunc(func);
			}
		}
	}
} //NOTE: core 0.762

void Hub::on(ClientListener::StatusMessage, Client *, const string &message, int /* flag */) throw()
{
	if (!message.empty())
	{
		if (BOOLSETTING(FILTER_MESSAGES))
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

		if (BOOLSETTING(LOG_STATUS_MESSAGES))
		{
			dcpp::ParamMap params;
			client->getHubIdentity().getParams(params, "hub", FALSE);
			params["hubURL"] = client->getHubUrl();
			client->getMyIdentity().getParams(params, "my", TRUE);
			params["message"] = message;
			LOG(LogManager::STATUS, params);
		}

		typedef Func3<Hub, string, string, Msg::TypeMsg> F3;
		F3 *func = new F3(this, &Hub::addMessage_gui, "", message, Msg::STATUS);
		WulforManager::get()->dispatchGuiFunc(func);
	}
}

void Hub::on(ClientListener::NickTaken, Client *) throw()
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
//[BMDC++
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

void Hub::on(ClientListener::ClientLine, Client* , const string &mess, int type) noexcept
{
	typedef Func3<Hub, string, Msg::TypeMsg, Sound::TypeSound> F3;
	F3 *func = new F3(this, &Hub::addStatusMessage_gui, mess, Msg::STATUS, Sound::NONE);
	WulforManager::get()->dispatchGuiFunc(func);
}

void Hub::on(ClientListener::HubIcon, Client *, const string &url) noexcept
{
	typedef Func1<Hub, string> F1;
	F1 *func = new F1(this, &Hub::setHubIcon_gui, url);
	WulforManager::get()->dispatchGuiFunc(func);
}

void Hub::setHubIcon_gui(string url)
{
	if(g_ascii_strncasecmp(url.c_str(), "http://", 7) == 0 ||
		g_ascii_strncasecmp(url.c_str(), "https://", 8) == 0 ||
		g_ascii_strncasecmp(url.c_str(), "www.", 4) == 0)
	{
	   iconshttp.reset(new HttpDownload(url,[this] { updateIcons(); }, false));
	}
}
void Hub::updateIcons()
{
	if(!iconshttp->buf.empty())
	{
		string path = Util::getPath(Util::PATH_USER_CONFIG) + "/icons/" + Util::toString(Util::rand(0,2147483647))+".png";
		File(path,File::WRITE, File::CREATE | File::TRUNCATE).write(iconshttp->buf);
		setIconPixbufs_gui(path);
	}
}
//custom popup menu
GtkWidget *Hub::createmenu()
{
    GtkWidget *item = getFItem();
	gtk_menu_item_set_label(GTK_MENU_ITEM(item),address.c_str());
	
	userCommandMenu1->cleanMenu_gui();
    userCommandMenu1->addUser(client->getMyIdentity().getUser()->getCID().toBase32());
    userCommandMenu1->addHub(client->getHubUrl());
    userCommandMenu1->buildMenu_gui();
    GtkWidget *menu = userCommandMenu1->getContainer();

    GtkWidget *copyHubUrl = gtk_menu_item_new_with_label(_("Copy URL"));
    GtkWidget *close = gtk_menu_item_new_with_label(_("Close"));
    GtkWidget *addFav = gtk_menu_item_new_with_label(_("Add to Favorite hubs"));
    GtkWidget *remfav = gtk_menu_item_new_with_label(_("Remove from Favorite hubs"));
    GtkWidget *setTab = gtk_menu_item_new_with_label(_("Set Tab Name"));

    gtk_menu_shell_append(GTK_MENU_SHELL(menu),close);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu),copyHubUrl);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu),addFav);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu),remfav);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), setTab);
    gtk_widget_show(close);
    gtk_widget_show(copyHubUrl);
    gtk_widget_show(addFav);
    gtk_widget_show(remfav);
    gtk_widget_show(setTab);
    gtk_widget_show_all(userCommandMenu1->getContainer());

    g_signal_connect_swapped(copyHubUrl, "activate", G_CALLBACK(onCopyHubUrl), (gpointer)this);
    g_signal_connect_swapped(close, "activate", G_CALLBACK(onCloseItem), (gpointer)this);
    g_signal_connect_swapped(addFav, "activate", G_CALLBACK(onAddFavItem), (gpointer)this);
    g_signal_connect_swapped(remfav, "activate", G_CALLBACK(onRemoveFavHub), (gpointer)this);
    g_signal_connect_swapped(setTab, "activate", G_CALLBACK(onSetTabText), (gpointer)this);
    return menu;
}

void Hub::onCloseItem(gpointer data)
{
    BookEntry *entry = (BookEntry *)data;
    WulforManager::get()->getMainWindow()->removeBookEntry_gui(entry);
}

void Hub::onCopyHubUrl(gpointer data)
{
    Hub *hub = (Hub *)data;
    gtk_clipboard_set_text(gtk_clipboard_get(GDK_SELECTION_CLIPBOARD), hub->address.c_str(), hub->address.length());
}

void Hub::onAddFavItem(gpointer data)
{
	Hub *hub = (Hub *)data;
	hub->addAsFavorite_client();
}

void Hub::onRemoveFavHub(gpointer data)
{
    Hub *hub = (Hub *)data;
    hub->removeAsFavorite_client();
}

void Hub::on_setImage_tab(GtkButton *widget, gpointer data)
{
	Hub *hub = (Hub *)data;
	GtkWidget *dialog = gtk_file_chooser_dialog_new ("Open Icon File to Set to Tab",
						GTK_WINDOW(WulforManager::get()->getMainWindow()->getContainer()),
				        GTK_FILE_CHOOSER_ACTION_OPEN,
				        GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
				        GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
				        NULL);
	if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT)
	{
		char *filename;
		filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));
		string tmp = Util::getFileExt(string(filename));
		tmp = WulforUtil::StringToUpper(tmp);
		if(tmp == ".PNG" || tmp == ".JPG" || tmp == ".GIF")
		{
			GdkPixbuf *pixbuf =	gdk_pixbuf_new_from_file_at_scale(filename,15,15,FALSE,NULL);
			gtk_image_set_from_pixbuf(GTK_IMAGE(hub->tab_image),pixbuf);
			hub->client->setTabIconStr(string(filename));
			hub->client->fire(ClientListener::HubUpdated(), hub->client);

			FavoriteHubEntryPtr fav = FavoriteManager::getInstance()->getFavoriteHubEntry(hub->client->getHubUrl());
			if(fav != NULL) {
				fav->setTabIconStr(hub->client->getTabIconStr());
				FavoriteManager::getInstance()->save();
			}
		}
		g_free (filename);

	}
	gtk_widget_destroy (dialog);

}

void Hub::onSetTabText(gpointer data)
{ ((Hub *)data)->SetTabText(data); }

void Hub::SetTabText(gpointer data)
{
	Hub *hub = (Hub *)data;
	GtkDialog *dialog =  GTK_DIALOG(gtk_dialog_new_with_buttons ("Setting for a Tab Text",
                                         GTK_WINDOW(WulforManager::get()->getMainWindow()->getContainer()),
                                         GTK_DIALOG_DESTROY_WITH_PARENT,
                                         GTK_STOCK_OK,
                                         GTK_RESPONSE_OK,
                                         GTK_STOCK_CANCEL,
                                         GTK_RESPONSE_CANCEL,
                                         NULL));
   GtkWidget *content_area = gtk_dialog_get_content_area (dialog);
   GtkWidget *entry = gtk_entry_new();
   GtkWidget *label = gtk_label_new("Text: ");
   GtkWidget *hbox = gtk_hbox_new(TRUE,0);
   GtkWidget *check = gtk_toggle_button_new_with_label("Set Icon Aviable");
   GdkPixbuf *pixbuf =	gdk_pixbuf_new_from_file_at_scale(hub->client->getTabIconStr().c_str(),15,15,FALSE,NULL);

   hub->tab_image = gtk_image_new_from_pixbuf(pixbuf);
   hub->tab_button = gtk_button_new_with_label("Set Icon: ");

   g_signal_connect(GTK_BUTTON(hub->tab_button), "clicked", G_CALLBACK(on_setImage_tab), hub);
   g_signal_connect(GTK_TOGGLE_BUTTON(check), "toggled", G_CALLBACK(onToglleButtonIcon),hub);

   gtk_box_pack_start(GTK_BOX(hbox),check, FALSE, TRUE, 0);
   gtk_box_pack_start(GTK_BOX(hbox), hub->tab_button, FALSE, TRUE, 0);
   gtk_box_pack_start(GTK_BOX(hbox), hub->tab_image, FALSE, TRUE, 0);
   gtk_container_add(GTK_CONTAINER(content_area), label);
   gtk_container_add(GTK_CONTAINER(content_area), entry);
   gtk_container_add(GTK_CONTAINER(content_area), hbox);


   gtk_widget_show(hub->tab_button);
   gtk_widget_show(hub->tab_image);
   gtk_widget_show(entry);
   gtk_widget_show(label);
   gtk_widget_show(hbox);
   gtk_widget_show(check);
   gtk_entry_set_text(GTK_ENTRY(entry) , hub->client->getTabText().c_str());

   gint response  = gtk_dialog_run(dialog);

   if(response == GTK_RESPONSE_OK)
    {
		const gchar *text = gtk_entry_get_text(GTK_ENTRY(entry));
		hub->client->setTabText(string(text));
		hub->client->fire(ClientListener::HubUpdated(), hub->client);

		FavoriteHubEntryPtr fav = FavoriteManager::getInstance()->getFavoriteHubEntry(hub->client->getHubUrl());
		if(fav != NULL) {
			fav->setTabText(hub->client->getTabText());
			FavoriteManager::getInstance()->save();
		}
	}
	gtk_widget_destroy(GTK_WIDGET(dialog));
}

void Hub::onToglleButtonIcon(GtkToggleButton *button, gpointer data)
{
	Hub *hub = (Hub *)data;
	gboolean active = gtk_toggle_button_get_active(button);
	if(active)
	{
		hub->client->setTabIconStr(Util::emptyString);
		hub->client->fire(ClientListener::HubUpdated(), hub->client);

		FavoriteHubEntryPtr fav = FavoriteManager::getInstance()->getFavoriteHubEntry(hub->client->getHubUrl());
		if(fav != NULL) {
			fav->setTabIconStr(Util::emptyString);
			FavoriteManager::getInstance()->save();
		}
	}
	gtk_widget_set_sensitive(hub->tab_button, !active);
	gtk_widget_set_sensitive(hub->tab_image, !active);
}
