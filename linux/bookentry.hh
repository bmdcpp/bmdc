/*
 * Copyright © 2004-2012 Jens Oknelid, paskharen@gmail.com
 * Copyright © 2010-2013 Mank freedcpp <at> seznam <dot> cz
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

#ifndef _BMDC_BOOK_ENTRY_HH
#define _BMDC_BOOK_ENTRY_HH

#include "entry.hh"

class BookEntry : public Entry
{
	public:
		BookEntry(): eventBox(NULL), labelBox(NULL), tabMenuItem(NULL),	closeButton(NULL),
				label(NULL), fItem(NULL), bold(false), urgent(false), labelSize(20), icon(NULL) , popTabMenuItem(NULL),
				type((EntryType)0), IsCloseButton(true)  { }
		BookEntry(const EntryType type, const std::string &text, const std::string &glade, const std::string &id = "");
		virtual ~BookEntry()
		{	}

		GtkWidget *getContainer(); //@ return Main Container of Book
		GtkWidget *getLabelBox() { return labelBox; }
		GtkWidget *getCloseButton() { return closeButton; }
		GtkWidget *getTabMenuItem() { return tabMenuItem; }
		void setIcon_gui(const EntryType type);
		void setBackForeGround(const EntryType type);//@ Setting BackGround and ForeGround of Book
		void setBackForeGround_unread(const EntryType type);
		void setIcon_gui(const std::string stock);
		void setIconPixbufs_gui(const std::string iconspath);
		void setLabel_gui(std::string text);
		const std::string& getLabelText() const;
		void setBold_gui();
		void setUrgent_gui();
		void setActive_gui();
		bool isActive_gui();
		virtual void show() = 0;
		virtual GtkWidget *createmenu();
		GtkWidget *getFItem() { return fItem;}
		
		void setSearchButtons(bool s) { IsCloseButton = s;}
	private:
		void updateLabel_gui();
		static void onCloseItem(gpointer data);
		void removeBooK_GUI();
		GtkWidget *createItemFirstMenu();
		static gboolean onButtonPressed_gui(GtkWidget *widget, GdkEventButton *event, gpointer data);
		static gboolean onButtonReleased_gui(GtkWidget *widget, GdkEventButton *event, gpointer data);
		std::string labelText;
		std::string truncatedLabelText;
		GtkWidget *eventBox;
		GtkWidget *labelBox;
		GtkWidget *tabMenuItem;
		GtkWidget *closeButton;
		GtkLabel *label;
		GtkWidget *fItem;

		bool bold;
		bool urgent;
		const glong labelSize;//@ size of Chars in Tab value in WulforSettingsManager 
		GtkWidget *icon;
		//[BMDC++
		GdkEventType previous;
		GtkWidget *popTabMenuItem;
		const EntryType type;
		bool IsCloseButton;
};

#else
class BookEntry;
#endif
