// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
// MA 02110-1301, USA.
//

#include <dcpp/stdinc.h>
#include <dcpp/GetSet.h>
#include <dcpp/Util.h>
#include <dcpp/ColorSettings.h>
#include <linux/entry.hh>
#include <linux/WulforUtil.hh>
#include "HigDialog.hh"
#include "definitons.hh"

static GtkWidget* createComboWith3Option(gchar* a,gchar* b,gchar *c)
{
	GtkWidget* combo = gtk_combo_box_text_new();
	gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(combo),a);
	gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(combo),b);
	gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(combo),c);

	return combo;
}

HigDialog::HigDialog(dcpp::ColorSettings *_cs , bool add):
init(add),cs(_cs)
{
	dialogWin = gtk_dialog_new();
	if(cs != NULL && (cs->getMatch().empty() == false ) )
		gtk_window_set_title (GTK_WINDOW(dialogWin), (_("Higliting Propteries for") + cs->getMatch()).c_str());
	else
		gtk_window_set_title (GTK_WINDOW(dialogWin), _("Higliting Propteries for New Hig Settings"));

	mainBox = gtk_dialog_get_content_area ( GTK_DIALOG(dialogWin) );
	GtkWidget* grid = gtk_grid_new();
	gtk_container_add(GTK_CONTAINER(mainBox),grid);

	gtk_grid_attach(GTK_GRID(grid),gtk_label_new(_("Name: ")),0,0,1,1);
	entryName = gen;
	if(cs->getMatch().empty() == false)
		gtk_entry_set_text(GTK_ENTRY(entryName),cs->getMatch().c_str());
	gtk_grid_attach(GTK_GRID(grid), entryName,0,1,1,1);


	gtk_grid_attach(GTK_GRID(grid), gtk_label_new(_("Type: ")),0,2,1,1);
	comboType = createComboWith3Option(_("Word in Chat"),_("Nick in Userlist"),_("Word in Filelist"));
	gtk_grid_attach(GTK_GRID(grid), comboType,1,2,1,1);

	gtk_combo_box_set_active(GTK_COMBO_BOX(comboType),(int)cs->getFlag()-1);

	sBold = gtk_switch_new();
	sItalic = gtk_switch_new();
	sUnderline = gtk_switch_new();
	sNoti = gtk_switch_new();
	sTab = gtk_switch_new();

	gtk_grid_attach(GTK_GRID(grid),gtk_label_new(_("Bold ")),0,3,1,1);
	gtk_grid_attach(GTK_GRID(grid), sBold,1,3,1,1);
	gtk_switch_set_active(GTK_SWITCH(sBold),cs->getBold());

	gtk_grid_attach(GTK_GRID(grid),gtk_label_new(_("Italic ")),0,4,1,1);
	gtk_grid_attach(GTK_GRID(grid), sItalic,1,4,1,1);
	gtk_switch_set_active(GTK_SWITCH(sItalic),cs->getItalic());

	gtk_grid_attach(GTK_GRID(grid),gtk_label_new(_("Underline ")),0,5,1,1);
	gtk_grid_attach(GTK_GRID(grid), sUnderline,1,5,1,1);
	gtk_switch_set_active(GTK_SWITCH(sUnderline),cs->getUnderline());

	GdkRGBA color;
	gdk_rgba_parse(&color,cs->getBgColor().c_str());
	colorBgButton = gtk_color_button_new_with_rgba(&color);
	GdkRGBA color2;
	gdk_rgba_parse(&color2,cs->getFgColor().c_str());
	colorFgButton = gtk_color_button_new_with_rgba(&color2);

	gtk_grid_attach(GTK_GRID(grid),gtk_label_new(_("Background color ")),0,6,1,1);
	gtk_grid_attach(GTK_GRID(grid), colorBgButton,1,6,1,1);

	gtk_grid_attach(GTK_GRID(grid),gtk_label_new(_("Foreground color ")),0,7,1,1);
	gtk_grid_attach(GTK_GRID(grid), colorFgButton,1,7,1,1);

	gtk_widget_show_all(grid);
	GtkWidget* okButton = gtk_button_new_with_label(_("Ok"));
	GtkWidget* cancelButton = 	gtk_button_new_with_label(_("Cancel"));

	gtk_dialog_add_action_widget (GTK_DIALOG(dialogWin),
							okButton,
							GTK_RESPONSE_OK);
	gtk_dialog_add_action_widget (GTK_DIALOG(dialogWin),
							cancelButton,
							-6);
	gtk_widget_show(cancelButton);
	gtk_widget_show(okButton);

}

bool HigDialog::run() {
	gint res = gtk_dialog_run(GTK_DIALOG(dialogWin));
	// Fix crash, if the dialog gets programmatically destroyed.
	if (res == GTK_RESPONSE_NONE)
		return FALSE;

	while (res == GTK_RESPONSE_OK)
	{
		cs->setMatch(gtk_entry_get_text(GTK_ENTRY(entryName)));
		cs->setBold(gtk_switch_get_active(GTK_SWITCH(sBold)));
		cs->setItalic(gtk_switch_get_active(GTK_SWITCH(sItalic)));
		cs->setUnderline(gtk_switch_get_active(GTK_SWITCH(sUnderline)));

		GdkRGBA color;
		gtk_color_chooser_get_rgba(GTK_COLOR_CHOOSER(colorFgButton),&color);
		cs->setFgColor(WulforUtil::colorToString(&color));

		GdkRGBA color2;
		gtk_color_chooser_get_rgba(GTK_COLOR_CHOOSER(colorBgButton),&color2);
		cs->setBgColor(WulforUtil::colorToString(&color2));

		cs->setFlag((dcpp::ColorSettings::ColorFlags)gtk_combo_box_get_active(GTK_COMBO_BOX(comboType))+1);

		if(cs->getMatch().empty()) {
			gtk_widget_hide(dialogWin);
			return false;
		}
		gtk_widget_hide(dialogWin);
		return true;
	}
	gtk_widget_hide(dialogWin);
	return true;
}
