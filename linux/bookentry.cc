/*
* Copyright © 2011-2025 BMDC
* This file is part of BMDC.
* BMDC is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 2 of the License, or (at your option) any later version.
*
* BMDC is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License along with Xfce-nameday-plugin. If not, see <https://www.gnu.org/licenses/>.
*/

#include "bookentry.hh"
#include "wulformanager.hh"
#include "settingsmanager.hh"
#include "GuiUtil.hh"
#include "gtk-fixies.hh"

using namespace std;

const GActionEntry BookEntry::book_entries[] =
 {
    { "close-button", onCloseItem , NULL, NULL, NULL }
};


BookEntry::BookEntry(const EntryType type, const string &text, const string &glade, const string &id):
	Entry(type, glade, id),
	labelBox(NULL),
	tabMenuItem(NULL), closeButton(NULL),
	label(NULL), bCreated(true),
	bold(false), urgent(false),
	labelSize((glong)WGETI("size-label-box-bookentry")),
	icon(NULL), popTabMenuItem(NULL),
	type(type), bIsCloseButton(false)
{
	labelBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL,1);
	GSimpleActionGroup* simple = g_simple_action_group_new ();
	g_action_map_add_action_entries (G_ACTION_MAP (simple), book_entries, G_N_ELEMENTS (book_entries), (gpointer)this);
	gtk_widget_insert_action_group(labelBox,"book" ,G_ACTION_GROUP(simple));
	gtk_widget_set_name(labelBox,getName().c_str());
	// icon
	icon = gtk_image_new();
	gtk_widget_set_name(icon,getName().c_str());
	gtk_box_append(GTK_BOX(labelBox), icon);

	label = GTK_LABEL(gtk_label_new(text.c_str()));
	gtk_widget_set_name(GTK_WIDGET(label),getName().c_str());
	gtk_box_append(GTK_BOX(labelBox), GTK_WIDGET(label));

    if(bIsCloseButton || WGETB("use-close-button"))
     {
		closeButton = gtk_button_new();
		gtk_widget_set_name(closeButton, getName().c_str());
     	// Add the image to the close button
        GtkWidget* image = gtk_image_new_from_icon_name ("window-close");
        gtk_button_set_child(GTK_BUTTON(closeButton), image);
        gtk_box_append(GTK_BOX(labelBox), closeButton);
    }
	gtk_widget_show(labelBox);

	tabMenuItem =  g_menu_new();

	setLabel_gui(text);
	setIcon_gui(type);
	setBackForeGround(type);

	// Associates entry to the widget for later retrieval in MainWindow::switchPage_gui()
	g_object_set_data(G_OBJECT(getContainer()), "entry", (gpointer)this);

/* Register for mouse right button click "pressed" and "released" events on  widget*/
  GtkGesture *gesture;
  gesture = gtk_gesture_click_new ();
  gtk_gesture_single_set_button (GTK_GESTURE_SINGLE (gesture), 3);
  g_signal_connect (gesture, "pressed",
                    G_CALLBACK (onButtonPressed_gui), (gpointer)this);
  g_signal_connect (gesture, "released",
                    G_CALLBACK (onButtonReleased_gui), (gpointer)this);
  gtk_widget_add_controller (GTK_WIDGET(getLabelBox()), GTK_EVENT_CONTROLLER (gesture));
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
		case Entry::ABOUT_CONFIG : stock = WGETS("icon-system"); break;
		default: ;
	}
	gtk_image_set_from_resource(GTK_IMAGE(icon) ,(string("/org/bmdc-team/bmdc/icons/hicolor/22x22/categories/")+stock+".png").c_str());
}

void BookEntry::setIcon_gui(const std::string stock)
{
	gtk_image_set_from_icon_name(GTK_IMAGE(icon), stock.c_str());
}

void BookEntry::setIconPixbufs_gui(const std::string iconspath)
{
    GdkPixbuf *pixbuf = gdk_pixbuf_new_from_file_at_size(iconspath.c_str(),15,15,NULL);
    gtk_image_set_from_pixbuf(GTK_IMAGE(icon),pixbuf);
}

void BookEntry::setLabel_gui(const string text)
{
	//@TODO: Update the tab menu item label

	if(bIsCloseButton || WGETB("use-close-button"))
    {
        // Update the notebook tab label
       gtk_widget_set_tooltip_text(getLabelBox(), text.c_str());
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

void BookEntry::onCloseItem(GtkWidget*,GVariant* ,gpointer data)
{
	BookEntry *book = (BookEntry *)data;
	if(book != NULL)
		book->removeBooK_GUI();
}

void BookEntry::removeBooK_GUI()
{
	WulforManager::get()->getMainWindow()->removeBookEntry_gui(this);
}

GMenu *BookEntry::createmenu()
{
    GMenu* menu = g_menu_new();
    GMenuItem* item_one = createItemFirstMenu();
    g_menu_append_item(menu, item_one);
    GMenuItem* item = g_menu_item_new("Close" , "book.close-button");
	g_menu_append_item(menu, item);
	return menu;
}

void BookEntry::onButtonPressed_gui(GtkGestureClick* /*gesture*/,
                                   int               /* n_press*/,
                                   double             x,
                                   double             y,
                                   gpointer         *data)
{
	BookEntry *book = (BookEntry *)data;
    GMenu* menu =   book->createmenu();
    GtkWidget *pop = gtk_popover_menu_new_from_model(G_MENU_MODEL(menu));
	gtk_widget_set_parent(pop, book->getLabelBox() );
	gtk_popover_set_pointing_to(GTK_POPOVER(pop), &(const GdkRectangle){x,y,1,1});
	gtk_popover_popup (GTK_POPOVER(pop));
}

void BookEntry::onButtonReleased_gui(GtkGestureClick* /*gesture*/,
                                    int              /*n_press*/,
                                    double           /*x*/,
                                    double           /*y*/,
                                    GtkWidget*       /*widget*/)
{

}

GMenuItem *BookEntry::createItemFirstMenu()
{
	if(bCreated) {
		string stock, info;
		switch (this->type)
		{
			case Entry::FAVORITE_HUBS :
			{
					info = _("Favorite Hubs");
					stock = WGETS("icon-favorite-hubs"); 
					break;
			}
			case Entry::FAVORITE_USERS :
			{
					info = _("Favorite Users");
					stock = WGETS("icon-favorite-users");
					break;
			}
			case Entry::PUBLIC_HUBS :
			{
					info = _("Public Hubs");
					stock = WGETS("icon-public-hubs");
					break;
			}
			case Entry::DOWNLOAD_QUEUE :
			{
					info = _("Download Queue");
					break;
			}
			case Entry::SEARCHS:
			case Entry::SEARCH :
			{
					info = _("Search");
					break;
			}
			case Entry::SEARCH_ADL :
			{
					info = _("ADL Search");
					break;
			}
			case Entry::SEARCH_SPY :
			{
					info = _("Spy Search");
					break;
			}
			case Entry::FINISHED_DOWNLOADS :
			{
					info = _("Finished Downloads");
					stock = WGETS("icon-finished-downloads");
					break;
			}
			case Entry::FINISHED_UPLOADS :
			{
					stock = WGETS("icon-finished-uploads");
					info = _("Finished Uploads");
					break;
			}
			case Entry::PRIVATE_MESSAGE :
			{
				//	stock = WGETS("icon-pm-online");
					info = _("Private Message");
					break;
			}
			case Entry::HUB :
			{
					info = _("Hub");
					break;
			}
			case Entry::SHARE_BROWSER :
			{
					info = _("Share Browser");
					break;
			}
			case Entry::NOTEPAD :
			{
					info = _("Notepad");
					stock = WGETS("icon-notepad");
					break;
			}
			case Entry::SYSTEML :
			{
					info = _("System Log");
					stock = WGETS("icon-system");
					break;
			}
			case Entry::ABOUT_CONFIG:
			{
					info = _("About:Config");
					stock = WGETS("icon-system");
					break;
			}
			default: ;
		}
		bCreated = false;
	    GMenuItem* item = g_menu_item_new(info.c_str(),NULL);
	    GtkWidget* image = gtk_image_new_from_resource((string("/org/bmdc-team/bmdc/icons/hicolor/22x22/categories")+stock+".png").c_str());
	    GIcon* icon = gtk_image_get_gicon(GTK_IMAGE(image));
	    g_menu_item_set_icon (item, icon);
        return item;
	}
}

void BookEntry::setBackForeGround(const EntryType type)
{
	string fg = "#FFFFFF",bg = "#000000",fg_unread = fg,bg_unread = bg;
	switch (type)
	{
		case Entry::FAVORITE_HUBS :
		{
					if(WGETB("colored-tabs-fav-hubs")) {
						fg = WGETS("colored-tabs-fav-hubs-color-fg");
						bg = WGETS("colored-tabs-fav-hubs-color-bg");
					}
					break;
		}
		case Entry::FAVORITE_USERS :
		{
					if(WGETB("colored-tabs-fav-users")) {
						fg = WGETS("colored-tabs-fav-users-color-fg");
						bg = WGETS("colored-tabs-fav-users-color-bg");
					}
					break;
		}
		case Entry::PUBLIC_HUBS :
		{
					if(WGETB("colored-tabs-public")) {
						fg = WGETS("colored-tabs-public-color-fg");
						bg = WGETS("colored-tabs-public-color-bg");
					}
					break;
		}
		case Entry::DOWNLOAD_QUEUE :
		{
					if(WGETB("colored-tabs-download-quene")) {
						fg = WGETS("colored-tabs-download-quene-color-fg");
						bg = WGETS("colored-tabs-download-quene-color-bg");
					}
					break;
		}
		case Entry::SEARCHS:
		case Entry::SEARCH :
		{
				if(WGETB("colored-tabs-searchs")) {
						fg = WGETS("colored-tabs-searchs-color-fg");
						bg = WGETS("colored-tabs-searchs-color-bg");
				}
				if(WGETB("colored-tabs-searchs-unread")) {
						fg_unread = WGETS("colored-tabs-searchs-color-fg-unread");
						bg_unread = WGETS("colored-tabs-searchs-color-bg-unread");
				}
					break;
		}
		case Entry::SEARCH_ADL :
		{
					if(WGETB("colored-tabs-adl")) {
						fg = WGETS("colored-tabs-adl-color-fg");
						bg = WGETS("colored-tabs-adl-color-bg");
					}
					break;
		}
		case Entry::SEARCH_SPY :
		{
					if(WGETB("colored-tabs-spy")) {
						fg = WGETS("colored-tabs-spy-color-fg");
						bg = WGETS("colored-tabs-spy-color-bg");
					}
					break;
		}
		case Entry::FINISHED_DOWNLOADS :
		{
				if(WGETB("colored-tabs-downloads")) {
						fg = WGETS("colored-tabs-downloads-color-fg");
						bg = WGETS("colored-tabs-downloads-color-bg");
					}
					break;
		}
		case Entry::FINISHED_UPLOADS :
		{
				if(WGETB("colored-tabs-uploads")) {
					fg = WGETS("colored-tabs-uploads-color-fg");
					bg = WGETS("colored-tabs-uploads-color-bg");
				}
				break;
		}
		case Entry::PRIVATE_MESSAGE :
		{
					if(WGETB("colored-tabs-pm")) {
						fg = WGETS("colored-tabs-pm-color-fg");
						bg = WGETS("colored-tabs-pm-color-bg");
					}
					if(WGETB("colored-tabs-pm-unread")) {
						fg_unread = WGETS("colored-tabs-pm-color-fg-unread");
						bg_unread = WGETS("colored-tabs-pm-color-bg-unread");
					}
					break;
		}
		case Entry::HUB :
		{
					if(WGETB("colored-tabs-hub")) {
						fg = WGETS("colored-tabs-hub-color-fg");
						bg = WGETS("colored-tabs-hub-color-bg");
					}
					if(WGETB("colored-tabs-hub-unread")) {
						fg_unread = WGETS("colored-tabs-hub-color-fg-unread");
						bg_unread = WGETS("colored-tabs-hub-color-bg-unread");
					}
					break;
		}
		case Entry::SHARE_BROWSER :
		{
					if(WGETB("colored-tabs-shareb")) {
						fg = WGETS("colored-tabs-shareb-color-fg");
						bg = WGETS("colored-tabs-shareb-color-bg");
					}
					break;
		}
		case Entry::NOTEPAD :
		{
					if(WGETB("colored-tabs-notepad")) {
						fg = WGETS("colored-tabs-notepad-color-fg");
						bg = WGETS("colored-tabs-notepad-color-bg");
					}
					break;
		}
		case Entry::SYSTEML :
		{
				if(WGETB("colored-tabs-system")) {
						fg = WGETS("colored-tabs-system-color-fg");
						bg = WGETS("colored-tabs-system-color-bg");
				}
				if(WGETB("colored-tabs-system-unread")) {
						fg_unread = WGETS("colored-tabs-system-color-fg-unread");
						bg_unread = WGETS("colored-tabs-system-color-bg-unread");
				}
				break;
		}
		case Entry::ABOUT_CONFIG:
		default: return;
	}
	string name = getName();

	GtkStyleContext *context;
	GtkCssProvider *provider;
	context = gtk_widget_get_style_context (getLabelBox());
	provider = gtk_css_provider_new ();

	string t_css = dcpp::Util::emptyString;
	if(WGETB("custom-font-size")) {
		string size = std::to_string(WGETI("book-font-size"))+" %";
		t_css = std::string("#"+name+" { color:"+fg+"; background: "+bg+"; font-size:"+size+"; }\n\0");
	} else
		t_css = std::string("#"+name+" { color:"+fg+"; background: "+bg+"; }\n\0");

	t_css += std::string("#"+name+":active { color:"+fg_unread+"; background: "+bg_unread+"; font-weight: bold; }\n\0");
	gtk_css_provider_load_from_data (GTK_CSS_PROVIDER (provider),t_css.c_str(),-1);

	gtk_style_context_add_provider (context,
                                GTK_STYLE_PROVIDER (provider),
                                GTK_STYLE_PROVIDER_PRIORITY_USER);

}
/*
 Dont Translate string in this function below!
 CSS:getName() In this should not include : ,spaces and so on #dialog(s) is there only for is it in one enum
*/
string BookEntry::getName()
{
	string str = string();
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



