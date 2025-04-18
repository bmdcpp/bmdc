/*
* Copyright © 2011-2025 BMDC
* This file is part of BMDC.
* BMDC is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 2 of the License, or (at your option) any later version.
*
* BMDC is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License along with Xfce-nameday-plugin. If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef _BMDC_BOOK_ENTRY_HH
#define _BMDC_BOOK_ENTRY_HH

#include "entry.hh"
#include <gtk/gtk.h>

class BookEntry : public Entry
{
	public:
		BookEntry(): eventBox(NULL), labelBox(NULL), tabMenuItem(NULL),	closeButton(NULL),
				label(NULL), bCreated(true),bold(false), urgent(false), labelSize(20), icon(NULL) , popTabMenuItem(NULL), type((EntryType)0), bIsCloseButton(true)  { }
		BookEntry(const EntryType type, const std::string &text, const std::string &glade, const std::string &id = dcpp::Util::emptyString  );
		virtual ~BookEntry()
		{

		}

		GtkWidget *getContainer(); //@ return Main Container of Book
		GtkWidget *getLabelBox() { return labelBox; }
		GtkWidget *getCloseButton() { return closeButton; }
		GMenu *getTabMenuItem() { return tabMenuItem; }
		void setIcon_gui(const EntryType type);
		void setBackForeGround(const EntryType type); //@ Setting BackGround and ForeGround of BookEntry
		void setIcon_gui(const std::string stock);
		void setIconPixbufs_gui(const std::string iconspath);
		void setLabel_gui(const std::string text);
		const std::string& getLabelText() const;
		void setBold_gui();
		void setUrgent_gui();
		void setActive_gui();
		bool isActive_gui();
		virtual void show() = 0;
		virtual GMenu *createmenu();

		void setSearchButtons(bool s) { bIsCloseButton = s;}
		void setName(const std::string& name)
		{ h_name = name; }
		
		void setUnread() //@set flag for tab
		{
			gtk_widget_set_state_flags (labelBox,GTK_STATE_FLAG_ACTIVE,TRUE);
		}
		
		void setNormal()
		{
			gtk_widget_set_state_flags (labelBox,GTK_STATE_FLAG_NORMAL,TRUE);
		}
		int getPositionTab() { return (-1);}
	protected:
		GMenuItem *createItemFirstMenu();
	private:
		void updateLabel_gui();
		static void onCloseItem(GtkWidget*,GVariant* ,gpointer data);
		void removeBooK_GUI();
		//@NOTE: For CSS
		std::string getName();
		static void onButtonPressed_gui(GtkGestureClick *gesture,
                                   int                n_press,
                                   double             x,
                                   double             y,
                                   gpointer         *data);
		static void onButtonReleased_gui(GtkGestureClick *gesture,
                                    int              n_press,
                                    double           x,
                                    double           y,
                                    GtkWidget       *widget);
		std::string labelText;
		std::string truncatedLabelText;
		std::string h_name;
		GtkWidget *eventBox;
		GtkWidget *labelBox;
		GMenu *tabMenuItem;
		GtkWidget *closeButton;
		GtkLabel *label;
		bool bCreated; //@ if menu created
		bool bold;
		bool urgent;
		const glong labelSize; //@ size of Chars in Tab value in WulforSettingsManager
		GtkWidget *icon;

		GdkEventType previous;
		GtkWidget *popTabMenuItem;
		const EntryType type;
		bool bIsCloseButton;
		static const GActionEntry book_entries[];
};

#else
class BookEntry;
#endif
