/*
 * Copyright © 2009-2016 freedcpp, http://code.google.com/p/freedcpp
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

#include <gdk/gdk.h>
#include <math.h>

#include "settingsmanager.hh"
#include "wulformanager.hh"
#include "WulforUtil.hh"
#include "emoticons.hh"
#include "emoticonsdialog.hh"


using namespace std;
using namespace dcpp;

const string EmoticonsDialog::sizeIcon[] = {
	"16x16", "22x22", "24x24", "32x32", "36x36", "48x48", "64x64", "0"
};
Emoticons* EmoticonsDialog::em_global = NULL;
EmoticonsDialog::EmoticonsDialog(GtkWidget *chat, GtkWidget *button, GtkWidget *menu, string packName /*Util::emptyString*/, const string& address /*Util::empty*/) :
	Chat(chat),
	Button(button),
	Menu(menu),
	dialog(NULL),
	icon_width(16),
	icon_height(16),
	currIconSize("16x16"),
	packName(packName),
	address(address)
{
	g_object_ref_sink(Menu);

	if(!address.empty()) {
		
		bool dontCreate = false;
		Emoticons *em = nullptr; 
		for(auto i:hubs)
		{
			em = i.second;
			if( (em != NULL) && (em->getCurrPackName_gui() == packName) ){
			   dontCreate = true; break;
			}
		}
		
		if(dontCreate == false) {
			em = Emoticons::start(packName);
		}
		
		if(em != NULL)
			hubs.insert(make_pair(address,em));
	} else{
		if(!em_global)
			em_global = Emoticons::start();
	}
}

Emoticons *EmoticonsDialog::getEmot(const std::string &address)
{
	if(address.empty()) return em_global;

	map<std::string,Emoticons*>::iterator it;
	if( (it = hubs.find(address) ) != hubs.end())
		return it->second;
	else return em_global;
}

EmoticonsDialog::~EmoticonsDialog()
{
	g_object_unref(Menu);
	map<std::string,Emoticons*>::iterator it;
	if( (it = hubs.find(address)) != hubs.end() )
	{
			delete it->second;
			hubs.erase(it);
	}
	delete em_global;


	if (dialog != NULL)
		gtk_widget_destroy(dialog);
}

void EmoticonsDialog::buildEmotMenu_gui()
{
	gtk_container_foreach(GTK_CONTAINER(Menu), (GtkCallback)gtk_widget_destroy, NULL);

	GtkWidget *item;

	/* add packs menu */
	item = gtk_menu_item_new_with_label(_("Emotion packs"));
	gtk_menu_shell_append(GTK_MENU_SHELL(Menu), item);
	addPacksMenu(item);

	/* add icon size menu */
	item = gtk_menu_item_new_with_label(_("Icon size"));
	gtk_menu_shell_append(GTK_MENU_SHELL(Menu), item);
	addIconSizeMenu(item);
}

void EmoticonsDialog::addPacksMenu(GtkWidget *item)
{
	const string currPackName = getEmot(address)->getCurrPackName_gui();
	string path = WulforManager::get()->getPath() + G_DIR_SEPARATOR_S + "emoticons" + G_DIR_SEPARATOR_S;

	GtkWidget *check_item;
	GtkWidget *packs_menu = gtk_menu_new();
	StringList files = File::findFiles(path, "*.xml");
	string packName;
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(item), packs_menu);

	for (auto it = files.begin(); it != files.end(); ++it)
	{
		packName = Util::getFileName(*it);
		string::size_type pos = packName.rfind('.');
		packName = packName.substr(0, pos);

		if (packName != "default")
		{
			check_item = gtk_check_menu_item_new_with_label(packName.c_str());
			gtk_menu_shell_prepend(GTK_MENU_SHELL(packs_menu), check_item);
		}
		else
		{
			check_item = gtk_separator_menu_item_new();
			gtk_menu_shell_append(GTK_MENU_SHELL(packs_menu), check_item);

			check_item = gtk_check_menu_item_new_with_label(_("Default"));
			gtk_menu_shell_append(GTK_MENU_SHELL(packs_menu), check_item);
		}

		if (currPackName == packName)
			gtk_check_menu_item_set_active((GtkCheckMenuItem*)check_item, TRUE);
		else
			gtk_check_menu_item_set_active((GtkCheckMenuItem*)check_item, FALSE);

		g_signal_connect(check_item, "activate", G_CALLBACK(onCheckPacksMenu), (gpointer)this);
		g_object_set_data_full(G_OBJECT(check_item), "current-pack-name", g_strdup(packName.c_str()), g_free);
	}
}

void EmoticonsDialog::addIconSizeMenu(GtkWidget *item)
{
	GtkWidget *check_item;
	GtkWidget *menu = gtk_menu_new();
	const string icon_size = WGETS("emoticons-icon-size");
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(item), menu);

	for (int i = FIRST; i < LAST; i++)
	{
		if (sizeIcon[i] != sizeIcon[DEFAULT])
		{
			check_item = gtk_check_menu_item_new_with_label(sizeIcon[i].c_str());
			gtk_menu_shell_prepend(GTK_MENU_SHELL(menu), check_item);
		}
		else
		{
			check_item = gtk_separator_menu_item_new();
			gtk_menu_shell_append(GTK_MENU_SHELL(menu), check_item);

			check_item = gtk_check_menu_item_new_with_label("100%");
			gtk_menu_shell_append(GTK_MENU_SHELL(menu), check_item);
		}

		if (icon_size == sizeIcon[i])
			gtk_check_menu_item_set_active((GtkCheckMenuItem*)check_item, TRUE);
		else
			gtk_check_menu_item_set_active((GtkCheckMenuItem*)check_item, FALSE);

		g_signal_connect(check_item, "activate", G_CALLBACK(onCheckIconSizeMenu), (gpointer) this);
		g_object_set_data_full(G_OBJECT(check_item), "icon-size", g_strdup(sizeIcon[i].c_str()), g_free);
	}
}

void EmoticonsDialog::onCheckIconSizeMenu(GtkMenuItem *checkItem, gpointer data)
{
	EmoticonsDialog *ed = (EmoticonsDialog *) data;
	string icon_size = (gchar*) g_object_get_data(G_OBJECT(checkItem), "icon-size");

	WSET("emoticons-icon-size", icon_size);
	ed->setCurrIconSize(icon_size);
}

void EmoticonsDialog::setCurrIconSize(const string &size)
{
	currIconSize = size;

	if (size == sizeIcon[x16])
		icon_width = icon_height = 16; // 16x16

	else if (size == sizeIcon[x22])
		icon_width = icon_height = 22; // 22x22

	else if (size == sizeIcon[x24])
		icon_width = icon_height = 24; // 24x24

	else if (size == sizeIcon[x32])
		icon_width = icon_height = 32; // 32x32

	else if (size == sizeIcon[x36])
		icon_width = icon_height = 36; // 36x36

	else if (size == sizeIcon[x48])
		icon_width = icon_height = 48; // 48x48

	else if (size == sizeIcon[x64])
		icon_width = icon_height = 64; // 64x64

	else if (size != sizeIcon[DEFAULT])    // unknown
	{
		currIconSize = sizeIcon[DEFAULT];
		WSET("emoticons-icon-size", sizeIcon[DEFAULT]);
	}
}

void EmoticonsDialog::onCheckPacksMenu(GtkMenuItem *checkItem, gpointer data)
{
	EmoticonsDialog *ed = (EmoticonsDialog *)data;	
	string currPackName = (gchar *)g_object_get_data(G_OBJECT(checkItem), "current-pack-name");
	{
		ed->getEmot(ed->address)->setCurrPackName_gui(currPackName);
		ed->getEmot(ed->address)->reloadPack_gui();
	}
}

void EmoticonsDialog::showEmotDialog_gui()
{
	g_return_if_fail(dialog == NULL);

	/* create popup dialog */
	dialog = gtk_window_new(GTK_WINDOW_POPUP);
	gtk_widget_set_name(dialog,"EmoticonsDialog");//name for CSS'ing

	build();
	position();
	graber();

	g_signal_connect(G_OBJECT(dialog), "event", G_CALLBACK(event), (gpointer)this);
}

void EmoticonsDialog::build()
{
	guint left_attach = 0,
		right_attach,
		top_attach = 0;

	const int sizetable = getEmot(address)->getCountFile_gui();
	Emot::List &list = getEmot(address)->getPack_gui();
	/* rows & columns */
	guint rows, columns;
	rows = columns = (guint)sqrt((double)sizetable);

	if ((rows*columns) != (guint)sizetable)
		if ((++columns*rows) < (guint)sizetable) rows++;

	/* set options dialog */
	GtkCssProvider *provider = gtk_css_provider_new ();
	GdkDisplay *display = gdk_display_get_default ();
	GdkScreen *screen = gdk_display_get_default_screen (display);
	std::string t_css = "GtkDialog#EmoticonsDialog { background: #faddab; }\n\0";
	gtk_css_provider_load_from_data (GTK_CSS_PROVIDER (provider),t_css.c_str(),-1, NULL);
	gtk_style_context_add_provider_for_screen (screen,
														GTK_STYLE_PROVIDER(provider),
														GTK_STYLE_PROVIDER_PRIORITY_USER);
	g_object_unref (provider);
	/*-------*/
	/* create dialog body */
	GtkWidget *frame = gtk_frame_new(NULL);
	gtk_container_add(GTK_CONTAINER(dialog), frame);
	gtk_frame_set_shadow_type(GTK_FRAME(frame), GTK_SHADOW_ETCHED_OUT);

	GtkWidget *table = gtk_grid_new();
	gtk_container_add(GTK_CONTAINER(frame), table);

	gtk_widget_show(frame);
	gtk_widget_show(table);

	int i = 1;

	setCurrIconSize(WGETS("emoticons-icon-size"));
	bool useDefault = currIconSize != sizeIcon[DEFAULT]? FALSE : TRUE;

	for (Emot::Iter it = list.begin(); it != list.end(); ++it)
	{
		GtkWidget *image = NULL;
		GdkPixbuf *pixbuf = (*it)->getPixbuf();

		if (pixbuf != NULL)
		{
			gchar *name = (gchar *)(*it)->getNames()->data;

			if (!useDefault)
			{
				GdkPixbuf *scale = WulforUtil::scalePixbuf(pixbuf, icon_width, icon_height);
				image = gtk_image_new_from_pixbuf(scale);
				g_object_unref(scale);
			}
			else
				image = gtk_image_new_from_pixbuf(pixbuf);

			GtkWidget *icon = gtk_button_new();
			gtk_button_set_image(GTK_BUTTON(icon), image);
			gtk_button_set_relief(GTK_BUTTON(icon), GTK_RELIEF_NONE);
			gtk_widget_show(icon);
			gtk_grid_attach(GTK_GRID(table), icon, left_attach, top_attach, 1, 1);

			gtk_widget_set_tooltip_text(icon, name);

			g_object_set_data_full(G_OBJECT(icon), "text", g_strdup(name), g_free);
			g_signal_connect(G_OBJECT(icon), "clicked", G_CALLBACK(onChat), (gpointer) this);

			right_attach = ++left_attach + 1;

			if (right_attach == columns + 1)
			{
				left_attach = 0;
				right_attach = left_attach + 1;
				++top_attach;
			}

			if (++i > sizetable)
				break;
		}
		else
			continue;
	}
}

void EmoticonsDialog::position()
{
	GtkRequisition requisition;

	// ox, oy, w, h
	gint Wx, Wy, Dh, Dw,
		Bx, By, Bw;
	GtkAllocation allocation;//@NOTE: GTK3

	gtk_widget_get_preferred_size (dialog,NULL, &requisition);

	Dw = requisition.width;
	Dh = requisition.height;

	gtk_widget_get_preferred_size(Button,NULL, &requisition);

	Bw = requisition.width;

	/* the position of a window in root window coordinates */
	gdk_window_get_origin( gtk_widget_get_window(Button), &Bx, &By);
	gtk_widget_get_allocation(Button,&allocation);

	Bx += allocation.x;
	By += allocation.y;

	Wx = Bx - Dw + Bw;
	Wy = By - Dh;

	gtk_window_move(GTK_WINDOW(dialog), Wx, Wy);
	gtk_widget_show(dialog);
}

void EmoticonsDialog::graber()
{
	/* grabs the pointer (usually a mouse) */
	if(gdk_device_grab(gtk_get_current_event_device(),gtk_widget_get_window(dialog), GDK_OWNERSHIP_NONE,TRUE,(GdkEventMask)(GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK),NULL,GDK_CURRENT_TIME))
		gtk_grab_add(dialog);
}

void EmoticonsDialog::onChat(GtkWidget *widget , gpointer data /*this*/)
{
	EmoticonsDialog *ed = (EmoticonsDialog *) data;

	// set focus chat enry
	if (!gtk_widget_is_focus(ed->Chat))
		gtk_widget_grab_focus(ed->Chat);

	/* insert text to chat entry */
	gchar *text = (gchar *) g_object_get_data(G_OBJECT(widget), "text");
	gint pos = gtk_editable_get_position(GTK_EDITABLE(ed->Chat));
	gtk_editable_insert_text(GTK_EDITABLE(ed->Chat), text, -1, &pos);
	gtk_editable_set_position(GTK_EDITABLE(ed->Chat), pos);

	gtk_widget_destroy(ed->dialog);
	ed->dialog = NULL;
}

gboolean EmoticonsDialog::event(GtkWidget*, GdkEvent *event, gpointer data /*this*/)
{
	EmoticonsDialog *ed = (EmoticonsDialog *) data;

	if (event->type == GDK_BUTTON_PRESS || event->type == GDK_BUTTON_RELEASE)
	{
		switch (event->button.button)
		{
			case 1: case 2: case 3:

				gtk_widget_destroy(ed->dialog);
				ed->dialog = NULL;
			break;
		}
	}
	return FALSE;
}
