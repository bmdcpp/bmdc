/*
 * Copyright © 2004-2012 Jens Oknelid, paskharen@gmail.com
 * Copyright © 2010-2016 BMDC Team
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

#include "bookentry.hh"
#include "wulformanager.hh"
#include "settingsmanager.hh"
#include "WulforUtil.hh"
#include "gtk-fixies.hh"


using namespace std;

BookEntry::BookEntry(const EntryType type, const string &text, const string &glade, const string &id):
	Entry(type, glade, id),
	eventBox(NULL),	labelBox(NULL),
	tabMenuItem(NULL), closeButton(NULL),
	label(NULL), bCreated(true),
	bold(false), urgent(false),
	labelSize((glong)WGETI("size-label-box-bookentry")),
	icon(NULL), popTabMenuItem(NULL),
	type(type), IsCloseButton(false)
{
	GSList *group = NULL;
	labelBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL,0);

	gtk_widget_set_name(labelBox,getName().c_str());//CSS
	eventBox = gtk_event_box_new();
	gtk_widget_set_name(eventBox,getName().c_str());//CSS
	gtk_event_box_set_above_child(GTK_EVENT_BOX(eventBox), TRUE);
	gtk_event_box_set_visible_window(GTK_EVENT_BOX(eventBox), FALSE);
	// icon
	icon = gtk_image_new();
	gtk_widget_set_name(icon,getName().c_str());//CSS
	gtk_box_pack_start(GTK_BOX(labelBox), icon, FALSE, FALSE, 0);

	// Make the eventbox fill to all left-over space.
	gtk_box_pack_start(GTK_BOX(labelBox), GTK_WIDGET(eventBox), TRUE, TRUE, 0);

	label = GTK_LABEL(gtk_label_new(text.c_str()));
	gtk_widget_set_name(GTK_WIDGET(label),getName().c_str());//CSS
	gtk_container_add(GTK_CONTAINER(eventBox), GTK_WIDGET(label));

	// Align text to the left (x = 0) and in the vertical center (0.5)
	#if !GTK_CHECK_VERSION(3, 12, 0)
	gtk_misc_set_alignment(GTK_MISC(label), 0, 0.5);
	#else
	gtk_widget_set_margin_start(GTK_WIDGET(label),0);
	gtk_widget_set_margin_end(GTK_WIDGET(label),0);
	gtk_widget_set_margin_top(GTK_WIDGET(label),0);
	gtk_widget_set_margin_bottom(GTK_WIDGET(label),0);
    #endif
    if(IsCloseButton || WGETB("use-close-button"))
     {
		closeButton = gtk_button_new();
		gtk_widget_set_name(closeButton,getName().c_str());
        gtk_button_set_relief(GTK_BUTTON(closeButton), GTK_RELIEF_NONE);
        gtk_button_set_focus_on_click(GTK_BUTTON(closeButton), FALSE);

        // Shrink the padding around the close button
		GtkCssProvider *provider =  gtk_css_provider_get_default ();
		GdkDisplay *display = gdk_display_get_default ();
		GdkScreen *screen = gdk_display_get_default_screen (display);
		gtk_css_provider_load_from_data(provider,".button {\n"
                "-GtkButton-default-border : 0px;\n"
                "-GtkButton-default-outside-border : 0px;\n"
                "-GtkButton-inner-border: 0px;\n"
                "-GtkWidget-focus-line-width : 0px;\n"
                "-GtkWidget-focus-padding : 0px;\n"
                "padding: 0px;}\n\0",-1, NULL);
		gtk_style_context_add_provider_for_screen (screen, GTK_STYLE_PROVIDER (provider), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);     
     // Add the stock icon to the close button
     #if GTK_CHECK_VERSION(3,9,0)
	    GtkWidget *image = gtk_image_new_from_icon_name("window-close",GTK_ICON_SIZE_MENU);
    #else
        GtkWidget *image = gtk_image_new_from_stock(BMDC_STOCK_CLOSE, GTK_ICON_SIZE_MENU);
    #endif
		gtk_widget_set_name(image,getName().c_str());

        gtk_container_add(GTK_CONTAINER(closeButton), image);
        gtk_box_pack_start(GTK_BOX(labelBox), closeButton, FALSE, FALSE, 0);

        gtk_widget_set_tooltip_text(closeButton, _("Close tab"));
    }
	gtk_widget_show_all(labelBox);

	tabMenuItem = gtk_radio_menu_item_new_with_label(group, text.c_str());
	group = gtk_radio_menu_item_get_group(GTK_RADIO_MENU_ITEM(tabMenuItem));

	setLabel_gui(text);
	setIcon_gui(type);
	setBackForeGround(type);

	// Associates entry to the widget for later retrieval in MainWindow::switchPage_gui()
	g_object_set_data(G_OBJECT(getContainer()), "entry", (gpointer)this);

	g_signal_connect(getLabelBox(), "button-press-event", G_CALLBACK(onButtonPressed_gui), (gpointer)this);
	g_signal_connect(getLabelBox(), "button-release-event", G_CALLBACK(onButtonReleased_gui), (gpointer)this);
}

GtkWidget* BookEntry::getContainer()
{
	return getWidget("mainBox");
}

void BookEntry::setIcon_gui(const EntryType type)
{
	string stock;
	switch (type)
	{
		case Entry::FAVORITE_HUBS : stock = WGETS("icon-favorite-hubs"); break;
		case Entry::FAVORITE_USERS : stock = WGETS("icon-favorite-users"); break;
		case Entry::PUBLIC_HUBS : stock = WGETS("icon-public-hubs"); break;
		case Entry::DOWNLOAD_QUEUE : stock = WGETS("icon-queue"); break;
		case Entry::SEARCHS:
		case Entry::SEARCH : stock = WGETS("icon-search"); break;
		case Entry::SEARCH_ADL : stock = WGETS("icon-search-adl"); break;
		case Entry::SEARCH_SPY : stock = WGETS("icon-search-spy"); break;
		case Entry::FINISHED_DOWNLOADS : stock = WGETS("icon-finished-downloads"); break;
		case Entry::FINISHED_UPLOADS : stock = WGETS("icon-finished-uploads"); break;
		case Entry::PRIVATE_MESSAGE : stock = WGETS("icon-pm-online"); break;
		case Entry::HUB : stock = WGETS("icon-hub-offline"); break;
		case Entry::SHARE_BROWSER : stock = WGETS("icon-directory"); break;
		case Entry::NOTEPAD : stock = WGETS("icon-notepad"); break;
		case Entry::SYSTEML : stock = WGETS("icon-system"); break;
		case Entry::ABOUT_CONFIG : stock = WGETS("icon-system"); break;//for now
		default: ;
	}
	
	#if GTK_CHECK_VERSION(3,9,0)
	gtk_image_set_from_icon_name(GTK_IMAGE(icon), stock.c_str(), GTK_ICON_SIZE_MENU);
	#else
	gtk_image_set_from_stock(GTK_IMAGE(icon), stock.c_str(), GTK_ICON_SIZE_MENU);
	#endif
}

void BookEntry::setIcon_gui(const std::string stock)
{
	#if GTK_CHECK_VERSION(3,9,0)
	gtk_image_set_from_icon_name(GTK_IMAGE(icon), stock.c_str(), GTK_ICON_SIZE_MENU);
	#else
	gtk_image_set_from_stock(GTK_IMAGE(icon), stock.c_str(), GTK_ICON_SIZE_MENU);
	#endif
}

void BookEntry::setIconPixbufs_gui(const std::string iconspath)
{
    GdkPixbuf *pixbuf = gdk_pixbuf_new_from_file_at_size(iconspath.c_str(),15,15,NULL);
    gtk_image_set_from_pixbuf(GTK_IMAGE(icon),pixbuf);
}

void BookEntry::setLabel_gui(const string text)
{
	// Update the tab menu item label
	GtkWidget *child = gtk_bin_get_child(GTK_BIN(tabMenuItem));
	if (child && GTK_IS_LABEL(child))
		gtk_label_set_text(GTK_LABEL(child), text.c_str());

	if(IsCloseButton || WGETB("use-close-button"))
    {
        // Update the notebook tab label
       gtk_widget_set_tooltip_text(eventBox, text.c_str());
    }

	glong len = g_utf8_strlen(text.c_str(), -1);

	// Truncate the label text
	if (len > labelSize)
	{
		gchar truncatedText[text.size()];
		const string clipText = "...";
		len = labelSize - g_utf8_strlen(clipText.c_str(), -1);
		g_utf8_strncpy(truncatedText, text.c_str(), len);
		truncatedLabelText = truncatedText + clipText;
	}
	else
	{
		truncatedLabelText = text;
	}

	labelText = text;
	updateLabel_gui();

	// Update the main window title if the current tab is selected.
	if (isActive_gui())
		WulforManager::get()->getMainWindow()->setTitle(getLabelText());
}

void BookEntry::setBold_gui()
{
	if (!bold && !isActive_gui())
	{
		bold = true;
		updateLabel_gui();
	}
}

void BookEntry::setUrgent_gui()
{
	if (!isActive_gui())
	{
		MainWindow *mw = WulforManager::get()->getMainWindow();

		if (!urgent)
		{
			bold = true;
			urgent = true;
			updateLabel_gui();
			setUnread();
		}

		if (mw && !mw->isActive_gui())
			mw->setUrgent_gui();
	}
}

void BookEntry::setActive_gui()
{
	if (bold || urgent)
	{
		bold = false;
		urgent = false;
		updateLabel_gui();
		setNormal();
	}
}

bool BookEntry::isActive_gui()
{
	MainWindow *mw = WulforManager::get()->getMainWindow();
	return mw->isActive_gui() && mw->currentPage_gui() == getContainer();
}

void BookEntry::updateLabel_gui()
{
	const char *format = "%s";
	bool b_color = WGETB("colorize-tab-text");
	char color_format[256];
	string color =  urgent ? WGETS("color-tab-text-urgent") : WGETS("color-tab-text-bold");
	sprintf(color_format,"<span foreground=\"%s\">%%s</span>",color.c_str());	

	if (urgent)
		format = b_color ? color_format : "<i><b>%s</b></i>";
	else if (bold)
		format = b_color ? color_format : "<b>%s</b>";

	char *markup = g_markup_printf_escaped(format, truncatedLabelText.c_str());
	gtk_label_set_markup(label, markup);
	g_free(markup);

}

const string& BookEntry::getLabelText() const
{
	return labelText;
}
//BMDC++
void BookEntry::onCloseItem(gpointer data)
{
	BookEntry *book = (BookEntry *)data;
	if(book != NULL)
		book->removeBooK_GUI();
}

void BookEntry::removeBooK_GUI()
{
	WulforManager::get()->getMainWindow()->removeBookEntry_gui(this);
}

GtkWidget *BookEntry::createmenu()
{
    if(!IsCloseButton) {
		popTabMenuItem = gtk_menu_new();
		GtkWidget *menuItemFirst =  createItemFirstMenu();
		GtkWidget *closeTabMenuItem = gtk_menu_item_new_with_label(_("Close"));
		gtk_menu_shell_append(GTK_MENU_SHELL(popTabMenuItem),menuItemFirst);
		gtk_menu_shell_append(GTK_MENU_SHELL(popTabMenuItem),closeTabMenuItem);
		gtk_widget_show(closeTabMenuItem);
		gtk_widget_show(menuItemFirst);
		g_signal_connect_swapped(closeTabMenuItem, "activate", G_CALLBACK(onCloseItem), (gpointer)this);
		return popTabMenuItem;		
	}
	return NULL;
}

gboolean BookEntry::onButtonPressed_gui(GtkWidget*, GdkEventButton *event, gpointer data)
{
	BookEntry *book = (BookEntry *)data;
	book->previous = event->type;
	return FALSE;
}

gboolean BookEntry::onButtonReleased_gui(GtkWidget*, GdkEventButton *event, gpointer data)
{
	BookEntry *book = (BookEntry *)data;

	if ( (book != NULL) && ((event->button == 3) && (event->type == GDK_BUTTON_RELEASE) ) )
	{
		// show menu
		book->popTabMenuItem = book->createmenu();
		gtk_widget_show(book->popTabMenuItem);
		g_object_ref_sink(book->popTabMenuItem);
		gtk_menu_popup(GTK_MENU(book->popTabMenuItem),NULL, NULL, NULL,NULL,0,0);
		return TRUE;
	}
	return FALSE;
}

GtkWidget *BookEntry::createItemFirstMenu()
{
	string stock, info;
	if(bCreated) {
		switch (this->type)
		{
			case Entry::FAVORITE_HUBS :
					stock = WGETS("icon-favorite-hubs");
					info = _("Favorite Hubs");
					break;
			case Entry::FAVORITE_USERS :
					stock = WGETS("icon-favorite-users");
					info = _("Favorite Users");
					break;
			case Entry::PUBLIC_HUBS :
					stock = WGETS("icon-public-hubs");
					info = _("Public Hubs");
					break;
			case Entry::DOWNLOAD_QUEUE :
					stock = WGETS("icon-queue");
					info = _("Download Queue");
					break;
			case Entry::SEARCHS:
			case Entry::SEARCH :
					stock = WGETS("icon-search");
					info = _("Search");
					break;
			case Entry::SEARCH_ADL :
					stock = WGETS("icon-search-adl");
					info = _("ADL Search");
					break;
			case Entry::SEARCH_SPY :
					stock = WGETS("icon-search-spy");
					info = _("Spy Search");
					break;
			case Entry::FINISHED_DOWNLOADS :
					stock = WGETS("icon-finished-downloads");
					info = _("Finished Downloads");
					break;
			case Entry::FINISHED_UPLOADS :
					stock = WGETS("icon-finished-uploads");
					info = _("Finished Uploads");
					break;
			case Entry::PRIVATE_MESSAGE :
					stock = WGETS("icon-pm-online");
					info = _("Private Message");
					break;
			case Entry::HUB :
					stock = WGETS("icon-hub-offline");
					info = _("Hub");
					break;
			case Entry::SHARE_BROWSER :
					stock = WGETS("icon-directory");
					info = _("Share Browser");
					break;
			case Entry::NOTEPAD :
					stock = WGETS("icon-notepad");
					info = _("Notepad");
					break;
			case Entry::SYSTEML :
					stock = WGETS("icon-system");
					info = _("System Log");
					break;
			case Entry::ABOUT_CONFIG:
					stock = WGETS("icon-system"); //for now
					info = _("About:Config");
					break;
			default: ;
		}
		bCreated = false;
		#if GTK_CHECK_VERSION(3,9,0)
			GtkWidget *item = gtk_menu_item_new();
			gtk_menu_item_set_label(GTK_MENU_ITEM(item),info.c_str());
			return item;
		#else
			return gtk_image_menu_item_new_from_stock(stock.c_str(),NULL);	
		#endif
	}
	return NULL;	
}

void BookEntry::setBackForeGround(const EntryType type)
{
	string fg,bg,fg_unread,bg_unread;
	switch (type)
	{
		case Entry::FAVORITE_HUBS :
					if(WGETB("colored-tabs-fav-hubs")) {
						fg = WGETS("colored-tabs-fav-hubs-color-fg");
						bg = WGETS("colored-tabs-fav-hubs-color-bg");
					}
					break;
		case Entry::FAVORITE_USERS :
					if(WGETB("colored-tabs-fav-users")) {
						fg = WGETS("colored-tabs-fav-users-color-fg");
						bg = WGETS("colored-tabs-fav-users-color-bg");
					}
					break;
		case Entry::PUBLIC_HUBS :
					if(WGETB("colored-tabs-public")) {
						fg = WGETS("colored-tabs-public-color-fg");
						bg = WGETS("colored-tabs-public-color-bg");
					}
					break;
		case Entry::DOWNLOAD_QUEUE :
					if(WGETB("colored-tabs-download-quene")) {
						fg = WGETS("colored-tabs-download-quene-color-fg");
						bg = WGETS("colored-tabs-download-quene-color-bg");
					}
					break;
		case Entry::SEARCHS:
		case Entry::SEARCH :
				if(WGETB("colored-tabs-searchs")) {
						fg = WGETS("colored-tabs-searchs-color-fg");
						bg = WGETS("colored-tabs-searchs-color-bg");
				}
				if(WGETB("colored-tabs-searchs-unread")) {
						fg_unread = WGETS("colored-tabs-searchs-color-fg-unread");
						bg_unread = WGETS("colored-tabs-searchs-color-bg-unread");
				}	
					break;
		case Entry::SEARCH_ADL :
					if(WGETB("colored-tabs-adl")) {
						fg = WGETS("colored-tabs-adl-color-fg");
						bg = WGETS("colored-tabs-adl-color-bg");
					}
					break;
		case Entry::SEARCH_SPY :
					if(WGETB("colored-tabs-spy")) {
						fg = WGETS("colored-tabs-spy-color-fg");
						bg = WGETS("colored-tabs-spy-color-bg");
					}
					break;
		case Entry::FINISHED_DOWNLOADS :
				if(WGETB("colored-tabs-downloads")) {
						fg = WGETS("colored-tabs-downloads-color-fg");
						bg = WGETS("colored-tabs-downloads-color-bg");
					}
					break;
		case Entry::FINISHED_UPLOADS :
				if(WGETB("colored-tabs-uploads")) {
					fg = WGETS("colored-tabs-uploads-color-fg");
					bg = WGETS("colored-tabs-uploads-color-bg");
				}
				break;
		case Entry::PRIVATE_MESSAGE :
					if(WGETB("colored-tabs-pm")) {
						fg = WGETS("colored-tabs-pm-color-fg");
						bg = WGETS("colored-tabs-pm-color-bg");
					}
					if(WGETB("colored-tabs-pm-unread")) {
						fg_unread = WGETS("colored-tabs-pm-color-fg-unread");
						bg_unread = WGETS("colored-tabs-pm-color-bg-unread");
					}
					break;
		case Entry::HUB :
					if(WGETB("colored-tabs-hub")) {
						fg = WGETS("colored-tabs-hub-color-fg");
						bg = WGETS("colored-tabs-hub-color-bg");
					}
					if(WGETB("colored-tabs-hub-unread")) {
						fg_unread = WGETS("colored-tabs-hub-color-fg-unread");
						bg_unread = WGETS("colored-tabs-hub-color-bg-unread");
					}
					break;
		case Entry::SHARE_BROWSER :
					if(WGETB("colored-tabs-shareb")) {
						fg = WGETS("colored-tabs-shareb-color-fg");
						bg = WGETS("colored-tabs-shareb-color-bg");
					}
					break;
		case Entry::NOTEPAD :
					if(WGETB("colored-tabs-notepad")) {
						fg = WGETS("colored-tabs-notepad-color-fg");
						bg = WGETS("colored-tabs-notepad-color-bg");
					}
					break;
		case Entry::SYSTEML :
				if(WGETB("colored-tabs-system")) {
						fg = WGETS("colored-tabs-system-color-fg");
						bg = WGETS("colored-tabs-system-color-bg");
				}
				if(WGETB("colored-tabs-system-unread")) {
						fg_unread = WGETS("colored-tabs-system-color-fg-unread");
						bg_unread = WGETS("colored-tabs-system-color-bg-unread");
				}
				break;
		case Entry::ABOUT_CONFIG:
		default: return;
	}
	string name = getName();
	GtkCssProvider *provider = gtk_css_provider_new ();
	GdkDisplay *display = gdk_display_get_default ();
	GdkScreen *screen = gdk_display_get_default_screen (display);
	
	string t_css = dcpp::Util::emptyString;
	if(WGETB("custom-font-size")) {
		string size = dcpp::Util::toString(WGETI("book-font-size"))+" %";
		t_css = std::string("#"+name+" { color:"+fg+"; background: "+bg+"; font-size:"+size+"; }\n\0");
	} else
		t_css = std::string("#"+name+" { color:"+fg+"; background: "+bg+"; }\n\0");
	
	gtk_css_provider_load_from_data (GTK_CSS_PROVIDER (provider),t_css.c_str(),-1, NULL);

	gtk_style_context_add_provider_for_screen (screen,
											GTK_STYLE_PROVIDER(provider),
											GTK_STYLE_PROVIDER_PRIORITY_USER);
	g_object_unref (provider);


	provider = gtk_css_provider_new ();
	display = gdk_display_get_default ();
	screen = gdk_display_get_default_screen (display);
	std::string t_css2 = std::string("#"+name+":active { color:"+fg_unread+"; background: "+bg_unread+"; font-weight: bold; }\n\0");
	gtk_css_provider_load_from_data (GTK_CSS_PROVIDER (provider),t_css2.c_str(),-1, NULL);

	gtk_style_context_add_provider_for_screen (screen,
											GTK_STYLE_PROVIDER(provider),
											GTK_STYLE_PROVIDER_PRIORITY_USER);
	g_object_unref (provider);

}
//Dont Translate string in this function below!
string BookEntry::getName() //CSS:getName() In this should not include : ,spaces and so #dialog(s) is there only for is it in one enum
{
	string str = dcpp::Util::emptyString;
	const Entry::EntryType  type = getType();
	
	switch(type)
	{
		
		case	DOWNLOAD_QUEUE:
				str = "DownloadQueue";
				break;
		case	FAVORITE_HUBS:
				str = "FavoriteHubs";
				break;
		case	FAVORITE_USERS:
				str = "FavoriteUsers";
				break;
		case	FINISHED_DOWNLOADS:
				str = "FinishedDowloads";
				break;
		case	FINISHED_UPLOADS:
				str = "FinishedUploads";
				break;
		case	HASH_DIALOG:
				break;	
		case	CMD:
				str = "CMDDebug";
				break;
		case	HUB:
				str = "Hub"+h_name;
				break;
		case	MAIN_WINDOW:
				break;
		case	PRIVATE_MESSAGE:
				str = "PrivateMessage"+h_name;
				break;
		case	PUBLIC_HUBS:
				str = "PublicHubs";
				break;
		case	SEARCH:
				str = "Search";
				break;
		case	SETTINGS_DIALOG:
				break;
		case	SHARE_BROWSER:
				str = "ShareBrowser"+h_name;
				break;
		case	TRANSFERS:
				break;
		case	USER_COMMAND_MENU:
				break;
		case	SEARCH_SPY:
				str = "SearchSpy";
				break;
		case	SEARCH_ADL:
				str = "SearchADL";
				break;
		case	NOTEPAD:
				str = "Notepad";
				break;
		case	SYSTEML:
				str = "SystemLog";
				break;
		case	UPLOADQUEUE:
				str = "UploadQueue";
				break;
		case	RECENT:
				str = "RecentHubs";
				break;
		case	DETECTION:
				str = "Detection";
				break;	
		case	ABOUT_CONFIG:
				str = "AboutConfig";
				break;
		case	EXPORT_DIALOG:
				break;
		case	SEARCHS:
				str = "SearchEntry";
		case	FAV_HUB:
		default:break;
	};
	
	return str;	
}


