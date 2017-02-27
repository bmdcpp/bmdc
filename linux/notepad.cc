//      Copyright 2011 - 2017 Mank <freedcpp at seznam dot cz>
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

Notepad::Notepad():
BookEntry(Entry::NOTEPAD, _("Notepad"), "notepad")
{
	WulforUtil::setTextDeufaults(getWidget("textview1"),SETTING(BACKGROUND_CHAT_COLOR),string(),false,string(),"Notepad");
	WulforUtil::setTextColor(string("black"),string("Notepad"));
	buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (getWidget("textview1")));
}

Notepad::~Notepad()
{
	GtkTextIter start;
	GtkTextIter end;

	gchar *text = NULL;
	/* Obtain iters for the start and end of points of the buffer */
	gtk_text_buffer_get_start_iter (buffer, &start);
	gtk_text_buffer_get_end_iter (buffer, &end);
	/* Get the entire buffer text. */
	text = gtk_text_buffer_get_text (buffer, &start, &end, FALSE);

    try {
		string configFile = dcpp::Util::getNotepadFile();
		File out(configFile + ".tmp", File::WRITE, File::CREATE | File::TRUNCATE);
		out.write(string(text));
		out.flush();
		out.close();
		File::deleteFile(configFile);
		File::renameFile(configFile + ".tmp", configFile);
	}
	catch (const Exception &e)
	{ }
	g_free (text);

}

void Notepad::add_gui(string file)
{
	gtk_text_buffer_set_text (buffer, file.c_str(), -1);
}

void Notepad::ini_client()
{
	try {
		string path = dcpp::Util::getNotepadFile();

		File f(path,File::READ,File::OPEN);
		//add to GUI
		add_gui(f.read());

		f.close();
	}
	catch (const Exception &e)
	{
		dcdebug("Failed Load Notepad.txt %s",e.what());
	}
}

void Notepad::show()
{
    ini_client();
}
