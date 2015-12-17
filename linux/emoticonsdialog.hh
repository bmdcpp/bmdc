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

#ifndef _BMDC_EMOTICONS_DIALOG_HH
#define _BMDC_EMOTICONS_DIALOG_HH
#include <dcpp/stdinc.h>
#include <dcpp/Util.h>
#include <map>
#include "emoticons.hh"

class EmoticonsDialog
{
	public:
		EmoticonsDialog(GtkWidget *chat, GtkWidget *button, GtkWidget *menu, std::string packName = SETTING(EMOT_PACK), const std::string& address = dcpp::Util::emptyString);
		~EmoticonsDialog();

		// GUI functions
		void showEmotDialog_gui();
		void buildEmotMenu_gui();
		std::string getCurrent(const std::string& address) { return getEmot(address)->getCurrPackName_gui();}
		Emoticons *getEmot(const std::string &address = dcpp::Util::emptyString);
	private:
		enum {FIRST, x16 = FIRST, x22, x24, x32, x36, x48, x64, DEFAULT, LAST};

		GtkWidget *Chat;       // chat entry
		GtkWidget *Button;     // chat emoticons button
		GtkWidget *Menu;       // packs menu
		GtkWidget *dialog;     // emoticons dialog

		int icon_width;
		int icon_height;
		std::string currIconSize;
		std::string packName;
		std::string address;
		std::map<std::string,Emoticons*> hubs;
		static const std::string sizeIcon[LAST];

		void build();
		void position();
		void graber();
		void addPacksMenu(GtkWidget *item);
		void addIconSizeMenu(GtkWidget *item);
		void setCurrIconSize(const std::string &size);

		//GUI callback functions
		static void onChat(GtkWidget *widget /*button*/, gpointer data /*this*/);
		static void onCheckPacksMenu(GtkMenuItem *checkItem, gpointer data);
		static void onCheckIconSizeMenu(GtkMenuItem *checkItem, gpointer data);
		static gboolean event(GtkWidget *widget /*dialog*/, GdkEvent *event, gpointer data /*this*/);

};

#else
class EmoticonsDialog;
#endif
