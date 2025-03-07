/*
* Copyright © 2011-2025 BMDC
* This file is part of BMDC.
* BMDC is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 2 of the License, or (at your option) any later version.
*
* BMDC is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License along with Xfce-nameday-plugin. If not, see <https://www.gnu.org/licenses/>.
*/

#include "System.hh"
#include "wulformanager.hh"
#include "GuiUtil.hh"
#include "settingsmanager.hh"
#include "../dcpp/LogManager.h"

using namespace std;
using namespace dcpp;

SystemLog::SystemLog():
BookEntry(Entry::SYSTEML,_("System Log"),"system"),
 buffer(NULL),sysMark(NULL)
{
	
	WulforUtil::setTextDeufaults(getWidget("systextview"),SETTING(BACKGROUND_CHAT_COLOR),"",false,"","SystemLog");
	WulforUtil::setTextColor(WGETS("text-system-fore-color"), string("SystemLog"), getWidget("systextview"));

	buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (getWidget("systextview")));
	gtk_text_buffer_get_end_iter(buffer, &iter);
	sysMark = gtk_text_buffer_create_mark(buffer, NULL, &iter, FALSE);

	GtkAdjustment *adjustment = gtk_scrolled_window_get_vadjustment(GTK_SCROLLED_WINDOW(getWidget("sysscroll")));

	g_signal_connect(adjustment, "value_changed", G_CALLBACK(onScroll_gui), (gpointer)this);
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

void SystemLog::add_gui(time_t t, string message,int sev)
{
	gtk_text_buffer_move_mark(buffer, sysMark, &iter);
	gtk_text_buffer_get_end_iter(buffer, &iter);
	gtk_text_buffer_insert_paintable(buffer, &iter , getImageSev(sev));
	gtk_text_buffer_move_mark(buffer, sysMark, &iter);
	
	string line = "[ " + Util::getShortTimeString(t)+" ] " + message + "\n\0";
	
	if(!g_utf8_validate(line.c_str(),-1,NULL))
			return;
	gsize oread,owrite;
	gchar*	buf = g_filename_to_utf8(line.c_str(),-1,&oread,&owrite,NULL);

	gtk_text_buffer_insert(buffer, &iter, buf, strlen(buf));
	gtk_text_buffer_get_end_iter(buffer, &iter);
	gtk_text_buffer_move_mark(buffer, sysMark, &iter);
	// Limit size of chat text
	if (gtk_text_buffer_get_line_count(buffer) > maxLines + 1)
	{
		GtkTextIter next;
		gtk_text_buffer_get_start_iter(buffer, &iter);
		gtk_text_buffer_get_iter_at_line(buffer, &next, 1);
		gtk_text_buffer_delete(buffer, &iter, &next);
		gtk_text_view_place_cursor_onscreen(GTK_TEXT_VIEW(getWidget("systextview")));//did we need this?
		return;
	}

	if(gtk_text_buffer_get_char_count (buffer) > 25000)
	{
		GtkTextIter startIter, endIter;
		gtk_text_buffer_get_start_iter(buffer, &startIter);
		gtk_text_buffer_get_end_iter(buffer, &endIter);
		gtk_text_buffer_delete(buffer, &startIter, &endIter);
		gtk_text_view_place_cursor_onscreen(GTK_TEXT_VIEW(getWidget("systextview")));
		return;
	}

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

void SystemLog::onClearButton(GtkWidget*, gpointer data)
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
		add_gui(i->second.tim, i->first , i->second.sev);
	}
}

void SystemLog::show()
{
    ini_client();
}

void SystemLog::on(LogManagerListener::Message, time_t t, const string& message,int sev) noexcept
{
    typedef Func3<SystemLog,time_t,std::string,int> F3;
    F3 *func = new F3(this,&SystemLog::add_gui, t, message, sev);
    WulforManager::get()->dispatchGuiFunc(func);
}

GdkPaintable* SystemLog::getImageSev(int sev)
{
	string src = dcpp::Util::emptyString;
	switch(sev)
	{
		case LogManager::Sev::LOW: 
			src = "/org/bmdc-team/bmdc/info/info.png";
			break;
		case LogManager::Sev::NORMAL: 
			src = "/org/bmdc-team/bmdc/info/warning.png";
			break;
		case LogManager::Sev::HIGH: 
			src = "/org/bmdc-team/bmdc/info/error.png";
			break;
		default:break;
	};
	GtkWidget* icon = gtk_image_new_from_resource(src.c_str());
	g_object_set(icon ,"icon-size" ,GTK_ICON_SIZE_NORMAL, NULL);
	return gtk_image_get_paintable(GTK_IMAGE(icon));
}

