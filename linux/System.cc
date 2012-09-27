//	System.cc
//      Copyright 2011-2012 Mank <freedcpp at seznam dot cz>
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


#include "System.hh"
#include "wulformanager.hh"
#include "WulforUtil.hh"

#include <dcpp/LogManager.h>

using namespace std;
using namespace dcpp;

systemlog::systemlog():
BookEntry(Entry::SYSTEML,_("System Log"),"system.glade"),
 buffer(NULL),sysMark(NULL)
{
	buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (getWidget("systextview")));
	gtk_text_buffer_get_end_iter(buffer, &iter);
	sysMark = gtk_text_buffer_create_mark(buffer, NULL, &iter, FALSE);

	GtkAdjustment *adjustment = gtk_scrolled_window_get_vadjustment(GTK_SCROLLED_WINDOW(getWidget("sysscroll")));

	g_signal_connect(adjustment, "value_changed", G_CALLBACK(onScroll_gui), (gpointer)this);
	g_signal_connect(adjustment, "changed", G_CALLBACK(onResize_gui), (gpointer)this);
	g_signal_connect(getWidget("buttonClear"), "clicked", G_CALLBACK(onClearButton), (gpointer)this);
}

systemlog::~systemlog()
{
	LogManager::getInstance()->removeListener(this);
}

void systemlog::add_gui(time_t t, string file)
{
	string line;
	line = "";
	gtk_text_buffer_get_end_iter(buffer, &iter);

	line = Text::toUtf8("[ " + Util::getShortTimeString(t)+" ] " + file + "\n\0");

	gtk_text_buffer_insert(buffer, &iter, line.c_str(), line.size());
	gtk_text_buffer_get_end_iter(buffer, &iter);

	// Limit size of chat text
	if (gtk_text_buffer_get_line_count(buffer) > maxLines + 1)
	{
		GtkTextIter next;
		gtk_text_buffer_get_start_iter(buffer, &iter);
		gtk_text_buffer_get_iter_at_line(buffer, &next, 1);
		gtk_text_buffer_delete(buffer, &iter, &next);
	}
	gtk_text_view_place_cursor_onscreen(GTK_TEXT_VIEW(getWidget("systextview")));
}

void systemlog::onScroll_gui(GtkAdjustment *adjustment, gpointer data)
{
    systemlog *sys = (systemlog *)data;
    gdouble value = gtk_adjustment_get_value(adjustment);
    sys->scrollToBottom = value >= (adjustment->upper-adjustment->page_size);
}

void systemlog::onResize_gui(GtkAdjustment *adjustment, gpointer data)
{
    systemlog *sys = (systemlog *)data;
    gdouble value = gtk_adjustment_get_value(adjustment);

    if (sys->scrollToBottom && value < (adjustment->upper-adjustment->page_size))
    {
        GtkTextIter iter;
        gtk_text_buffer_get_end_iter(sys->buffer, &iter);
        gtk_text_buffer_move_mark(sys->buffer, sys->sysMark, &iter);
        gtk_text_view_scroll_to_mark(GTK_TEXT_VIEW(sys->getWidget("systextview")), sys->sysMark, 0, FALSE, 0, 0);
    }
}

void systemlog::onClearButton(GtkWidget *widget, gpointer data)
{
	systemlog *sys = (systemlog *)data;
	GtkTextIter startIter, endIter;
	gtk_text_buffer_get_start_iter(sys->buffer, &startIter);
	gtk_text_buffer_get_end_iter(sys->buffer, &endIter);
	gtk_text_buffer_delete(sys->buffer, &startIter, &endIter);

}
void systemlog::ini_client()
{
	LogManager::List oldMessages = LogManager::getInstance()->getLastLogs();

	// Technically, we might miss a message or two here, but who cares...
	LogManager::getInstance()->addListener(this);
	for(LogManager::List::const_iterator i = oldMessages.begin(); i != oldMessages.end(); ++i) {
		add_gui(i->first, Text::toT(i->second));
	}
}

void systemlog::show()
{
    ini_client();
}

void systemlog::on(LogManagerListener::Message, time_t t, const string& message) noexcept
{
    typedef Func2<systemlog,time_t,std::string> F2;
    F2 *func = new F2(this,&systemlog::add_gui, t, message);
    WulforManager::get()->dispatchGuiFunc(func);
}
