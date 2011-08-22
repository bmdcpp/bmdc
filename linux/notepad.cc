//      notepad.cc
//
//      Copyright 2011 Mank <mank@no-ip.sk>
//
//      This program is free software; you can redistribute it and/or modify
//      it under the terms of the GNU General Public License as published by
//      the Free Software Foundation; either version 2 of the License, or
//      (at your option) any later version.
//
//      This program is distributed in the hope that it will be useful,
//      but WITHOUT ANY WARRANTY; without even the implied warranty of
//      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//      GNU General Public License for more details.
//
//      You should have received a copy of the GNU General Public License
//      along with this program; if not, write to the Free Software
//      Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
//      MA 02110-1301, USA.


#include "notepad.hh"
#include "wulformanager.hh"
#include "WulforUtil.hh"

using namespace std;
using namespace dcpp;


notepad::notepad():
BookEntry(Entry::NOTEPAD,_("Notepad"),"notepad.glade"),
usefile(false),
file("")
{
	buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (getWidget("textview1")));
}

notepad::~notepad()
{
	GtkTextIter start;
	GtkTextIter end;
	
	gchar *text;
	/* Obtain iters for the start and end of points of the buffer */
	gtk_text_buffer_get_start_iter (buffer, &start);
	gtk_text_buffer_get_end_iter (buffer, &end);

	/* Get the entire buffer text. */

	text = gtk_text_buffer_get_text (buffer, &start, &end, FALSE);
	std::string stext ;
	stext.assign(text);

  try {
	string configFile;
	
	if(!usefile)
	{
		configFile = dcpp::Util::getNotepadFile();
	}
	else
		configFile = file;

	File out(configFile + ".tmp", File::WRITE, File::CREATE | File::TRUNCATE);
		out.write(stext);
		out.flush();
		out.close();
		File::deleteFile(configFile);
		File::renameFile(configFile + ".tmp", configFile);

	}
	catch (const Exception &e)
	{ }
  g_free (text);

}

void notepad::add_gui(string file)
{
	gtk_text_buffer_set_text (buffer, file.c_str(), -1);
}

void notepad::ini_client()
{
	try {
		string path;
	
		if(!usefile) {
			path = dcpp::Util::getNotepadFile();
		}
		else
			path=file;


		File f(path,File::READ,File::OPEN);
		//add to GUI
		add_gui(f.read());

		f.close();

	}
	catch (const Exception &e)
	{
		dcdebug("Failed Load Notepad.txt");
	}
}

void notepad::show()
{
    ini_client();
}

/*this is a pop menu*/
void notepad::popmenu()
{
    GtkWidget *closeMenuItem = gtk_menu_item_new_with_label(_("Close"));
    gtk_menu_shell_append(GTK_MENU_SHELL(getNewTabMenu()), closeMenuItem);

    g_signal_connect_swapped(closeMenuItem, "activate", G_CALLBACK(onCloseItem), (gpointer)this);

}

void notepad::onCloseItem(gpointer data)
{
    BookEntry *entry = (BookEntry *)data;
    WulforManager::get()->getMainWindow()->removeBookEntry_gui(entry);

}
