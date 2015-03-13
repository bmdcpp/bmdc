//      Copyright 2011-2015 Mank <freedcpp at seznam dot cz>
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
#include "settingsmanager.hh"
#include <dcpp/LogManager.h>

using namespace std;
using namespace dcpp;

SystemLog::SystemLog():
BookEntry(Entry::SYSTEML,_("System Log"),"system"),
 buffer(NULL),sysMark(NULL)
{
	WulforUtil::setTextDeufaults(getWidget("systextview"),SETTING(BACKGROUND_CHAT_COLOR),dcpp::Util::emptyString,false,dcpp::Util::emptyString,"SystemLog");
    WulforUtil::setTextColor(string("black"),string("SystemLog"));//TODO: Settings

	buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (getWidget("systextview")));
	gtk_text_buffer_get_end_iter(buffer, &iter);
	sysMark = gtk_text_buffer_create_mark(buffer, NULL, &iter, FALSE);

	GtkAdjustment *adjustment = gtk_scrolled_window_get_vadjustment(GTK_SCROLLED_WINDOW(getWidget("sysscroll")));

	g_signal_connect(adjustment, "value_changed", G_CALLBACK(onScroll_gui), (gpointer)this);
	g_signal_connect(adjustment, "changed", G_CALLBACK(onResize_gui), (gpointer)this);
	g_signal_connect(getWidget("buttonClear"), "clicked", G_CALLBACK(onClearButton), (gpointer)this);
}

void SystemLog::preferences_gui()
{
	WulforUtil::setTextDeufaults(getWidget("systextview"),SETTING(BACKGROUND_CHAT_COLOR));
}

SystemLog::~SystemLog()
{
	LogManager::getInstance()->removeListener(this);
}

void SystemLog::add_gui(time_t t, string file)
{
	gtk_text_buffer_get_end_iter(buffer, &iter);

	string line = Text::toUtf8("[ " + Util::getShortTimeString(t)+" ] " + file + "\n\0");

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

void SystemLog::onScroll_gui(GtkAdjustment *adjustment, gpointer data)
{
    SystemLog *sys = (SystemLog *)data;
    gdouble value = gtk_adjustment_get_value(adjustment);
    sys->scrollToBottom = value >= ( gtk_adjustment_get_upper(adjustment) - gtk_adjustment_get_page_size (adjustment));
}

void SystemLog::onResize_gui(GtkAdjustment *adjustment, gpointer data)
{
    SystemLog *sys = (SystemLog *)data;
    gdouble value = gtk_adjustment_get_value(adjustment);

    if (sys->scrollToBottom && value < (gtk_adjustment_get_upper(adjustment) - gtk_adjustment_get_page_size (adjustment)))
    {
        GtkTextIter iter;
        gtk_text_buffer_get_end_iter(sys->buffer, &iter);
        gtk_text_buffer_move_mark(sys->buffer, sys->sysMark, &iter);
        gtk_text_view_scroll_to_mark(GTK_TEXT_VIEW(sys->getWidget("systextview")), sys->sysMark, 0, FALSE, 0, 0);
    }
}

void SystemLog::onClearButton(GtkWidget *widget, gpointer data)
{
	SystemLog *sys = (SystemLog *)data;
	GtkTextIter startIter, endIter;
	gtk_text_buffer_get_start_iter(sys->buffer, &startIter);
	gtk_text_buffer_get_end_iter(sys->buffer, &endIter);
	gtk_text_buffer_delete(sys->buffer, &startIter, &endIter);
	LogManager::getInstance()->clearLogs();

}
void SystemLog::ini_client()
{
	LogManager::List oldMessages = LogManager::getInstance()->getLastLogs();

	// Technically, we might miss a message or two here, but who cares...
	LogManager::getInstance()->addListener(this);
	for(auto i = oldMessages.begin(); i != oldMessages.end(); ++i) {
		add_gui(i->first, Text::toT(i->second));
	}
}

void SystemLog::show()
{
    ini_client();
}

void SystemLog::on(LogManagerListener::Message, time_t t, const string& message,int sev) noexcept
{
#ifndef _DEBUG
	if(sev == LogManager::Sev::NORMAL)
	{
#endif
    typedef Func2<SystemLog,time_t,std::string> F2;
    F2 *func = new F2(this,&SystemLog::add_gui, t, message);
    WulforManager::get()->dispatchGuiFunc(func);
 #ifndef _DEBUG
	}
#endif
}
