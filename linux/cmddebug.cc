/*
 * Copyright © 2010-2015 Mank <freedcpp <at> seznam <dot> cz>
 * Copyright © 2010-2011 Eugene Petrov <dhamp@ya.ru>
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

#include "cmddebug.hh"
#include "wulformanager.hh"
#include "WulforUtil.hh"
#include "settingsmanager.hh"
#include <dcpp/DebugManager.h>

using namespace std;
using namespace dcpp;

cmddebug::cmddebug():
BookEntry(Entry::CMD,_("CMD"),"cmddebug"),
stop(false)
{
	WulforUtil::setTextDeufaults(getWidget("cmdtextview"),WGETS("background-color-chat"));
    
    buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (getWidget("cmdtextview")));
    gtk_text_buffer_get_end_iter(buffer, &iter);
    cmdMark = gtk_text_buffer_create_mark(buffer, NULL, &iter, FALSE);
    gboolean hubin = WGETB("cmd-debug-hub-in");
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(getWidget("hub_in_button")) , hubin);//TRUE
    gboolean hubout = WGETB("cmd-debug-hub-out");
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(getWidget("hub_out_button")) , hubout);//TRUE
    gboolean clientin = WGETB("cmd-debug-client-in");
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(getWidget("client_in_button")) , clientin);//TRUE
    gboolean clientout = WGETB("cmd-debug-client-out");
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(getWidget("client_out_button")) , clientout);//TRUE
    gboolean detection = WGETB("cmd-debug-detection");
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(getWidget("detection_button")) , detection);//FALSE
    
    GtkAdjustment *adjustment = gtk_scrolled_window_get_vadjustment(GTK_SCROLLED_WINDOW(getWidget("cmdscroll")));
    g_signal_connect(adjustment, "value_changed", G_CALLBACK(onScroll_gui), (gpointer)this);
    g_signal_connect(adjustment, "changed", G_CALLBACK(onResize_gui), (gpointer)this);
    g_signal_connect(getWidget("buttonClear"), "clicked", G_CALLBACK(onClearButton), (gpointer)this);
    
    ClientManager *clientMgr = ClientManager::getInstance();
    {
		auto lock = clientMgr->lock();
		clientMgr->addListener(this);
		auto& clients = clientMgr->getClients();
		int i = 0;
		for(auto it = clients.begin(); it != clients.end(); ++it) {
			Client* client = *it;
			if(!client->isConnected())
				continue;
			gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(getWidget("comboboxadr")),client->getIpPort().c_str());
			iters.insert(Iters::value_type(client->getIpPort(), i));	
			i++;
		}		
	}
	DebugManager::getInstance()->addListener(this);
}

cmddebug::~cmddebug()
{
	stop = true;
	s.signal();
	WSET("cmd-debug-hub-out", gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(getWidget("hub_out_button"))));
	WSET("cmd-debug-hub-in", gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(getWidget("hub_in_button"))));
	WSET("cmd-debug-client-out", gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(getWidget("client_out_button"))));
	WSET("cmd-debug-client-in", gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(getWidget("client_in_button"))));
	WSET("cmd-debug-detection", gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(getWidget("detection_button"))));
	DebugManager::getInstance()->removeListener(this);
	ClientManager::getInstance()->removeListener(this);
}

void cmddebug::add_gui(time_t t, string file)
{
    string line;

    gtk_text_buffer_get_end_iter(buffer, &iter);

    line = Text::toUtf8("[" + Util::getShortTimeString(t) + "]" + file + "\n\0");

    gtk_text_buffer_insert(buffer, &iter, line.c_str(), line.size());
    gtk_text_buffer_get_end_iter(buffer, &iter);

    // Limit size of text
    if (gtk_text_buffer_get_line_count(buffer) > maxLines + 1)
    {
            GtkTextIter next;
            gtk_text_buffer_get_start_iter(buffer, &iter);
            gtk_text_buffer_get_iter_at_line(buffer, &next, 1);
            gtk_text_buffer_delete(buffer, &iter, &next);
    }
    gtk_text_view_place_cursor_onscreen(GTK_TEXT_VIEW(getWidget("cmdtextview")));

}

void cmddebug::ini_client()
{
    start();
}

void cmddebug::show()
{
    ini_client();
}

void cmddebug::on(ClientConnected, Client* c) noexcept {
	
	typedef Func2<cmddebug, Client*, bool> F2;
	F2 *func = new F2(this,&cmddebug::UpdateCombo,c, true);
	WulforManager::get()->dispatchGuiFunc(func);
}

void cmddebug::on(ClientDisconnected, Client* c) noexcept {
	
	typedef Func2<cmddebug, Client*, bool> F2;
	F2 *func = new F2(this,&cmddebug::UpdateCombo,c, false);
	WulforManager::get()->dispatchGuiFunc(func);
}

void cmddebug::UpdateCombo(Client* c, bool add)
{
	GtkWidget *widget = getWidget("comboboxadr");
	int i = 0;
	if(add)
	{ 	
		if(iters.find(c->getHubUrl()) == iters.end())
		{
			if(!c->isConnected())
				return;
			gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(widget),c->getIpPort().c_str());
			i = iters.size()+1;
			iters.insert(Iters::value_type(c->getIpPort(),i));		
		}
	}else {
		if(c == NULL)
			return;
		string url = c->getIpPort();   
		auto it = iters.find(url);
		if(it != iters.end())
		{
			i = it->second;
			gtk_combo_box_text_remove(GTK_COMBO_BOX_TEXT(widget),i);
			iters.erase(url);	
		}
	}
}
		
void cmddebug::onScroll_gui(GtkAdjustment *adjustment, gpointer data)
{
    cmddebug *cmd = (cmddebug *)data;
    gdouble value = gtk_adjustment_get_value(adjustment);
    cmd->scrollToBottom = value >= ( gtk_adjustment_get_upper(adjustment) - gtk_adjustment_get_page_size (adjustment));
}

void cmddebug::onResize_gui(GtkAdjustment *adjustment, gpointer data)
{
    cmddebug *cmd = (cmddebug *)data;
    gdouble value = gtk_adjustment_get_value(adjustment);

    if (cmd->scrollToBottom && value < (gtk_adjustment_get_upper(adjustment) - gtk_adjustment_get_page_size (adjustment)))
    {
        GtkTextIter iter;
        gtk_text_buffer_get_end_iter(cmd->buffer, &iter);
        gtk_text_buffer_move_mark(cmd->buffer, cmd->cmdMark, &iter);
        gtk_text_view_scroll_to_mark(GTK_TEXT_VIEW(cmd->getWidget("cmdtextview")), cmd->cmdMark, 0, FALSE, 0, 0);
    }
}

void cmddebug::onClearButton(GtkWidget *widget, gpointer data)
{
	cmddebug *cmd = (cmddebug *)data;
	GtkTextIter startIter, endIter;
	gtk_text_buffer_get_start_iter(cmd->buffer, &startIter);
	gtk_text_buffer_get_end_iter(cmd->buffer, &endIter);
	gtk_text_buffer_delete(cmd->buffer, &startIter, &endIter);
	
}
