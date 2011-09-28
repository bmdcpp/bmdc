/*
 * Copyright © 2004-2011 Jens Oknelid, paskharen@gmail.com
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
#include "privatemessage.hh"
#include "search.hh"
#include "settingsmanager.hh"
#include "emoticonsdialog.hh"
#include "emoticons.hh"
#include "UserCommandMenu.hh"
#include "wulformanager.hh"
#include "WulforUtil.hh"
#include "version.hh"

#include <dcpp/StringTokenizer.h>

#ifdef _USELUA
	#include <dcpp/ScriptManager.h>
#endif

#include <dcpp/Client.h>
//CMD
#include <dcpp/QueueManager.h>
#include <dcpp/RsxUtil.h>
#include <dcpp/HubUsersMap.h>


using namespace std;
using namespace dcpp;

const string Hub::tagPrefix = "#";
const string Hub::tPrefix = "*";

Hub::Hub(const string &address, const string &encoding):
	BookEntry(Entry::HUB, address, "hub.glade", address),
	client(NULL),
	historyIndex(0),
	totalShared(0),
	address(address),
	encoding(encoding),
	scrollToBottom(TRUE),
	PasswordDialog(FALSE),
	WaitingPassword(FALSE)
{
	// Configure the dialog
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(getWidget("userListCheckButton")), TRUE);

	// Initialize nick treeview
	nickView.setView(GTK_TREE_VIEW(getWidget("nickView")), true, "hub");
	nickView.insertColumn(N_("Nick"), G_TYPE_STRING, TreeView::ICON_STRING_TEXT_COLOR, 100, "Icon", "NickColor");
	nickView.insertColumn(N_("Shared"), G_TYPE_INT64, TreeView::SIZE, 75);
	nickView.insertColumn(N_("Description"), G_TYPE_STRING, TreeView::STRING, 85);
	nickView.insertColumn(N_("Tag"), G_TYPE_STRING, TreeView::STRING, 100);
	nickView.insertColumn(N_("Connection"), G_TYPE_STRING, TreeView::STRING, 85);
	nickView.insertColumn("IP", G_TYPE_STRING, TreeView::STRING, 85);
	nickView.insertColumn(N_("eMail"), G_TYPE_STRING, TreeView::STRING, 90);
	nickView.insertColumn(N_("CC"), G_TYPE_STRING, TreeView::PIXBUF_STRING, 50, "Country");//Patched
	nickView.insertColumn(N_("Exact Share"),G_TYPE_INT64, TreeView::ESIZE, 100);//Patched
	nickView.insertColumn(N_("Slots"),G_TYPE_STRING, TreeView::STRING, 50);//Patched
	nickView.insertColumn(N_("Hubs"), G_TYPE_STRING, TreeView::STRING, 50);//Patched
	nickView.insertColumn("PK", G_TYPE_STRING, TreeView::STRING, 80);//PK
	nickView.insertColumn(N_("Cheat"), G_TYPE_STRING, TreeView::STRING, 80);//Cheat
	nickView.insertColumn(N_("Generator"), G_TYPE_STRING, TreeView::STRING, 80);//Generator
	nickView.insertColumn(N_("Support"), G_TYPE_STRING, TreeView::STRING, 80);
	nickView.insertHiddenColumn("Country", GDK_TYPE_PIXBUF);//Country
	nickView.insertHiddenColumn("ClientType", G_TYPE_STRING); //User/BOT/OP/ADMIN/FavUser
	nickView.insertHiddenColumn("Icon", G_TYPE_STRING);
	nickView.insertHiddenColumn("Nick Order", G_TYPE_STRING);
	nickView.insertHiddenColumn("Favorite", G_TYPE_STRING);
	nickView.insertHiddenColumn("CID", G_TYPE_STRING);
	nickView.insertHiddenColumn("NickColor", G_TYPE_STRING);
	nickView.finalize();
	nickStore = gtk_list_store_newv(nickView.getColCount(), nickView.getGTypes());
	gtk_tree_view_set_model(nickView.get(), GTK_TREE_MODEL(nickStore));
	g_object_unref(nickStore);

	nickSelection = gtk_tree_view_get_selection(nickView.get());
	gtk_tree_selection_set_mode(gtk_tree_view_get_selection(nickView.get()), GTK_SELECTION_MULTIPLE);
	string sort = BOOLSETTING(SORT_FAVUSERS_FIRST) ? "ClientType"/*"Favorite"*/ : "Nick Order";

	nickView.setSortColumn_gui(N_("Nick"), sort);
	gtk_tree_sortable_set_sort_column_id(GTK_TREE_SORTABLE(nickStore), nickView.col(sort), GTK_SORT_ASCENDING);
	gtk_tree_view_column_set_sort_indicator(gtk_tree_view_get_column(nickView.get(), nickView.col(N_("Nick"))), TRUE);
	gtk_tree_view_set_fixed_height_mode(nickView.get(), TRUE);
    gtk_tree_view_set_search_equal_func(nickView.get(), onNickListSearch_gui, 0,0);
	
	nickView.setSelection(nickSelection);
	nickView.buildCopyMenu(getWidget("CopyMenus"));
	
	g_object_set(G_OBJECT(nickView.get()), "has-tooltip", TRUE, NULL);
	g_signal_connect(nickView.get(), "query-tooltip", G_CALLBACK(onUserListTooltip_gui), (gpointer)this);
	g_signal_connect (gtk_tree_view_get_selection (GTK_TREE_VIEW (nickView.get())), "changed", G_CALLBACK (selection_changed_cb_gui), GTK_WIDGET(nickView.get()));
	/* Set a tooltip on the column */
	set_Header_tooltip();

	// Initialize the chat window
	if (BOOLSETTING(USE_OEM_MONOFONT))
	{
		PangoFontDescription *fontDesc = pango_font_description_new();
		pango_font_description_set_family(fontDesc, "Mono");
		gtk_widget_modify_font(getWidget("chatText"), fontDesc);
		pango_font_description_free(fontDesc);
	}

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
	
	// menu
	g_object_ref_sink(getWidget("nickMenu"));
	g_object_ref_sink(getWidget("magnetMenu"));
	g_object_ref_sink(getWidget("linkMenu"));
	g_object_ref_sink(getWidget("hubMenu"));
	g_object_ref_sink(getWidget("ipMenu"));

	// Initialize the user command menu
	userCommandMenu = new UserCommandMenu(getWidget("usercommandMenu"), ::UserCommand::CONTEXT_USER);//NOTE: core 0.762
	addChild(userCommandMenu);

	userCommandMenu1 = new UserCommandMenu(getNewTabMenu(), ::UserCommand::CONTEXT_HUB);
	addChild(userCommandMenu1);

	// Emoticons dialog
	emotdialog = new EmoticonsDialog(getWidget("chatEntry"), getWidget("emotButton"), getWidget("emotPacksMenu"));
	if (!WGETB("emoticons-use"))
		gtk_widget_set_sensitive(getWidget("emotButton"), FALSE);
	useEmoticons = TRUE;

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
	// Patched
	g_signal_connect(getWidget("IgnoreUserItem"), "activate", G_CALLBACK(onAddIgnItemClicked_gui), (gpointer)this);
	g_signal_connect(getWidget("unIgnoreUserItem"), "activate", G_CALLBACK(onRemoveIgnItemClicked_gui), (gpointer)this);
	g_signal_connect(getWidget("UserInfo") , "activate", G_CALLBACK(onUserInfo_gui), (gpointer)this);
	g_signal_connect(getWidget("TestSURItem"), "activate", G_CALLBACK(onTestSUR_gui), (gpointer)this);
	g_signal_connect(getWidget("CheckUserItem"), "activate", G_CALLBACK(onCheckFL), (gpointer)this);
	g_signal_connect(getWidget("Protect"), "activate", G_CALLBACK(onProtect), (gpointer)this);
	g_signal_connect(getWidget("UnProtect"), "activate", G_CALLBACK(onUnProtect), (gpointer)this);
    // Refresh UL Button & Item
	g_signal_connect(getWidget("buttonrefresh"), "clicked", G_CALLBACK(refreshul), (gpointer)this);
	g_signal_connect(getWidget("userlistrefreshMenuItem"),"activate",G_CALLBACK(refreshul),(gpointer)this);
	// End
	g_signal_connect(getWidget("downloadBrowseItem"), "activate", G_CALLBACK(onDownloadToClicked_gui), (gpointer)this);
	g_signal_connect(getWidget("downloadItem"), "activate", G_CALLBACK(onDownloadClicked_gui), (gpointer)this);
	//
	g_signal_connect(getWidget("ripeMenuItem"), "activate", G_CALLBACK(ripeIp) , (gpointer)this);
	g_signal_connect(getWidget("copyItem") ,"activate", G_CALLBACK(copyIp) , (gpointer)this);
	// End
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
	TagsMap[Tag::TAG_HIGHL] = createTag_gui("TAG_HIGHL", Tag::TAG_HIGHL);
	TagsMap[Tag::TAG_MYNICK] = createTag_gui("TAG_MYNICK", Tag::TAG_MYNICK);
	TagsMap[Tag::TAG_NICK] = createTag_gui("TAG_NICK", Tag::TAG_NICK);
	TagsMap[Tag::TAG_OPERATOR] = createTag_gui("TAG_OPERATOR", Tag::TAG_OPERATOR);
	TagsMap[Tag::TAG_FAVORITE] = createTag_gui("TAG_FAVORITE", Tag::TAG_FAVORITE);
	TagsMap[Tag::TAG_URL] = createTag_gui("TAG_URL", Tag::TAG_URL);
	TagsMap[Tag::TAG_IPADR] = createTag_gui("TAG_IPADR", Tag::TAG_IPADR);

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

	RecentHubEntry r;
	r.setName("*");
	r.setDescription("***");
	r.setUsers("*");
	r.setShared("*");
	r.setServer(address);
	FavoriteManager::getInstance()->addRecent(r);

	// log chat
	FavoriteHubEntry *entry = FavoriteManager::getInstance()->getFavoriteHubEntry(address);

	if(entry != NULL)
	{
		logChat = entry->getLogChat() ? true : false;
	}
	// End
	tooltip = gtk_tooltips_new();

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
	
	g_object_unref(getWidget("nickMenu"));
	g_object_unref(getWidget("magnetMenu"));
	g_object_unref(getWidget("linkMenu"));
	g_object_unref(getWidget("hubMenu"));
	g_object_unref(getWidget("ipMenu"));
}

void Hub::selection_changed_cb_gui (GtkTreeSelection *selection,
		      GtkWidget        *tree_view)
{
  gtk_widget_trigger_tooltip_query (tree_view);
}

void Hub::set_Header_tooltip()//How beter ?
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

void Hub::show()
{
	// Connect to the hub
	typedef Func2<Hub, string, string> F2;
	F2 *func = new F2(this, &Hub::connectClient_client, address, encoding);
	WulforManager::get()->dispatchClientFunc(func);
}

void Hub::setStatus_gui(string statusBar, string text)
{
	if (!statusBar.empty() && !text.empty())
	{
		if (statusBar == "statusMain")
		{
				//tooltipcount++;

			if(statustext.size() > maxtooltip)
			{
					statustext.pop();
			}
				
			queue<string> tmp = statustext;
			string statusTool;
			while(!tmp.empty())
			{
				statusTool+="\n"+tmp.front();
				tmp.pop();
			}
			statustext.push(text);
			statusTool+"\n"+text;
			
			gtk_tooltips_set_tip (tooltip, getWidget("statusMain"), statusTool.c_str(), NULL);
			text = "[" + Util::getShortTimeString() + "] " + text;
		}
		gtk_statusbar_pop(GTK_STATUSBAR(getWidget(statusBar)), 0);
		gtk_statusbar_push(GTK_STATUSBAR(getWidget(statusBar)), 0, text.c_str());
	}
}

bool Hub::findUser_gui(const string &cid, GtkTreeIter *iter)
{
	std::unordered_map<string, GtkTreeIter>::const_iterator it = userIters.find(cid);

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
	std::unordered_map<string, string>::const_iterator it = userMap.find(nick);

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
	/**/
	bool isOP = userOPMap.find(cid) != userOPMap.end();
	bool isPasive = userPasiveMap.find(cid) != userPasiveMap.end();
	bool isIgnore = userIgnoreMap.find(cid) != userIgnoreMap.end();
	bool protect = userProtect.find(cid) != userProtect.end();
	/* Country */
	#ifndef _DEBUG
		string country((!(params["IP"].empty())) ? Util::getIpCountry(params["IP"]).c_str() : string("").c_str());//Country from IP
	#else
		string country("CZ");
	#endif
	/*end*/
	string hubs(params["Hubs"].c_str());
	string slots(params["Slots"].c_str());
	/**/
	string sup(params["SUP"].c_str());
	string cheat(params["Cheat"].c_str());
	/* Country */
	#ifndef _DEBUG
		GdkPixbuf *buf = WulforUtil::loadCountry(params["Country"]);
	#else
		GdkPixbuf *buf = WulforUtil::loadCountry("CZ");
	#endif
	
	if (findUser_gui(cid, &iter))
	{
		totalShared += shared - nickView.getValue<int64_t>(&iter, N_("Shared"));
		string nick = nickView.getString(&iter, N_("Nick"));

		if (nick != Nick)
		{
			// User has changed nick, update userMap and remove the old Nick tag
			userMap.erase(nick);
			removeTag_gui(nick);
			userMap.insert(UserMap::value_type(Nick, cid));

			// update favorite
			if (favorite)
				userFavoriteMap[cid] = Nick;
			// Update OP,Pass,Ignored,Protected
			if(isOP)
				userOPMap[cid] = Nick;
			if(isPasive)
				userPasiveMap[cid] = Nick;
			if (isIgnore)
				userIgnoreMap[cid] = Nick;
			if(protect)
				userProtect[cid] = Nick;
			//end
		}

		//Color of OP,Pasive, Fav, Ignore, Protect
		//string nickColor = (protect ? "#8B6914":(isOP ? "#1E90FF" : (isPasive ? "#747677" :(favorite ? "#ff0000" : (isIgnore? "#9affaf" :"#000000")))));
		string nickColor = (protect ? WGETS("userlist-text-protected") :(isOP ? WGETS("userlist-text-operator") : (isPasive ? WGETS("userlist-text-pasive") :(favorite ? WGETS("userlist-text-favorite") : ( isIgnore ? WGETS("userlist-text-ignored") : WGETS("userlist-text-normal"))))));

		gtk_list_store_set(nickStore, &iter,
			nickView.col(N_("Nick")), Nick.c_str(),
			nickView.col(N_("Shared")), shared,
			nickView.col(N_("Description")), params["Description"].c_str(),
			nickView.col(N_("Tag")), params["Tag"].c_str(),
 			nickView.col(N_("Connection")), params["Connection"].c_str(),
			nickView.col("IP"), params["IP"].c_str(),
			nickView.col(N_("eMail")), params["eMail"].c_str(),
			nickView.col(N_("CC")), country.c_str(),
			nickView.col(N_("Exact Share")), shared,
			nickView.col(N_("Slots")), slots.c_str(),
			nickView.col(N_("Hubs")), hubs.c_str(),
			nickView.col("PK"), sup.c_str(),
			nickView.col(N_("Cheat")), cheat.c_str(),
			nickView.col("Generator"), params["FLGEN"].c_str(),
			nickView.col(N_("Support")), params["SUPPORT"].c_str(),
			nickView.col("ClientType"), params["TypeC"].c_str(),
			nickView.col("Country"), buf,
			nickView.col("Icon"), icon.c_str(),
			nickView.col("Nick Order"), nickOrder.c_str(),
			nickView.col("Favorite"), favorite ? ("f" + nickOrder).c_str() : nickOrder.c_str(),
			nickView.col("CID"), cid.c_str(),
			nickView.col("NickColor"), nickColor.c_str(),
			-1);

	}
	else
	{
		totalShared += shared;
		userMap.insert(UserMap::value_type(Nick, cid));

		//color of Op, Pasive...
		//string nickColor = (protect? "#8B6914":(isOP ? "#1E90FF" : (isPasive ? "#747677" :(favorite ? "#ff0000" : (isIgnore? "#9affaf" : "#000000")))));
		string nickColor = (protect ? WGETS("userlist-text-protected") :(isOP ? WGETS("userlist-text-operator") : (isPasive ? WGETS("userlist-text-pasive") :(favorite ? WGETS("userlist-text-favorite") : ( isIgnore ? WGETS("userlist-text-ignored") : WGETS("userlist-text-normal"))))));

		gtk_list_store_insert_with_values(nickStore, &iter, userMap.size(),
			nickView.col(N_("Nick")), Nick.c_str(),
			nickView.col(N_("Shared")), shared,
			nickView.col(N_("Description")), params["Description"].c_str(),
			nickView.col(N_("Tag")), params["Tag"].c_str(),
 			nickView.col(N_("Connection")), params["Connection"].c_str(),
			nickView.col("IP"), params["IP"].c_str(),
			nickView.col(N_("eMail")), params["eMail"].c_str(),
			nickView.col(N_("CC")), country.c_str(),
			nickView.col(N_("Exact Share")), shared,
			nickView.col(N_("Slots")), slots.c_str(),
			nickView.col(N_("Hubs")), hubs.c_str(),
			nickView.col("PK"), sup.c_str(),
			nickView.col(N_("Cheat")), cheat.c_str(),
            nickView.col("Generator"), params["FLGEN"].c_str(),
            nickView.col(N_("Support")), params["SUPPORT"].c_str(),
			nickView.col("ClientType"), params["TypeC"].c_str(),
			nickView.col("Country"), buf ,
			nickView.col("Icon"), icon.c_str(),
			nickView.col("Nick Order"), nickOrder.c_str(),
			nickView.col("Favorite"), favorite ? ("f" + nickOrder).c_str() : nickOrder.c_str(),
			nickView.col("CID"), cid.c_str(),
			nickView.col("NickColor"), nickColor.c_str(),
			-1);


		userIters.insert(UserIters::value_type(cid, iter));

		if (BOOLSETTING(SHOW_JOINS))
		{
			// Show joins in chat by default
			addStatusMessage_gui(Nick + _(" has joined"), Msg::STATUS, favorite ? Sound::FAVORITE_USER_JOIN : Sound::NONE);
			string message = Nick + _(" has joined hub ") + client->getHubName();
			WulforManager::get()->getMainWindow()->addPrivateStatusMessage_gui(Msg::STATUS, cid, message);
			#ifdef HAVE_LIBNOTIFY
				if (favorite)
					Notify::get()->showNotify("", message, Notify::FAVORITE_USER_JOIN);
			#endif		
		}
		else if (BOOLSETTING(FAV_SHOW_JOINS) && favorite)
		{
			// Only show joins for favorite users
			string message = Nick + _(" has joined hub ") + client->getHubName();
			addStatusMessage_gui(Nick + _(" has joined"), Msg::STATUS, Sound::FAVORITE_USER_JOIN);
			WulforManager::get()->getMainWindow()->addPrivateStatusMessage_gui(Msg::STATUS, cid, message);
			
			#ifdef HAVE_LIBNOTIFY
				Notify::get()->showNotify("", message, Notify::FAVORITE_USER_JOIN);
			#endif	
		}
	}

	setStatus_gui("statusUsers", Util::toString(userMap.size()) + _(" Users"));
	setStatus_gui("statusShared", Util::formatBytes(totalShared));
}

void Hub::removeUser_gui(string cid)
{
	GtkTreeIter iter;
	string nick, order;

	if (findUser_gui(cid, &iter))
	{
		order = nickView.getString(&iter, "Favorite");
		nick = nickView.getString(&iter, N_("Nick"));
		totalShared -= nickView.getValue<int64_t>(&iter, N_("Shared"));
		gtk_list_store_remove(nickStore, &iter);
		removeTag_gui(nick);
		userMap.erase(nick);
		userIters.erase(cid);
		setStatus_gui("statusUsers", Util::toString(userMap.size()) + _(" Users"));
		setStatus_gui("statusShared", Util::formatBytes(totalShared));

		if (BOOLSETTING(SHOW_JOINS))
		{
			// Show parts in chat by default
			string message = nick + _(" has quit hub ") + client->getHubName();
			addStatusMessage_gui(nick + _(" has quit"), Msg::STATUS, order[0] == 'f'? Sound::FAVORITE_USER_QUIT : Sound::NONE);
			WulforManager::get()->getMainWindow()->addPrivateStatusMessage_gui(Msg::STATUS, cid, message);
		#ifdef HAVE_LIBNOTIFY
			if (order[0] == 'f')
				Notify::get()->showNotify("", message, Notify::FAVORITE_USER_QUIT);
		#endif		
		}
		else if (BOOLSETTING(FAV_SHOW_JOINS) && order[0] == 'f')
		{
			// Only show parts for favorite users
			string message = nick + _(" has quit hub ") + client->getHubName();
			addStatusMessage_gui(nick + _(" has quit"), Msg::STATUS, Sound::FAVORITE_USER_QUIT);
			WulforManager::get()->getMainWindow()->addPrivateStatusMessage_gui(Msg::STATUS, cid, message);
		#ifdef HAVE_LIBNOTIFY	
			Notify::get()->showNotify("", message, Notify::FAVORITE_USER_QUIT);
		#endif	
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
	/*unordered_map<string, string>::const_iterator it;
	for (it = userMap.begin(); it != userMap.end(); ++it)
		removeTag_gui(it->first);
	*/
	gtk_list_store_clear(nickStore);
	userMap.clear();
	userIters.clear();
	totalShared = 0;
	setStatus_gui("statusUsers", _("0 Users"));
	setStatus_gui("statusShared", "0 B");
}


void Hub::popupNickMenu_gui()
{
	// Build user command menu
	userCommandMenu->cleanMenu_gui();

	string nick;
	GtkTreeIter iter;
	GList *list = gtk_tree_selection_get_selected_rows(nickSelection, NULL);

	for (GList *i = list; i; i = i->next)
	{
		GtkTreePath *path = (GtkTreePath *)i->data;
		if (gtk_tree_model_get_iter(GTK_TREE_MODEL(nickStore), &iter, path))
		{
			userCommandMenu->addUser(nickView.getString(&iter, "CID"));
			nick += " "+ nickView.getString(&iter, N_("Nick"));
		}
		gtk_tree_path_free(path);
	}
	g_list_free(list);

	userCommandMenu->addHub(client->getHubUrl());
	userCommandMenu->buildMenu_gui();

	gchar *markup;
	markup = g_markup_printf_escaped ("<span fgcolor=\"blue\" ><b>%s</b></span>", nick.c_str());
	GtkMenuItem *item = GTK_MENU_ITEM(getWidget("ShowNick"));
	///remove events (all) from widget
	WulforUtil::my_gtk_widget_remove_events(GTK_WIDGET(item),GDK_ALL_EVENTS_MASK);
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
			addMessage_gui(line, typemsg);
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

gboolean Hub::onUserListTooltip_gui(GtkWidget *widget, gint x, gint y, gboolean keyboard_tip, GtkTooltip *_tooltip, gpointer data)
{
  Hub *hub = (Hub *)data;
	
  GtkTreeIter iter;
  GtkTreeView *tree_view = GTK_TREE_VIEW (widget);
  GtkTreeModel *model = gtk_tree_view_get_model (tree_view);
  GtkTreePath *path = NULL;
  gchar *tmp;
  gchar *tag;
  gchar *desc;
  gchar *con;
  gchar *ip;
  gchar *e;
  gchar *country;
  gchar *slots;
  gchar *hubs;
  gchar *pk;
  gchar *cheat;
  gchar *gen;
  gchar *sup;
  gchar *cid;
  gchar *pathstring;
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
									20, &cid,
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

void Hub::addMessage_gui(string message, Msg::TypeMsg typemsg)
{
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

	applyTags_gui(line);

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
gboolean Hub::HitIP(string name, string &sIp)
{
	for(int i = 0;i < name.length();i++)
	{
		if(!((name[i] == 0) || (name[i] == '.') || ((name[i] >= '0') && (name[i] <= '9')))) {
			return FALSE;
		}
	}
	
	name += ".";
	size_t begin = 0, pos = string::npos,end = 0;
	bool isOk = true;
	for(int i = 0; i < 4; i++) {
		pos = name.find('.', begin);
		if(pos == tstring::npos) {
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
		sIp = name.substr(0,pos);
		
	}
	return isOk;
}

void Hub::applyTags_gui(const string &line)
{
	GtkTextIter start_iter;
	string ccimage;
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
		bool isNick = FALSE , image_tag = FALSE;
		gchar *temp = gtk_text_iter_get_text(&tag_start_iter, &tag_end_iter);
		gchar *pname = temp; //IP
		
		GtkTextTag *tag = gtk_text_tag_table_lookup(gtk_text_buffer_get_tag_table(chatBuffer), temp);

		if(WGETB("use-highliting"))
		{
			bool isTab = false;
			if(WulforUtil::isHighlitingWorld(chatBuffer,tag,string(temp),isTab,(gpointer)this, TagsMap))
			{
				gtk_text_buffer_apply_tag(chatBuffer, TagsMap[Tag::TAG_HIGHL]/*tag*/, &tag_start_iter, &tag_end_iter);
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
				string order = nickView.getString(&iter, "Favorite");

				if (tagName == client->getMyNick())
					tagStyle = Tag::TAG_MYNICK;
				else if (order[0] == 'f')
					tagStyle = Tag::TAG_FAVORITE;
				else if (order[0] == 'o')
					tagStyle = Tag::TAG_OPERATOR;
				else if (order[0] == 'u')
					tagStyle = Tag::TAG_NICK;

				tagName = tagPrefix + tagName;
			}
			else
			{

			bool notlink = FALSE;
			if(g_ascii_strncasecmp(tagName.c_str(), "[ccc]", 5) == 0)
			{
				string::size_type i = tagName.rfind("[/ccc]");
				if (i != string::npos)
				{
						ccimage = tagName.substr(5, i - 5);
						if(ccimage.length() == 2 )
						{
							notlink = image_tag = TRUE;

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

				tagStyle = Tag::TAG_URL;
			}
			
			if(HitIP(tagName,ip))
			{
				callback = G_CALLBACK(onIpTagEvent_gui);
				tagStyle = Tag::TAG_IPADR;
				isIp = true;
			}	
			
		}
    }
		g_free(temp);

		if(image_tag)
		{

			gtk_text_buffer_move_mark(chatBuffer, tag_mark, &tag_end_iter);
			if(ccimage.length() == 2)
			{
			gtk_text_buffer_delete(chatBuffer, &tag_start_iter, &tag_end_iter);
			GtkTextChildAnchor *anchor = gtk_text_buffer_create_child_anchor(chatBuffer, &tag_start_iter);
			GtkWidget *event_box = gtk_event_box_new();

			// Creating a visible window may cause artifacts that are visible to the user.
			gtk_event_box_set_visible_window(GTK_EVENT_BOX(event_box), FALSE);

			GdkPixbuf *buf = WulforUtil::loadCountry(ccimage);

			GtkWidget *image = gtk_image_new_from_pixbuf (buf);
			gtk_container_add(GTK_CONTAINER(event_box), image);
			gtk_text_view_add_child_at_anchor(GTK_TEXT_VIEW(getWidget("chatText")), event_box, anchor);
			g_object_set_data_full(G_OBJECT(event_box), "CC", g_strdup(ccimage.c_str()), g_free);
			g_signal_connect(G_OBJECT(image), "expose-event", G_CALLBACK(expose), NULL);

			gtk_widget_show_all(event_box);
			#if GTK_CHECK_VERSION(2, 12, 0)
				gtk_widget_set_tooltip_text(event_box, ccimage.c_str());
			#else
				gtk_tooltips_set_tip(tips, event_box, ccimage.c_str(), ccimage.c_str());
			#endif
			}
		}

		if (image_tag)
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
				dcassert(tagStyle >= Tag::TAG_MYNICK && tagStyle < Tag::TAG_LAST);

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

gboolean Hub::expose(GtkWidget *widget, GdkEventExpose *event, gpointer data)
{
	GTK_WIDGET_CLASS(GTK_WIDGET_GET_CLASS(widget))->expose_event(widget, event);
	return true;
}

void Hub::applyEmoticons_gui()
{
	GtkTextIter start_iter, end_iter;

	gtk_text_buffer_get_iter_at_mark(chatBuffer, &start_iter, start_mark);
	gtk_text_buffer_get_iter_at_mark(chatBuffer, &end_iter, end_mark);

	if(gtk_text_iter_equal(&start_iter, &end_iter))
		return;

	/* apply general tag */
	dcassert(tagMsg >= Tag::TAG_GENERAL && tagMsg < Tag::TAG_CHEAT);
	gtk_text_buffer_apply_tag(chatBuffer, TagsMap[tagMsg], &start_iter, &end_iter);

	/* emoticons */
	if (tagMsg == Tag::TAG_SYSTEM || tagMsg == Tag::TAG_STATUS)
	{
		return;
	}
	else if (!Emoticons::get()->useEmoticons_gui())
	{
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
	this->refreshul(NULL, (gpointer)this);//Just for refresh UL colors..
	// resort users
	string sort = BOOLSETTING(SORT_FAVUSERS_FIRST)? "ClientType" : "Nick Order";
	nickView.setSortColumn_gui(N_("Nick"), sort);
	gtk_tree_sortable_set_sort_column_id(GTK_TREE_SORTABLE(nickStore), nickView.col(sort), GTK_SORT_ASCENDING);
	
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
		break;	  

		case Tag::TAG_HIGHL:
			/*fore = "#000000";
			back = "#FFFFFF";
			bold = true; fix
			italic =true;*/
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
		case Tag::TAG_IPADR:
			fore = wsm->getString("text-ip-fore-color");
			back = wsm->getString("text-ip-back-color");
			bold = wsm->getInt("text-ip-bold");
			italic = wsm->getInt("text-ip-italic");
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
	#ifdef HAVE_LIBNOTIFY
		Notify::get()->showNotify("<b>" + client->getHubUrl() + ":</b> ", message, notify);
	#endif
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
/*
 * Implements a case-insensitive substring search for UTF-8 strings.
 */
gboolean Hub::onNickListSearch_gui(GtkTreeModel *model, gint column, const gchar *key, GtkTreeIter *iter, gpointer data)
{
    gboolean result = TRUE;
    gchar *nick;
    gtk_tree_model_get(model, iter, column, &nick, -1);

    gchar *keyCasefold = g_utf8_casefold(key, -1);
    gchar *nickCasefold = g_utf8_casefold(nick, -1);

    // Return false per search equal func API if the key is contained within the nick
    if (g_strstr_len(nickCasefold, -1, keyCasefold) != NULL)//-
		result = FALSE;

    g_free(nick);
    g_free(keyCasefold);
    g_free(nickCasefold);
    return result;
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
				string nick = hub->nickView.getString(&iter, N_("Nick"));
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
			gtk_tree_view_scroll_to_cell(hub->nickView.get(), path, gtk_tree_view_get_column(hub->nickView.get(), hub->nickView.col(N_("Nick"))), FALSE, 0.0, 0.0);
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

void Hub::copyIp(GtkWidget *wid, gpointer data)
{
	Hub *hub = (Hub *)data;
	gtk_clipboard_set_text(gtk_clipboard_get(GDK_SELECTION_CLIPBOARD), hub->ip.c_str(), hub->ip.length());	
}

void Hub::ripeIp(GtkWidget *wid, gpointer data)
{
	Hub *hub = (Hub *)data;
	WulforUtil::openURI("http://www.db.ripe.net/whois?searchtext="+hub->ip+"&searchSubmit=search");
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

			hub->emotdialog->showEmotMenu_gui();
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
		string command = text, param;
		string mess,status;
		bool thirdPerson = false;
		if(WulforUtil::checkCommand(command, param,mess,status,thirdPerson))
		{
			if(!mess.empty())
				hub->sendMessage_client(mess,thirdPerson);

			if(!status.empty())
				hub->addStatusMessage_gui(status,Msg::SYSTEM,Sound::NONE);
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
			WulforManager::get()->dispatchClientFunc(new Func0<Hub>(hub, &Hub::addAsFavorite_client));
		}
		else if (command == "fuser" || command == "fu")
		{
			if (hub->client->getMyNick() == param)
				return;

			if (hub->userMap.find(param) != hub->userMap.end())
			{
				string &cid = hub->userMap[param];
				if (hub->userFavoriteMap.find(cid) == hub->userFavoriteMap.end())
				{
					Func1<Hub, string> *func = new Func1<Hub, string>(hub, &Hub::addFavoriteUser_client, cid);
					WulforManager::get()->dispatchClientFunc(func);
				} else
					hub->addStatusMessage_gui(param + _(" is favorite user"), Msg::STATUS, Sound::NONE);
			} else
				hub->addStatusMessage_gui(_("Not found user: ") + param, Msg::SYSTEM, Sound::NONE);
		}
		else if (command == "removefu" || command == "rmfu")
		{
			if (hub->client->getMyNick() == param)
				return;

			UserMap::const_iterator it = find_if(hub->userFavoriteMap.begin(), hub->userFavoriteMap.end(),
				CompareSecond<string, string>(param));

			if (it != hub->userFavoriteMap.end())
			{
				Func1<Hub, string> *func = new Func1<Hub, string>(hub, &Hub::removeFavoriteUser_client, it->first);
				WulforManager::get()->dispatchClientFunc(func);
			}
			else
				hub->addStatusMessage_gui(param + _(" is not favorite user"), Msg::STATUS, Sound::NONE);
		}
		else if (command == "listfu" || command == "lsfu")
		{
			string list;
			for (UserMap::const_iterator it = hub->userFavoriteMap.begin(); it != hub->userFavoriteMap.end(); ++it)
			{
				list += " " + it->second;
			}
			hub->addMessage_gui(_("User favorite list:") + (list.empty()? list = _(" empty...") : list), Msg::SYSTEM);
		}
		else if (command == "getlist")
		{
			if (hub->userMap.find(param) != hub->userMap.end())
			{
				func2 = new F2(hub, &Hub::getFileList_client, hub->userMap[param], FALSE);
				WulforManager::get()->dispatchClientFunc(func2);
			}
			else
				hub->addStatusMessage_gui(_("Not found user: ") + param, Msg::SYSTEM, Sound::NONE);
		}
		else if (command == "grant")
		{
			if (hub->userMap.find(param) != hub->userMap.end())
			{
				func = new F1(hub, &Hub::grantSlot_client, hub->userMap[param]);
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
		else if (command == "help")
		{
			hub->addMessage_gui(string(_("*** Available commands:")) + "\n\n" +
			"/away <message>\t\t - "	+ _("Away mode message on/off") + "\n" +
			"/back\t\t\t\t - " 			+ _("Away mode off") + "\n" +
			"/clear\t\t\t\t - " 		+ _("Clear chat") + "\n" +
			"/close\t\t\t\t - " 		+ _("Close chat") + "\n" +
			"/password pass\t\t\t" 		+ _("send password") + "\n" +
			"/favorite, /fav\t\t\t - " 	+ _("Add a hub to favorites") + "\n" +
			"/fuser, /fu <nick>\t\t - " + _("Add user to favorites list") + "\n" +
			"/removefu, /rmfu <nick>\t - " + _("Remove user favorite") + "\n" +
			"/listfu, /lsfu\t\t\t - " 	+ _("Show favorites list") + "\n" +
			"/getlist <nick>\t\t\t - " 	+ _("Get file list") + "\n" +
			"/grant <nick>\t\t\t - " 	+ _("Grant extra slot") + "\n" +
			"/help\t\t\t\t - " 			+ _("Show help") + "\n" +
			"/join <address>\t\t - " 	+ _("Connect to the hub") + "\n" +
			"/me <message>\t\t - " 		+ _("Say a third person") + "\n" +
			"/pm <nick> <text>\t\t - " 	+ _("Private message") + "\n" +
			"/rebuild\t\t\t\t - " 		+ _("Rebuild hash") + "\n" +
			"/refresh\t\t\t\t - " 		+ _("Update own file list") + "\n" +
			"/userlist\t\t\t\t - " 		+ _("User list show/hide") + "\n" +
			"/bmdc [mc]\t\t\t - "   	+ _("Show version") + "\n" +
			"/emoticons, /emot\t\t - " 	+ _("Emoticons on/off") + "\n" +
			#ifdef _USELUA
			"/luafile <file>\t\t - " 	+ _("Load Lua file") + "\n" +
			"/lua <chunk>\t\t\t -  " 	+ _("Execute Lua Chunk") + "\n" +
			#endif
			"/uptime \t\t\t\t\t\t -  " 	+ _("Show Client Uptime") + "\n" +
			"/df [mc] \t\t\t\t -  "		+ _("Show Free space (mainchat)") + "\n" +
			"/w ,/auda, /kaff, /amar\t"	+ _("Media Spam") + "\n" +
			"/stats \t\t\t\t - " 		+ _("Stats Clients") + "\n" +
			"/exec \t\t\t\t  - " 		+ _("Execute code (bash)") + "\n" +
			"/slots [n]\t\t\t\t"		+ _("Set Uploads slots") + "\n" +
			"/ratio [mc]\t\t\t\t"		+ _("Show ratio (mainchat)") + "\n" +
			"/alias list\t\t\t" 		+ _("Alias List") + "\n"
			"/alias purge ::A\t\t"		+ _("Alias Remove A") + "\n"
			"/alias A::uname -a" 		+ _("Alias add uname -a as A") + "\n" +
			"/A\t\t\t\t\t\t\t\t\t" 		+ _("Alias A executing") + "\n" +
			"/sc\t\t\t\t\t\t\t\t"		+ _("Start checkers") + "\n" +
			"/ulrefresh\t\t\t\t  "		+ _("Refresh UserList") + "\n" +
			"/leech\t\t\t\t"			+ _("Show Leech Info") + "\n" +
			"/topic\t\t\t\t"			+ _("Show topic text in chat") + "\n" +
			"/cleanmc\t\t\t\t"			+ _("Clean Mainchat (sended to Hub)") + "\n" +
			"/ws [name-of-set-in-file] [value]\t"      + _("Set GUI Setting (all)") + "\n" +
			"/dcpps [name-of-set-in-file] [value]\t" + _("Set dcpp kernel setting") + "\n"
			 , Msg::SYSTEM);
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
			func2 = new F2(hub, &Hub::sendMessage_client, param, true);
			WulforManager::get()->dispatchClientFunc(func2);
		}
		else if (command == "pm")
		{
			/* pokud neni /pm nick text , melo by otevrit okno z konverzaci bez napsani :)*/
			string::size_type j = param.find(" ");


				if(j != string::npos)	 {
					tstring nick = param.substr(0, j);
					UserPtr ui = ClientManager::getInstance()->getUser(nick,"");

					if	(ui)
					{
						if(param.size() > j+1)
						{
							WulforManager::get()->getMainWindow()->addPrivateMessage_gui(Msg::PRIVATE, hub->userMap[nick], "" ,param.substr(j+1),false);
						}
						else
						{
							WulforManager::get()->getMainWindow()->addPrivateMessage_gui(Msg::UNKNOWN, hub->userMap[nick], hub->client->getHubUrl(),"",false);
						}

					}
					else
					{
						hub->addStatusMessage_gui(_("User not found"), Msg::SYSTEM, Sound::NONE);
					}

				}
				else
				{
					hub->addStatusMessage_gui(_("User not found"), Msg::SYSTEM, Sound::NONE);

				}
		}
		else if ( command == "topic")
		{
			hub->addMessage_gui(_("Topic: ")+hub->client->getHubDescription(), Msg::SYSTEM);
		}	  
		else if ( command == "sc")
		{
		//Checker
			string det=hub->client->startChecking(param);
			if(!det.empty())
				hub->addMessage_gui(det, Msg::SYSTEM);

		}
		else if (command == "ulrefresh")
		{
			hub->clearNickList_gui();
			hub->client->refreshUserList(true);
		}

		else if (command == "userlist")
		{
			if (GTK_WIDGET_VISIBLE(hub->getWidget("scrolledwindow2")))
				gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(hub->getWidget("userListCheckButton")), FALSE);
			else
				gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(hub->getWidget("userListCheckButton")), TRUE);
		}
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
		#ifdef _USELUA
		else if (command == "lua" ) {
			ScriptManager::getInstance()->EvaluateChunk(Text::fromT(param));
		}
		else if( command == "luafile") {
			ScriptManager::getInstance()->EvaluateFile(Text::fromT(param));
		}
		#endif
		// protect command
		else if (command == "password")
		{
			if (!hub->WaitingPassword)
				return;

			F1 *func = new F1(hub, &Hub::setPassword_client, param);
			WulforManager::get()->dispatchClientFunc(func);
			hub->WaitingPassword = FALSE;
		}
	}
	else
	{
        #ifdef _USELUA
		if(!dropMessage)
		{
		#endif
            if(BOOLSETTING(SEND_UNKNOWN_COMMANDS))
            {
                func2 = new F2(hub, &Hub::sendMessage_client, text, false);
                WulforManager::get()->dispatchClientFunc(func2);
            }
            else
				hub->addStatusMessage_gui(_("Unknown command '") + text + _("': type /help for a list of available commands"), Msg::SYSTEM, Sound::NONE);
        #ifdef _USELUA
		}
		else
			hub->addStatusMessage_gui(_("Unknown command '") + text + _("': type /help for a list of available commands"), Msg::SYSTEM, Sound::NONE);
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
				nicks += hub->nickView.getString(&iter, N_("Nick")) + ", ";

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
/*
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
				nicks += hub->nickView.getString(&iter, N_("Nick")) + ' ';
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
}*/
//NOTE:Patched is this used ?
/*void Hub::onCopyTag_gui(GtkMenuItem *item, gpointer data)
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
				nicks += hub->nickView.getString(&iter, _("Tag")) + ' ';
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
*/
void Hub::onUserInfo_gui(GtkMenuItem *item, gpointer data)
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

		hub->addMessage_gui(WulforUtil::getReport(id), Msg::SYSTEM);

	}
}
//End
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

	WulforUtil::openURI(hub->selectedTagStr);
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

void Hub::onAddIgnItemClicked_gui(GtkMenuItem *item, gpointer data)
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

void Hub::onRemoveIgnItemClicked_gui(GtkMenuItem *item, gpointer data)
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
//Test SUR
void Hub::onTestSUR_gui(GtkMenuItem *item, gpointer data)
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
				nicks = hub->nickView.getString(&iter, _("CID"));
			}
			gtk_tree_path_free(path);
		}
		g_list_free(list);

		if (!nicks.empty())
		{
			OnlineUser *ou = ClientManager::getInstance()->findOnlineUser(CID(nicks),hub->address,false);
			if(ou != NULL)
			{	  
				try {
					HintedUser hintedUser(ou->getUser(), hub->address); 
					ClientManager::getInstance()->addCheckToQueue(hintedUser, false);
				}catch(...)
				{ }
			}	 
		}
	}
}
//End
//check FL
void Hub::onCheckFL(GtkMenuItem *item , gpointer data)
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
				nicks = hub->nickView.getString(&iter, _("CID"));
			}
			gtk_tree_path_free(path);
		}
		g_list_free(list);

		if (!nicks.empty())
		{
			OnlineUser *ou = ClientManager::getInstance()->findOnlineUser(CID(nicks),hub->address,false);
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
//Protect Users
void Hub::onProtect(GtkMenuItem *item , gpointer data)
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
			if(ou->getUser() && !ou->getUser()->isSet(User::PROTECTED))
				const_cast<UserPtr&>(ou->getUser())->setFlag(User::PROTECTED);
			ParamMap params;
			hub->getParams_client(params, ou->getIdentity());
			hub->AddProtectUser(params);
		}
	}
}

void Hub::onUnProtect(GtkMenuItem *item , gpointer data)
{
	Hub *hub =(Hub *)data;
	if (gtk_tree_selection_count_selected_rows(hub->nickSelection) == 1)//think
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
			if(ou->getUser() && !ou->getUser()->isSet(User::PROTECTED))
				const_cast<UserPtr&>(ou->getUser())->unsetFlag(User::PROTECTED);
			ParamMap params;
			hub->getParams_client(params, ou->getIdentity());
			hub->AddProtectUser(params);
		}
	}
}
//END
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
				nick = hub->nickView.getString(&iter, N_("Nick"));
				order = hub->nickView.getString(&iter, "Favorite");

				if (!cid.empty() && nick != hub->client->getMyNick())
				{
					if (order[0] == 'o' || order[0] == 'u')
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
				nick = hub->nickView.getString(&iter, N_("Nick"));
				order = hub->nickView.getString(&iter, "Favorite");

				if (!cid.empty() && nick != hub->client->getMyNick())
				{
					if (order[0] == 'f')
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
				nickView.col("Favorite"), ("f" + params["Order"] + nick).c_str(),
				nickView.col("NickColor"), /*"#ff0000"*/WGETS("userlist-text-favorite").c_str(),
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
			string nickOrder = nickView.getString(&iter, "Nick Order");
			gtk_list_store_set(nickStore, &iter,
				nickView.col("Favorite"), nickOrder.c_str(),
				nickView.col("NickColor"), WGETS("userlist-text-normal").c_str()/*"#000000"*/,
				-1);
			removeTag_gui(nick);
		}

		string message = nick + _(" removed from favorites list");
		addStatusMessage_gui(message, Msg::STATUS, Sound::NONE);
		WulforManager::get()->getMainWindow()->addPrivateStatusMessage_gui(Msg::STATUS, cid, message);
	}
}
/**/
void Hub::addOp(ParamMap params)
{
	const string &cid = params["CID"];
	if (userOPMap.find(cid) == userOPMap.end())
	{
		GtkTreeIter iter;
		const string &nick = params["Nick"];

		userOPMap.insert(UserMap::value_type(cid,nick));

		if( findUser_gui(cid, &iter))
		{
			gtk_list_store_set(nickStore,&iter,
					nickView.col("NickColor"),/*"#1E90FF"*/WGETS("userlist-text-operator").c_str(),
					-1);
			removeTag_gui(nick);
		}
	}
}

void Hub::addPasive(ParamMap params)
{
	const string &cid = params["CID"];
	if (userPasiveMap.find(cid) == userPasiveMap.end())
	{
		GtkTreeIter iter;
		const string &nick = params["Nick"];

		userPasiveMap.insert(UserMap::value_type(cid,nick));

		if( findUser_gui(cid, &iter))
		{
			gtk_list_store_set(nickStore,&iter,
					nickView.col("NickColor"),/*"#747677"*/WGETS("userlist-text-pasive").c_str(),
					-1);
			removeTag_gui(nick);
		}
	}
}

void Hub::addIgnore(ParamMap params)
{
	const string &cid = params["CID"];

	if (userIgnoreMap.find(cid) == userIgnoreMap.end())
	{
		GtkTreeIter iter;
		const string &nick = params["Nick"];

		userIgnoreMap.insert(UserMap::value_type(cid,nick));

		if( findUser_gui(cid, &iter))
		{
			gtk_list_store_set(nickStore,&iter,
					nickView.col("NickColor"),/*"#9affaf"*/WGETS("userlist-text-ignored").c_str(),
					-1);
			removeTag_gui(nick);
		}
	}
}

void Hub::AddProtectUser(ParamMap params)
{
	const string &cid = params["CID"];
	if(userProtect.find(cid) == userProtect.end())
	{
		GtkTreeIter iter;
		const string &nick = params["Nick"];
		userProtect.insert(UserMap::value_type(cid,nick));
		if( findNick_gui(cid,&iter))
		{
			gtk_list_store_set(nickStore,&iter,
						nickView.col("NickColor"), WGETS("userlist-text-protected").c_str()/*"#8B6914*/,
						-1);
		removeTag_gui(nick);

		}

	}
}
void Hub::delOp(ParamMap params)
{
	const string &cid = params["CID"];
	if(userOPMap.find(cid) != userOPMap.end())
	{

		GtkTreeIter iter;
		const string &nick = params["Nick"];
		userOPMap.erase(cid);
		if(findUser_gui(cid,&iter))
		{
			gtk_list_store_set(nickStore, &iter,
				nickView.col("NickColor"), /*"#000000"*/WGETS("userlist-text-normal").c_str(),
				-1);
			removeTag_gui(nick);
		}
	}


}
void Hub::delPasive(ParamMap params)
{
	const string &cid = params["CID"];
	if(userPasiveMap.find(cid) != userPasiveMap.end())
	{

		GtkTreeIter iter;
		const string &nick = params["Nick"];
		userPasiveMap.erase(cid);
		if(findUser_gui(cid,&iter))
		{
			gtk_list_store_set(nickStore, &iter,
				nickView.col("NickColor"), /*"#000000"*/WGETS("userlist-text-normal").c_str(),
				-1);
			removeTag_gui(nick);
		}
	}
}

void Hub::delIgnore(ParamMap params)
{
	const string &cid = params["CID"];
	if(userIgnoreMap.find(cid) != userIgnoreMap.end())
	{

		GtkTreeIter iter;
		const string &nick = params["Nick"];
		userIgnoreMap.erase(cid);
		if(findUser_gui(cid,&iter))
		{
			gtk_list_store_set(nickStore, &iter,
				nickView.col("NickColor"), /*"#000000"*/WGETS("userlist-text-normal").c_str(),
				-1);
			removeTag_gui(nick);
		}
	}

}
/* end */
void Hub::addPrivateMessage_gui(Msg::TypeMsg typemsg, string CID, string cid, string url, string message, bool useSetting)
{
	if (userFavoriteMap.find(CID) != userFavoriteMap.end())
		typemsg = Msg::FAVORITE;

	WulforManager::get()->getMainWindow()->addPrivateMessage_gui(typemsg, cid, url, message, useSetting);
}

void Hub::addStatusPrivateMessage_gui(string cid, string message)
{

	WulforManager::get()->getMainWindow()->addPrivateStatusMessage_gui(Msg::SYSTEM, cid , message);

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
}

void Hub::disconnect_client()
{
	if (client)
	{
		FavoriteManager::getInstance()->removeListener(this);
		client->removeListener(this);
		client->disconnect(true);
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
			error+=address;
			typedef Func2<Hub, string, Msg::TypeMsg> F3;//st
			F3 *f3 = new F3(this, &Hub::addMessage_gui, error, Msg::SYSTEM);
			WulforManager::get()->dispatchGuiFunc(f3);
			return;
		}

		string fadrero = _("Address  => ");
		fadrero += address;
		typedef Func2<Hub, string, Msg::TypeMsg> F2;
		F2 *func2 = new F2(this,&Hub::addMessage_gui,fadrero,Msg::SYSTEM);
		WulforManager::get()->dispatchGuiFunc(func2);

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

void Hub::addAsFavorite_client()
{
	///@
 	typedef Func4<Hub, string, Msg::TypeMsg, Sound::TypeSound, Notify::TypeNotify> F4;
 	F4 *func;

 	FavoriteHubEntry *existingHub = FavoriteManager::getInstance()->getFavoriteHubEntry(client->getHubUrl());

 	if (!existingHub)
 	{
 		FavoriteHubEntry aEntry;
 		aEntry.setServer(client->getHubUrl());
 		aEntry.setName(client->getHubName());
 		aEntry.setDescription(client->getHubDescription());
 		aEntry.setNick(client->getMyNick());
 		aEntry.setEncoding(encoding);
 		aEntry.setGroup("");
 		if(client->getPassword().size() > 0)  {
            aEntry.setPassword(client->getPassword());
        }   
 		FavoriteManager::getInstance()->addFavorite(aEntry);
 		func = new F4(this, &Hub::addStatusMessage_gui, _("Favorite hub added"), Msg::STATUS, Sound::NONE,Notify::NONE);
 		WulforManager::get()->dispatchGuiFunc(func);
 	}
 	else
 	{
 		func = new F4(this, &Hub::addStatusMessage_gui, _("Favorite hub already exists"), Msg::STATUS, Sound::NONE,Notify::NONE);
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

void Hub::refreshul(GtkWidget *widget , gpointer data)
{
    Hub *hub = (Hub *)data;
    hub->clearNickList_gui();
    hub->client->refreshUserList(true);
}
/*Inspipred by code UserInfoBase getImageIndex*/
string Hub::getConn(const Identity& id)
{
	string tmp = "other";
	if(id.isOp()) {
		 tmp = "op";
	} else /*status*/
	{
		string conn = id.getConnection();
		if(	(conn == "28.8Kbps") || (conn == "33.6Kbps") ||	(conn == "56Kbps") || (conn == "Modem") ||	(conn == "ISDN")) {
				tmp =  "modem";
			} else if(	(conn == "Satellite") ||(conn == "Microwave") ||(conn == "Wireless")) {
				tmp = "wireless";
			} else if(	(conn == "DSL") ||	(conn == "Cable")) {
				tmp = "dsl";
			} else if(	(strncmp(conn.c_str(), "LAN", 3) == 0)) {
				tmp = "lan";
			} else if( (strncmp(conn.c_str(), "NetLimiter", 10) == 0)) {
				tmp = "netlimiter";
			} else {

				double us = conn.empty() ? (8 * Util::toDouble(id.get("US")) / 1024 / 1024): Util::toDouble(conn);
				if(us >= 10) {
					tmp = "ten";
				} else if(us > 0.1) {
					tmp = "zeroone";//temped;
				} else if(us >= 0.01) {
					tmp = "zerozeroone";
				} else if(us > 0) {
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
	string icon = getConn(id);
	params.insert(ParamMap::value_type("Icon",icon));

	if (id.isOp())
	{
		params.insert(ParamMap::value_type("Nick Order", "o" + id.getNick()));
	}
	else
	{
		params.insert(ParamMap::value_type("Nick Order", "u" + id.getNick()));
	}

	params.insert(ParamMap::value_type("Nick", id.getNick()));
	params.insert(ParamMap::value_type("Shared", Util::toString(id.getBytesShared())));
	params.insert(ParamMap::value_type("Description", id.getDescription()));
	params.insert(ParamMap::value_type("Tag", id.getTag()));
	params.insert(ParamMap::value_type("Connection", id.getConnection()));
	params.insert(ParamMap::value_type("IP", id.getIp()));
	params.insert(ParamMap::value_type("eMail", id.getEmail()));
	params.insert(ParamMap::value_type("CID", id.getUser()->getCID().toBase32()));
	const string cn = Util::toString(Util::toInt(id.get("HN")) + Util::toInt(id.get("HR")) + Util::toInt(id.get("HO")));//hubs
	params.insert(ParamMap::value_type("Hubs", cn )); //Hubs
	params.insert(ParamMap::value_type("Slots", id.get("SL"))); //Slots
	
	#ifndef _DEBUG
        params.insert(ParamMap::value_type("Country", Util::getIpCountry(id.getIp())));
	#else
        params.insert(ParamMap::value_type("Country", Util::getIpCountry("0.0.0.0")));
	#endif
	//add
//	params.insert(ParamMap::value_type("Version", id.get("VE")));
//	params.insert(ParamMap::value_type("Mode", id.isTcpActive() ? "A" : "P"  ));

	params.insert(ParamMap::value_type("SUP", id.get("PK")));
	params.insert(ParamMap::value_type("Cheat", id.get("CS")));

    params.insert(ParamMap::value_type("FLGEN", id.get("GE")));

    params.insert(ParamMap::value_type("SUPPORT", id.get("SU")));

	if(id.isBot() || id.isHub())
		params.insert(ParamMap::value_type("TypeC", "BOT" + id.getNick()));
	else if(id.isOp())
		params.insert(ParamMap::value_type("TypeC", "COP" + id.getNick()));
	else if(FavoriteManager::getInstance()->isFavoriteUser(id.getUser()))
		params.insert(ParamMap::value_type("TypeC", "F" + id.getNick()));
	else
		params.insert(ParamMap::value_type("TypeC", "U" + id.getNick()));

}

void Hub::on(FavoriteManagerListener::UserAdded, const FavoriteUser &user) throw()
{
	if (user.getUrl() != client->getHubUrl())
		return;

	ParamMap params;
	params.insert(ParamMap::value_type("Nick", user.getNick()));
	params.insert(ParamMap::value_type("CID", user.getUser()->getCID().toBase32()));
	params.insert(ParamMap::value_type("Order", ClientManager::getInstance()->isOp(user.getUser(), user.getUrl()) ? "o" : "u"));

	if(FavoriteManager::getInstance()->isIgnoredUser(user.getUser()))
	{
		Func1<Hub, ParamMap> *func = new Func1<Hub, ParamMap>(this, &Hub::addIgnore, params);
		WulforManager::get()->dispatchGuiFunc(func);
		return;
	}
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
	
	if(FavoriteManager::getInstance()->isIgnoredUser(user.getUser()))
	{
		Func1<Hub, ParamMap> *func = new Func1<Hub, ParamMap>(this, &Hub::delIgnore, params);
		WulforManager::get()->dispatchGuiFunc(func);
	}
}

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
		Func1<Hub, ParamMap> *func = new Func1<Hub, ParamMap>(this, &Hub::updateUser_gui, params);
		WulforManager::get()->dispatchGuiFunc(func);

		//Patch
		string message = params["Nick"] + _( " is online");
		string cid = params["CID"];
		//end
		Func2<Hub,string,string> *func1 = new Func2<Hub, string , string>(this,&Hub::addStatusPrivateMessage_gui, cid, message);
		WulforManager::get()->dispatchGuiFunc(func1);
		/* new */
		if(id.isOp() || id.isHub() || id.isBot())
		{
			ParamMap params;
			getParams_client(params,id);
			Func1<Hub, ParamMap> *func2 = new Func1<Hub, ParamMap>(this, &Hub::addOp, params);
			WulforManager::get()->dispatchGuiFunc(func2);
		}

		if(id.getUser()->isSet(User::PASSIVE))
		{
			ParamMap params;
			getParams_client(params,id);
			Func1<Hub, ParamMap> *func3 = new Func1<Hub, ParamMap>(this, &Hub::addPasive, params);
			WulforManager::get()->dispatchGuiFunc(func3);
		}
		if(id.getUser()->isSet(User::PROTECTED))
		{
			ParamMap params;
			getParams_client(params,id);
			Func1<Hub, ParamMap> *func3 = new Func1<Hub, ParamMap>(this, &Hub::AddProtectUser, params);
			WulforManager::get()->dispatchGuiFunc(func3);

		}
	}
	//new
	if(id.isOp() || id.isHub())
	{
		ParamMap params;
		getParams_client(params,id);
		Func1<Hub, ParamMap> *func2 = new Func1<Hub, ParamMap>(this, &Hub::addOp, params);
		WulforManager::get()->dispatchGuiFunc(func2);
	}

	if(id.getUser()->isSet(User::PASSIVE))
	{
		ParamMap params;
		getParams_client(params,id);
		Func1<Hub, ParamMap> *func3 = new Func1<Hub, ParamMap>(this, &Hub::addPasive, params);
		WulforManager::get()->dispatchGuiFunc(func3);
	}

	if(id.getUser()->isSet(User::PROTECTED))
	{
			ParamMap params;
			getParams_client(params,id);
			Func1<Hub, ParamMap> *func3 = new Func1<Hub, ParamMap>(this, &Hub::AddProtectUser, params);
			WulforManager::get()->dispatchGuiFunc(func3);
	}

	if(FavoriteManager::getInstance()->isIgnoredUser(id.getUser()))
	{
		ParamMap params;
		getParams_client(params,id);
		Func1<Hub, ParamMap> *func4 = new Func1<Hub, ParamMap>(this, &Hub::addIgnore, params);
		WulforManager::get()->dispatchGuiFunc(func4);
	}
	//end
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
			//
			if(id.isOp() || id.isHub() || id.isBot())
			{
				F1 *func2 = new F1(this,&Hub::addOp,params);
				WulforManager::get()->dispatchGuiFunc(func2);
			}

			if(id.getUser()->isSet(User::PASSIVE))
			{
				F1 *func2 = new F1(this, &Hub::addPasive, params);
				WulforManager::get()->dispatchGuiFunc(func2);
			}

			if(id.getUser()->isSet(User::PROTECTED))
			{
			Func1<Hub, ParamMap> *func3 = new Func1<Hub, ParamMap>(this, &Hub::AddProtectUser, params);
			WulforManager::get()->dispatchGuiFunc(func3);
			}
			//end
		}
		//new
			ParamMap params;
			getParams_client(params,id);
			if(id.isOp() || id.isHub() )
			{
				F1 *func2 = new F1(this,&Hub::addOp,params);
				WulforManager::get()->dispatchGuiFunc(func2);

			}

			if(id.getUser()->isSet(User::PASSIVE))
			{
				F1 *func2 = new F1(this, &Hub::addPasive, params);
				WulforManager::get()->dispatchGuiFunc(func2);
			}

			if(id.getUser()->isSet(User::PROTECTED))
			{
			Func1<Hub, ParamMap> *func3 = new Func1<Hub, ParamMap>(this, &Hub::AddProtectUser, params);
			WulforManager::get()->dispatchGuiFunc(func3);
			}

			if(FavoriteManager::getInstance()->isIgnoredUser(id.getUser()))
			{
				ParamMap params;
				getParams_client(params,id);
				Func1<Hub, ParamMap> *func4 = new Func1<Hub, ParamMap>(this,&Hub::addIgnore, params);
				WulforManager::get()->dispatchGuiFunc(func4);
			}
		//end
	}
}

void Hub::on(ClientListener::UserRemoved, Client *, const OnlineUser &user) throw()
{
	Func1<Hub, string> *func = new Func1<Hub, string>(this, &Hub::removeUser_gui, user.getUser()->getCID().toBase32());
	WulforManager::get()->dispatchGuiFunc(func);

	//Patched
	string message = user.getIdentity().getNick() + _( " is offline");
	string cid = user.getUser()->getCID().toBase32();
	ParamMap params;
	params["CID"] = cid;
	params["Nick"] = user.getIdentity().getNick();

	if (user.getIdentity().isOp())
	{
		Func1<Hub,ParamMap> *func1 = new Func1<Hub, ParamMap>(this, &Hub::delOp, params);
		WulforManager::get()->dispatchGuiFunc(func1);
	}

	if (user.getIdentity().getUser()->isSet(User::PASSIVE))
	{
		Func1<Hub,ParamMap> *func1 = new Func1<Hub, ParamMap>(this, &Hub::delPasive, params);
		WulforManager::get()->dispatchGuiFunc(func1);
	}

	//end
	Func2<Hub,string,string> *func1 = new Func2<Hub, string , string>(this, &Hub::addStatusPrivateMessage_gui, cid, message);
	WulforManager::get()->dispatchGuiFunc(func1);

}

void Hub::on(ClientListener::Redirect, Client *, const string &address) throw()
{
	// redirect_client() crashes unless I put it into the dispatcher (why?)
	typedef Func2<Hub, string, bool> F2;
	F2 *func = new F2(this, &Hub::redirect_client, address, BOOLSETTING(AUTO_FOLLOW));
	WulforManager::get()->dispatchClientFunc(func);
}

void Hub::on(ClientListener::Failed, Client *, const string &reason) throw()
{
	Func0<Hub> *f0 = new Func0<Hub>(this, &Hub::clearNickList_gui);
	WulforManager::get()->dispatchGuiFunc(f0);

	typedef Func4<Hub, string, Msg::TypeMsg, Sound::TypeSound, Notify::TypeNotify> F4;
	F4 *f4 = new F4(this, &Hub::addStatusMessage_gui, _("Connect failed: ") + reason, Msg::SYSTEM, Sound::HUB_DISCONNECT, Notify::HUB_DISCONNECT);
	WulforManager::get()->dispatchGuiFunc(f4);
}

void Hub::on(ClientListener::GetPassword, Client *) throw()
{
	if (!client->getPassword().empty())
	{
		client->password(client->getPassword());
		//
		typedef Func4<Hub, string,Msg::TypeMsg, Sound::TypeSound, Notify::TypeNotify> F4;
		F4 *func4 = new F4(this, &Hub::addStatusMessage_gui, _("Send Stored password. "), Msg::SYSTEM, Sound::NONE, Notify::NONE);
		WulforManager::get()->dispatchGuiFunc(func4);

	}
	else
	{
		Func0<Hub> *func = new Func0<Hub>(this, &Hub::getPassword_gui);
		WulforManager::get()->dispatchGuiFunc(func);
	}
}

void Hub::on(ClientListener::HubUpdated, Client *) throw()
{
	typedef Func1<Hub, string> F1;
	string hubName = "";

	if (client->getHubName().empty())
		hubName += client->getAddress() + ":" + Util::toString(client->getPort());
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
		string cc = Util::getIpCountry(aIp);
		bool showIp = BOOLSETTING(USE_IP) || sIp;
		bool showCc = (BOOLSETTING(USE_COUNTRY) || sCC) && !cc.empty();
		bool useFlagIcons = (WGETB("use-flag") && !isPm && !cc.empty());

		if(showIp) {
			ret = "[ " + aIp + " ] ";
		}
		
		if(showCc) {
			ret += "[" + cc + "] ";
		}
		
		if(useFlagIcons) {
			ret += " [ccc]" + cc + "[/ccc] ";
		}

	}
	return Text::toT(ret);
}
/* END */

void Hub::on(ClientListener::Message, Client*, const ChatMessage& message) throw() //NOTE: core 0.762
{
	if (message.text.empty())
		return;

	Msg::TypeMsg typemsg;
	string line;

	if( (!message.from->getIdentity().isHub()) && (!message.from->getIdentity().isBot()) )
	{

		string info;
		string extraInfo;
		if((!(message.from->getUser() == client->getMyIdentity().getUser())) || client->getMyIdentity().isOp())
			info = formatAdditionalInfo(message.from->getIdentity().getIp(), BOOLSETTING(USE_IP), BOOLSETTING(USE_COUNTRY), message.to && message.replyTo);
		else info = "";

		//Extra Info

		StringMap params;
		params["hubURL"] = client->getHubUrl();
		client->getHubIdentity().getParams(params, "hub", false);
		client->getMyIdentity().getParams(params, "my", true);
		message.from->getIdentity().getParams(params, "user", true);
		extraInfo = Text::toT(Util::formatParams(client->getChatExtraInfo(), params, false));
		info += extraInfo;

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
			size_t nend = message.text.find_last_of (" ",nestle);  
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
		string error = _("Ignored PM/Chat message from User ")+ message.from->getIdentity().getNick();
		error += _("\nMessage: ")+message.text+"\n";

		StringMap params;
		params["message"] = error;

		if(WGETB("log-messages"))
            LOG(LogManager::SYSTEM,params);

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
			if(FavoriteManager::getInstance()->isFavoriteUser(message.from->getUser()) && WGETB("only-fav") || message.from->getIdentity().isOp())
			{
				typedef Func6<Hub, Msg::TypeMsg, string, string, string, string, bool> F6;
				F6 *func = new F6(this, &Hub::addPrivateMessage_gui, typemsg, message.from->getUser()->getCID().toBase32(),
				user->getUser()->getCID().toBase32(), client->getHubUrl(), line, TRUE);
				WulforManager::get()->dispatchGuiFunc(func);
			}
			else if(!WGETB("only-fav"))
			{
				typedef Func6<Hub, Msg::TypeMsg, string, string, string, string, bool> F6;
				F6 *func = new F6(this, &Hub::addPrivateMessage_gui, typemsg, message.from->getUser()->getCID().toBase32(),
				user->getUser()->getCID().toBase32(), client->getHubUrl(), line, TRUE);
				WulforManager::get()->dispatchGuiFunc(func);
		    }
		    else
		    {
				error = _("Ignored private message from User what is not in Fav Users ") + user->getIdentity().getNick();
				error +=  _("\nMessage ") + line;
				typedef Func3<Hub, string, Msg::TypeMsg, Sound::TypeSound> F3;
				F3 *func = new F3(this, &Hub::addStatusMessage_gui, error, Msg::STATUS, Sound::NONE);
				WulforManager::get()->dispatchGuiFunc(func);
			}
		}
	}
	else
	{
		 // chat message

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

		if (BOOLSETTING(LOG_MAIN_CHAT) || logChat)
		{
			StringMap params;
			params["message"] = line;
			client->getHubIdentity().getParams(params, "hub", false);
			params["hubURL"] = client->getHubUrl();
			client->getMyIdentity().getParams(params, "my", true);
			LOG(LogManager::CHAT, params);
		}

		typedef Func2<Hub, string, Msg::TypeMsg> F2;
		F2 *func = new F2(this, &Hub::addMessage_gui, line, typemsg);
		WulforManager::get()->dispatchGuiFunc(func);

		// Set urgency hint if message contains user's nick
		if (BOOLSETTING(BOLD_HUB) && message.from->getIdentity().getUser() != client->getMyIdentity().getUser())
		{
			if (message.text.find(client->getMyIdentity().getNick()) != string::npos)
			{
				typedef Func0<Hub> F0;
				F0 *func = new F0(this, &Hub::setUrgent_gui);
				WulforManager::get()->dispatchGuiFunc(func);
			}
		}
	}
} //NOTE: core 0.762

void Hub::on(ClientListener::StatusMessage, Client *, const string &message, int  flag ) throw()
{
	if (!message.empty())
	{
		if (BOOLSETTING(FILTER_MESSAGES) && flag == ClientListener::FLAG_IS_SPAM)
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
			StringMap params;
			client->getHubIdentity().getParams(params, "hub", FALSE);
			params["hubURL"] = client->getHubUrl();
			client->getMyIdentity().getParams(params, "my", TRUE);
			params["message"] = message;
			LOG(LogManager::STATUS, params);
		}

		typedef Func2<Hub, string, Msg::TypeMsg> F2;
		F2 *func = new F2(this, &Hub::addMessage_gui, message, Msg::STATUS);
		WulforManager::get()->dispatchGuiFunc(func);
	}
}

void Hub::on(ClientListener::NickTaken, Client *) throw()
{
	typedef Func3<Hub, string, Msg::TypeMsg, Sound::TypeSound> F3;
	F3 *func = new F3(this, &Hub::addStatusMessage_gui, _("Nick already taken"), Msg::STATUS, Sound::NONE);
	WulforManager::get()->dispatchGuiFunc(func);
}

void Hub::on(ClientListener::SearchFlood, Client *, const string &msg) throw()
{
	typedef Func3<Hub, string, Msg::TypeMsg, Sound::TypeSound> F3;
	F3 *func = new F3(this, &Hub::addStatusMessage_gui, _("Search spam detected from ") + msg, Msg::STATUS, Sound::NONE);
	WulforManager::get()->dispatchGuiFunc(func);
}
//CheatMessage NOTE: RSX++
void Hub::on(ClientListener::CheatMessage,const Client *c, const string &msg) throw()
{
	typedef Func2<Hub, string, Msg::TypeMsg> F2;
	F2 *func = new F2(this, &Hub::addMessage_gui, _("** ") + msg, Msg::CHEAT);
	WulforManager::get()->dispatchGuiFunc(func);
}

/*this is a pop menu*/
void Hub::popmenu()
{
    userCommandMenu1->cleanMenu_gui();
    userCommandMenu1->addUser(client->getMyIdentity().getUser()->getCID().toBase32());
    userCommandMenu1->addHub(address);
    userCommandMenu1->buildMenu_gui();
    GtkWidget *menu = userCommandMenu1->getContainer();

    GtkWidget *copyHubUrl = gtk_menu_item_new_with_label(_("Copy URL"));
    GtkWidget *close = gtk_menu_item_new_with_label(_("Close"));
    GtkWidget *addFav = gtk_menu_item_new_with_label(_("Add to Favorite hubs"));

    gtk_menu_shell_append(GTK_MENU_SHELL(menu),close);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu),copyHubUrl);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu),addFav);

    g_signal_connect_swapped(copyHubUrl, "activate", G_CALLBACK(onCopyHubUrl),this);
    g_signal_connect_swapped(close, "activate", G_CALLBACK(onCloseItem),this);
    g_signal_connect_swapped(addFav, "activate", G_CALLBACK(onAddFavItem),(gpointer)this);

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
