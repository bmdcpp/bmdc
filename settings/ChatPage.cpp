/* 
* (C) 2025 BMDC
* This file is part of BMDC.
* BMDC is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 2 of the License, or (at your option) any later version.
*
* BMDC is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License along with Xfce-nameday-plugin. If not, see <https://www.gnu.org/licenses/>.
*/

#include "../dcpp/Util.h"
#include "../linux/GuiUtil.hh"
#include "ChatPage.hh"
#include "seUtil.hh"

using namespace std;
using namespace dcpp;

const char* ChatPage::page_name = "→ Colors&Fonts";

void ChatPage::show(GtkWidget* parent, GtkWidget* old)
{
	WulforSettingsManager *wsm = WulforSettingsManager::getInstance();
	box = gtk_box_new(GTK_ORIENTATION_VERTICAL,0);
	GtkWidget *box2 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL,0);
	//GtkWidget *scroll = gtk_scrolled_window_new(NULL);
	textStyleView = TreeView();//fix crashes
	textStyleView.setView(GTK_TREE_VIEW(gtk_tree_view_new()));
	textStyleView.insertColumn(_("Style"), G_TYPE_STRING, TreeView::STRING, -1);
	textStyleView.insertHiddenColumn("ForeColor", G_TYPE_STRING);
	textStyleView.insertHiddenColumn("BackColor", G_TYPE_STRING);
	textStyleView.insertHiddenColumn("Bolt", G_TYPE_INT);
	textStyleView.insertHiddenColumn("Italic", G_TYPE_INT);
	textStyleView.insertHiddenColumn("keyForeColor", G_TYPE_STRING);
	textStyleView.insertHiddenColumn("keyBackColor", G_TYPE_STRING);
	textStyleView.insertHiddenColumn("keyBolt", G_TYPE_STRING);
	textStyleView.insertHiddenColumn("keyItalic", G_TYPE_STRING);
	textStyleView.finalize();

	textStyleStore = gtk_list_store_newv(textStyleView.getColCount(), textStyleView.getGTypes());
	gtk_tree_view_set_model(textStyleView.get(), GTK_TREE_MODEL(textStyleStore));
	g_object_unref(textStyleStore);

	//gtk_container_add(GTK_CONTAINER(scroll),GTK_WIDGET(textStyleView.get()) );
	gtk_box_append(GTK_BOX(box2),GTK_WIDGET(textStyleView.get()));
	

	gtk_box_append(GTK_BOX(box),box2);

		// Available styles
	addOption_gui(textStyleStore, wsm, _("General text"),
			"text-general-fore-color", "text-general-back-color", "text-general-bold", "text-general-italic");

	addOption_gui(textStyleStore, wsm, _("My nick"),
			"text-mynick-fore-color", "text-mynick-back-color", "text-mynick-bold", "text-mynick-italic");

		addOption_gui(textStyleStore, wsm, _("My own message"),
			"text-myown-fore-color", "text-myown-back-color", "text-myown-bold", "text-myown-italic");

		addOption_gui(textStyleStore, wsm, _("Private message"),
			"text-private-fore-color", "text-private-back-color", "text-private-bold", "text-private-italic");

		addOption_gui(textStyleStore, wsm, _("System message"),
			"text-system-fore-color", "text-system-back-color", "text-system-bold", "text-system-italic");

		addOption_gui(textStyleStore, wsm, _("Status message"),
			"text-status-fore-color", "text-status-back-color", "text-status-bold", "text-status-italic");
		//Cheat
		addOption_gui(textStyleStore, wsm, _("Cheat message"),
			"text-cheat-fore-color", "text-cheat-back-color", "text-cheat-bold", "text-cheat-italic");

		addOption_gui(textStyleStore, wsm, _("Timestamp"),
			"text-timestamp-fore-color", "text-timestamp-back-color", "text-timestamp-bold", "text-timestamp-italic");

		addOption_gui(textStyleStore, wsm, _("URL"),
			"text-url-fore-color", "text-url-back-color", "text-url-bold", "text-url-italic");
		//IP adr.
		addOption_gui(textStyleStore, wsm, _("IP address"),
			"text-ip-fore-color", "text-ip-back-color", "text-ip-bold", "text-ip-italic");

		addOption_gui(textStyleStore, wsm, _("Favorite User"),
			"text-fav-fore-color", "text-fav-back-color", "text-fav-bold", "text-fav-italic");

		addOption_gui(textStyleStore, wsm, _("Operator"),
			"text-op-fore-color", "text-op-back-color", "text-op-bold", "text-op-italic");

// the reference count on the buffer is not incremented and caller of this function won't own a new reference.
		textView = gtk_text_view_new();
		textStyleBuffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(textView));
		gtk_text_view_set_buffer(GTK_TEXT_VIEW(textView), textStyleBuffer);
		gtk_box_append(GTK_BOX(box2),GTK_WIDGET(textView));

 		// Preview style
		GtkTreeIter treeIter;
		GtkTreeModel *m = GTK_TREE_MODEL(textStyleStore);
		gboolean valid = gtk_tree_model_get_iter_first(m, &treeIter);

		GtkTextIter textIter, textStartIter, textEndIter;
		string line, style;

		string timestamp = "[" + Util::getShortTimeString() + "] ";
		const gint ts_strlen = g_utf8_strlen(timestamp.c_str(), -1);
		string fore = wsm->getString("text-timestamp-fore-color");
		string back = wsm->getString("text-timestamp-back-color");
		int bold = wsm->getInt("text-timestamp-bold");
		int italic = wsm->getInt("text-timestamp-italic");

		GtkTextTag *tag = NULL;
		GtkTextTag *tagTimeStamp = gtk_text_buffer_create_tag(textStyleBuffer, _("Timestamp"),
			"background", back.c_str(),
			"foreground", fore.c_str(),
			"weight", bold ? TEXT_WEIGHT_BOLD : TEXT_WEIGHT_NORMAL,
			"style", italic ? TEXT_STYLE_ITALIC : TEXT_STYLE_NORMAL,
			NULL);

		gint row_count = 0;

		while (valid)
		{
			style = textStyleView.getString(&treeIter, _("Style"));
			fore = wsm->getString(textStyleView.getString(&treeIter, "keyForeColor"));
			back = wsm->getString(textStyleView.getString(&treeIter, "keyBackColor"));
			bold = wsm->getInt(textStyleView.getString(&treeIter, "keyBolt"));
			italic = wsm->getInt(textStyleView.getString(&treeIter, "keyItalic"));
			const gint st_strlen = g_utf8_strlen(style.c_str(), -1);

			line = timestamp + style + "\n";

			gtk_text_buffer_get_end_iter(textStyleBuffer, &textIter);
			gtk_text_buffer_insert(textStyleBuffer, &textIter, line.c_str(), line.size());

			textStartIter = textEndIter = textIter;

			// apply text style
			gtk_text_iter_backward_chars(&textStartIter, st_strlen + 1);

			if (row_count != 7)
				tag = gtk_text_buffer_create_tag(textStyleBuffer, style.c_str(),
					"background", back.c_str(),
					"foreground", fore.c_str(),
					"weight", bold ? TEXT_WEIGHT_BOLD : TEXT_WEIGHT_NORMAL,
					"style", italic ? TEXT_STYLE_ITALIC : TEXT_STYLE_NORMAL,
					NULL);
			else
				tag = tagTimeStamp;

			gtk_text_buffer_apply_tag(textStyleBuffer, tag, &textStartIter, &textEndIter);

			// apply timestamp style
			gtk_text_iter_backward_chars(&textStartIter, ts_strlen);
			gtk_text_iter_backward_chars(&textEndIter, st_strlen + 2);
			gtk_text_buffer_apply_tag(textStyleBuffer, tagTimeStamp, &textStartIter, &textEndIter);

			row_count++;

			valid = gtk_tree_model_iter_next(m, &treeIter);
		}
		toggle_autors = gtk_check_button_new_with_label("Bold Autors");
		
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(toggle_autors), WGETB("text-bold-autors"));
		//[BMDC
		//[BMDC
		string strcolor = SETTING(BACKGROUND_CHAT_COLOR);

		gtk_widget_set_name(textView,"prewienTextView");
//		GtkCssProvider *provider = gtk_css_provider_new ();
//		GdkDisplay *display = gdk_display_get_default ();
//		GdkScreen *screen = gdk_display_get_default_screen (display);
//		std::string t_css = std::string("#prewienTextView text { background: "+strcolor+" ;}\n\0");
//		gtk_css_provider_load_from_data (GTK_CSS_PROVIDER (provider),t_css.c_str(),-1, NULL);
//
//		gtk_style_context_add_provider_for_screen (screen,
//								GTK_STYLE_PROVIDER(provider),
//								GTK_STYLE_PROVIDER_PRIORITY_USER);
//		g_object_unref (provider);

		gtk_box_append(GTK_BOX(box),toggle_autors);
		GtkWidget *grid = gtk_grid_new();
		bFore = gtk_button_new_with_label("Foreground");
		bBack = gtk_button_new_with_label("Background");
		bStyle = gtk_button_new_with_label("Font&Style");
		bBW = gtk_button_new_with_label("Black&White");
		bDef = gtk_button_new_with_label("Default Styles");
		bBaAll = gtk_button_new_with_label("Whole Background");//TODO ?
		gtk_box_append(GTK_BOX(box),grid);

		gtk_grid_attach(GTK_GRID(grid),bFore,0,0,1,1);
		gtk_grid_attach(GTK_GRID(grid),bBack,1,0,1,1);
		gtk_grid_attach(GTK_GRID(grid),bStyle,2,0,1,1);
		gtk_grid_attach(GTK_GRID(grid),bBW,3,0,1,1);
		gtk_grid_attach(GTK_GRID(grid),bDef,4,0,1,1);
		gtk_grid_attach(GTK_GRID(grid),bBaAll,5,0,1,1);


	g_signal_connect(bBaAll, "clicked", G_CALLBACK(onTextBackGroundChat), (gpointer)this);
	g_signal_connect(bFore, "clicked", G_CALLBACK(onTextColorForeClicked_gui), (gpointer)this);
	g_signal_connect(bBack, "clicked", G_CALLBACK(onTextColorBackClicked_gui), (gpointer)this);
	g_signal_connect(bBW, "clicked", G_CALLBACK(onTextColorBWClicked_gui), (gpointer)this);
	g_signal_connect(bStyle, "clicked", G_CALLBACK(onTextStyleClicked_gui), (gpointer)this);
	g_signal_connect(bDef, "clicked", G_CALLBACK(onTextStyleDefaultClicked_gui), (gpointer)this);

}


/* Adds a colors and fonts options */

void ChatPage::addOption_gui(GtkListStore *store, WulforSettingsManager *wsm, const string &name,
	const string &key1, const string &key2, const string &key3, const string &key4)
{
 	GtkTreeIter iter;
	gtk_list_store_append(store, &iter);
	gtk_list_store_set(store, &iter,
		0, name.c_str(),
		1, wsm->getString(key1).c_str(),
		2, wsm->getString(key2).c_str(),
		3, wsm->getInt(key3),
		4, wsm->getInt(key4),
		5, key1.c_str(),
		6, key2.c_str(),
		7, key3.c_str(),
		8, key4.c_str(),
		-1);
}


void ChatPage::onTextColorForeClicked_gui(GtkWidget *widget, gpointer data)
{
	ChatPage *s = (ChatPage *)data;
	s->selectTextColor_gui(0);
}

void ChatPage::onTextColorBackClicked_gui(GtkWidget *widget, gpointer data)
{
	ChatPage *s = (ChatPage *)data;
	s->selectTextColor_gui(1);
}

void ChatPage::onTextColorBWClicked_gui(GtkWidget *widget, gpointer data)
{
	ChatPage *s = (ChatPage *)data;
	s->selectTextColor_gui(2);
}

void ChatPage::onTextStyleClicked_gui(GtkWidget *widget, gpointer data)
{
	ChatPage *s = (ChatPage *)data;
	s->selectTextStyle_gui(0);
}

void ChatPage::onTextStyleDefaultClicked_gui(GtkWidget *widget, gpointer data)
{
	ChatPage *s = (ChatPage *)data;
	s->selectTextStyle_gui(1);
}


void ChatPage::selectTextColor_gui(const int select)
{
	GtkTreeIter iter;

	if (select == 2)
	{
		/* black and white style */
		GtkTreeModel *m = GTK_TREE_MODEL(textStyleStore);
		gboolean valid = gtk_tree_model_get_iter_first(m, &iter);

		string style = "";
		GtkTextTag *tag = NULL;
		GtkTextTagTable *tag_table = gtk_text_buffer_get_tag_table(textStyleBuffer);

		while (valid)
		{
			gtk_list_store_set(textStyleStore, &iter,
				textStyleView.col("ForeColor"), "#000000",
				textStyleView.col("BackColor"), "#FFFFFF", -1);

			style = textStyleView.getString(&iter, _("Style"));
			tag = gtk_text_tag_table_lookup(tag_table, style.c_str());

			if (tag)
				g_object_set(tag, "foreground", "#000000", "background", "#FFFFFF", NULL);

			valid = gtk_tree_model_iter_next(m, &iter);
		}

		gtk_widget_queue_draw(textView);

		return;
	}

	GtkTreeSelection *selection = gtk_tree_view_get_selection(textStyleView.get());

	if (!gtk_tree_selection_get_selected(selection, NULL, &iter))
	{
		return;
	}

	GdkRGBA color;
	string currentcolor = "";
	GtkWidget *dialog = gtk_color_chooser_dialog_new(_("Select Color"),NULL);

	if (select == 0)
		currentcolor = textStyleView.getString(&iter, "ForeColor");
	else
		currentcolor = textStyleView.getString(&iter, "BackColor");

	if (gdk_rgba_parse(&color,currentcolor.c_str()))
		gtk_color_chooser_set_rgba(GTK_COLOR_CHOOSER(dialog), &color);

	/*gint response = gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_hide(dialog);

	if (response == GTK_RESPONSE_OK)
	{
		gtk_color_chooser_get_rgba(GTK_COLOR_CHOOSER(dialog), &color);

		string ground = "";
		string strcolor = WulforUtil::colorToString(&color);
		string style = textStyleView.getString(&iter, _("Style"));

		if (select == 0)
		{
			ground = "foreground-rgba";
			gtk_list_store_set(textStyleStore, &iter, textStyleView.col("ForeColor"), strcolor.c_str(), -1);
		}
		else
		{
			ground = "background-rgba";
			gtk_list_store_set(textStyleStore, &iter, textStyleView.col("BackColor"), strcolor.c_str(), -1);
		}

		GtkTextTag *tag = gtk_text_tag_table_lookup(gtk_text_buffer_get_tag_table(textStyleBuffer), style.c_str());

		if (tag)
			g_object_set(tag, ground.c_str(), &color, NULL);

		gtk_widget_queue_draw(textView);
	}*/
}

void ChatPage::selectTextStyle_gui(const int select)
{
	GtkTreeIter iter;
	int bolt, italic;
	GtkTextTag *tag = NULL;
	string style = "";

	WulforSettingsManager *wsm = WulforSettingsManager::getInstance();

	if (select == 1)
	{
		/* default style */
		GtkTreeModel *m = GTK_TREE_MODEL(textStyleStore);
		gboolean valid = gtk_tree_model_get_iter_first(m, &iter);
		GtkTextTagTable *tag_table = gtk_text_buffer_get_tag_table(textStyleBuffer);
		string fore, back;

		while (valid)
		{
			style = textStyleView.getString(&iter, _("Style"));
			fore = wsm->getString(textStyleView.getString(&iter, "keyForeColor"), true);
			back = wsm->getString(textStyleView.getString(&iter, "keyBackColor"), true);
			bolt = wsm->getInt(textStyleView.getString(&iter, "keyBolt"), true);
			italic = wsm->getInt(textStyleView.getString(&iter, "keyItalic"), true);

			gtk_list_store_set(textStyleStore, &iter,
				textStyleView.col("ForeColor"), fore.c_str(),
				textStyleView.col("BackColor"), back.c_str(),
				textStyleView.col("Bolt"), bolt,
				textStyleView.col("Italic"), italic,
				-1);

			tag = gtk_text_tag_table_lookup(tag_table, style.c_str());

			if (tag)
				g_object_set(tag,
					"foreground", fore.c_str(),
					"background", back.c_str(),
					"weight", bolt ? TEXT_WEIGHT_BOLD : TEXT_WEIGHT_NORMAL,
					"style", italic ? TEXT_STYLE_ITALIC : TEXT_STYLE_NORMAL,
					NULL);

			valid = gtk_tree_model_iter_next(m, &iter);
		}

		gtk_widget_queue_draw(textView);

		return;
	}

	GtkTreeSelection *selection = gtk_tree_view_get_selection(textStyleView.get());

	if (!gtk_tree_selection_get_selected(selection, NULL, &iter))
	{

		return;
	}
	/*GtkWidget *dialog = gtk_font_chooser_dialog_new (_("Select Font"),NULL); 	
	gint response = gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_hide(dialog);

	if (response == GTK_RESPONSE_OK)
	{
		gchar *temp =  gtk_font_chooser_get_font(GTK_FONT_CHOOSER(dialog)); 

		if (temp)
		{
			string font_name = temp;
			g_free(temp);

			bolt = font_name.find("Bold") != string::npos ? 1 : 0;
			italic = font_name.find("Italic") != string::npos ? 1 : 0;

			style = textStyleView.getString(&iter, _("Style"));
			gtk_list_store_set(textStyleStore, &iter, textStyleView.col("Bolt"), bolt, textStyleView.col("Italic"), italic, -1);

			tag = gtk_text_tag_table_lookup(gtk_text_buffer_get_tag_table(textStyleBuffer), style.c_str());

			if (tag)
				g_object_set(tag,
					"weight", bolt ? TEXT_WEIGHT_BOLD : TEXT_WEIGHT_NORMAL,
					"style", italic ? TEXT_STYLE_ITALIC : TEXT_STYLE_NORMAL,
					NULL);
			gtk_widget_queue_draw(textView);
		}
	}*/
}


void ChatPage::onTextBackGroundChat(GtkWidget *widget , gpointer data)
{
	/*ChatPage *s = (ChatPage *)data;
	GtkWidget *dialog = gtk_color_chooser_dialog_new(_("Set Color"),NULL);

	GdkRGBA color;
	if (gdk_rgba_parse(&color,SETTING(BACKGROUND_CHAT_COLOR).c_str()))
		gtk_color_chooser_set_rgba(GTK_COLOR_CHOOSER(dialog), &color);

	gint response = gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_hide(dialog);

	if (response == GTK_RESPONSE_OK)
	{
		gtk_color_chooser_get_rgba(GTK_COLOR_CHOOSER(dialog),&color);
		string strcolor = WulforUtil::colorToString(&color);
		SettingsManager::getInstance()->set(SettingsManager::BACKGROUND_CHAT_COLOR, strcolor);
		SettingsManager::getInstance()->save();*/
//		GtkCssProvider *provider = gtk_css_provider_new ();
//		GdkDisplay *display = gdk_display_get_default ();
//		GdkScreen *screen = gdk_display_get_default_screen (display);
//		std::string t_css = std::string("#prewienTextView { background: "+strcolor+" ;}\n\0");
//		gtk_css_provider_load_from_data (GTK_CSS_PROVIDER (provider),t_css.c_str(),-1, NULL);
//
//		gtk_style_context_add_provider_for_screen (screen,
//											GTK_STYLE_PROVIDER(provider),
//											GTK_STYLE_PROVIDER_PRIORITY_USER);
//		g_object_unref (provider);
//	}
}

void ChatPage::write()
{
	WulforSettingsManager *wsm = WulforSettingsManager::getInstance();
	GtkTreeIter iter;
	GtkTreeModel *m = GTK_TREE_MODEL(textStyleStore);
	gboolean valid = gtk_tree_model_get_iter_first(m, &iter);

	while (valid)
	{
		wsm->set(textStyleView.getString(&iter, "keyForeColor"), textStyleView.getString(&iter, "ForeColor"));
		wsm->set(textStyleView.getString(&iter, "keyBackColor"), textStyleView.getString(&iter, "BackColor"));
		wsm->set(textStyleView.getString(&iter, "keyBolt"), textStyleView.getValue<int>(&iter, "Bolt"));
		wsm->set(textStyleView.getString(&iter, "keyItalic"), textStyleView.getValue<int>(&iter, "Italic"));

		valid = gtk_tree_model_iter_next(m, &iter);
	}

	WSET("text-bold-autors", gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(toggle_autors)));

}
