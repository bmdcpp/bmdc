// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
// MA 02110-1301, USA.
// 

#include "QuenePage.hh"
#include "seUtil.hh"
#include <dcpp/SettingsManager.h>

using namespace dcpp;

const char* QuenePage::name_page = "→ Queue";

void QuenePage::show(GtkWidget *parent, GtkWidget* old)
{

	table = gtk_grid_new();
	GtkWidget *frame = gtk_frame_new("Priority");
	gtk_grid_attach(GTK_GRID(table),frame,0,0,1,1);
	GtkWidget *box = gtk_grid_new();
	gtk_container_add(GTK_CONTAINER(frame),box);

	spin_low = gtk_spin_button_new_with_range(10,100000000000,1);
	spin_normal = gtk_spin_button_new_with_range(10,100000000000,1);
	spin_higlest = gtk_spin_button_new_with_range(10,100000000000,1);
	spin_higt = gtk_spin_button_new_with_range(10,100000000000,1);
	gtk_grid_attach(GTK_GRID(box),gtk_label_new("Low"),0,0,1,1);
	gtk_grid_attach(GTK_GRID(box),spin_low,1,0,1,1);

	gtk_grid_attach(GTK_GRID(box),gtk_label_new("KiB"),2,0,1,1);
	
	gtk_grid_attach(GTK_GRID(box),gtk_label_new("Normal"),0,1,1,1);
	gtk_grid_attach(GTK_GRID(box),spin_normal,1,1,1,1);

	gtk_grid_attach(GTK_GRID(box),gtk_label_new("KiB"),2,1,1,1);
	
	gtk_grid_attach(GTK_GRID(box),gtk_label_new("Higlest"),0,2,1,1);
	gtk_grid_attach(GTK_GRID(box),spin_higlest,1,2,1,1);

	gtk_grid_attach(GTK_GRID(box),gtk_label_new("KiB"),2,2,1,1);
	
	gtk_grid_attach(GTK_GRID(box),gtk_label_new("High"),0,3,1,1);
	gtk_grid_attach(GTK_GRID(box),spin_higt,1,3,1,1);

	gtk_grid_attach(GTK_GRID(box),gtk_label_new("KiB"),2,3,1,1);
	
	// Auto-priority
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(spin_higlest), (double)SETTING(PRIO_HIGHEST_SIZE));
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(spin_higt), (double)SETTING(PRIO_HIGH_SIZE));
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(spin_normal), (double)SETTING(PRIO_NORMAL_SIZE));
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(spin_low), (double)SETTING(PRIO_LOW_SIZE));
	
	GtkWidget *frame2 = gtk_frame_new("Auto-Drop");
	gtk_grid_attach(GTK_GRID(table),frame2,0,1,1,1);
	GtkWidget *box2 = gtk_grid_new();
	gtk_container_add(GTK_CONTAINER(frame2),box2);

	GtkWidget* label_speed = gtk_label_new("B/s");

	spinSpeedDrop = gtk_spin_button_new_with_range(0,100000000000,1);
	spinElapse = gtk_spin_button_new_with_range(0,500,1);
	spinMinSources = gtk_spin_button_new_with_range(0,10000000,1);
	spinIntervalCheck = gtk_spin_button_new_with_range(0,100000,1);
	spinInactivity = gtk_spin_button_new_with_range(0,100000000,1);
	spinSizeDrop = gtk_spin_button_new_with_range(0,100000000,1);
	// Auto-drop
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(spinSpeedDrop), (double)SETTING(AUTODROP_SPEED));
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(spinElapse), (double)SETTING(AUTODROP_ELAPSED));
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(spinMinSources), (double)SETTING(AUTODROP_MINSOURCES));
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(spinIntervalCheck), (double)SETTING(AUTODROP_INTERVAL));
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(spinInactivity), (double)SETTING(AUTODROP_INACTIVITY));
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(spinSizeDrop), (double)SETTING(AUTODROP_FILESIZE));
	GtkWidget *label_DropSpeed = gtk_label_new("Speed");
	gtk_grid_attach(GTK_GRID(box2),label_DropSpeed,0,0,1,1);
	gtk_grid_attach(GTK_GRID(box2),spinSpeedDrop,1,0,1,1);
	gtk_grid_attach(GTK_GRID(box2),label_speed,2,0,1,1);
	GtkWidget *label_el = gtk_label_new("Elapse");
	gtk_grid_attach(GTK_GRID(box2),label_el,0,1,1,1);
	gtk_grid_attach(GTK_GRID(box2),spinElapse,1,1,1,1);
	gtk_grid_attach(GTK_GRID(box2),gtk_label_new("s"),2,1,1,1);
	GtkWidget *label_min_s = gtk_label_new("Min Sources");
	gtk_grid_attach(GTK_GRID(box2),label_min_s,0,2,1,1);
	gtk_grid_attach(GTK_GRID(box2),spinMinSources,1,2,1,1);
	GtkWidget *label_inter = gtk_label_new("IntervalCheck");
	gtk_grid_attach(GTK_GRID(box2),label_inter,0,3,1,1);
	gtk_grid_attach(GTK_GRID(box2),spinIntervalCheck,1,3,1,1);
	gtk_grid_attach(GTK_GRID(box2),gtk_label_new("s"),2,3,1,1);
	GtkWidget *label_ina = gtk_label_new("Inactivity");
	gtk_grid_attach(GTK_GRID(box2),label_ina,0,4,1,1);
	gtk_grid_attach(GTK_GRID(box2),spinInactivity,1,4,1,1);
	gtk_grid_attach(GTK_GRID(box2),gtk_label_new("s"),2,4,1,1);
	GtkWidget *label_size = gtk_label_new("Size");
	gtk_grid_attach(GTK_GRID(box2),label_size,0,5,1,1);
	gtk_grid_attach(GTK_GRID(box2),spinSizeDrop,1,5,1,1);
	gtk_grid_attach(GTK_GRID(box2),gtk_label_new("KiB"),2,5,1,1);
	GtkWidget *box3 = gtk_scrolled_window_new(NULL,NULL);

	qView = TreeView();//workaround for if selected double time
	SEUtil::createOptionsView_gui(qView,queueStore);
	gtk_container_add(GTK_CONTAINER(box3),GTK_WIDGET(qView.get()));
	gtk_grid_attach(GTK_GRID(table),box3,0,6,10,8);

	gtk_grid_set_column_homogeneous (GTK_GRID(table),TRUE);
	gtk_widget_set_size_request(box3,-1,200);
	
	SEUtil::addOption_gui(queueStore, _("Set lowest priority for newly added files larger than low priority size"), SettingsManager::PRIO_LOWEST);
	SEUtil::addOption_gui(queueStore, _("Auto-drop slow sources for all queue items (except filelists)"), SettingsManager::AUTODROP_ALL);
	SEUtil::addOption_gui(queueStore, _("Remove slow filelists"), SettingsManager::AUTODROP_FILELISTS);
	SEUtil::addOption_gui(queueStore, _("Don't remove the source when auto-dropping, only disconnect"), SettingsManager::AUTODROP_DISCONNECT);
	SEUtil::addOption_gui(queueStore, _("Automatically search for alternative download locations"), SettingsManager::AUTO_SEARCH);
	SEUtil::addOption_gui(queueStore, _("Automatically match queue for auto search hits"), SettingsManager::AUTO_SEARCH_AUTO_MATCH);
	SEUtil::addOption_gui(queueStore, _("Skip zero-byte files"), SettingsManager::SKIP_ZERO_BYTE);
	SEUtil::addOption_gui(queueStore, _("Don't download files already in share"), SettingsManager::DONT_DL_ALREADY_SHARED);
	SEUtil::addOption_gui(queueStore, _("Don't download files already in the queue"), SettingsManager::DONT_DL_ALREADY_QUEUED);

	/*@Add to parent*/
	SEUtil::reAddItemCo(parent,old,table);
}

void QuenePage::write()
{
	SettingsManager *sm = SettingsManager::getInstance();
	SEUtil::saveOptionsView_gui(qView,queueStore,sm);
	// Queue
	// Auto-priority
	sm->set(SettingsManager::PRIO_HIGHEST_SIZE, Util::toString(gtk_spin_button_get_value(GTK_SPIN_BUTTON(spin_higlest ))));
	sm->set(SettingsManager::PRIO_HIGH_SIZE, Util::toString(gtk_spin_button_get_value(GTK_SPIN_BUTTON(spin_higt))));
	sm->set(SettingsManager::PRIO_NORMAL_SIZE, Util::toString(gtk_spin_button_get_value(GTK_SPIN_BUTTON(spin_normal))));
	sm->set(SettingsManager::PRIO_LOW_SIZE, Util::toString(gtk_spin_button_get_value(GTK_SPIN_BUTTON(spin_low))));
	// Auto-drop
	sm->set(SettingsManager::AUTODROP_SPEED, Util::toString(gtk_spin_button_get_value(GTK_SPIN_BUTTON(spinSpeedDrop))));
	sm->set(SettingsManager::AUTODROP_ELAPSED, Util::toString(gtk_spin_button_get_value(GTK_SPIN_BUTTON(spinElapse))));
	sm->set(SettingsManager::AUTODROP_MINSOURCES, Util::toString(gtk_spin_button_get_value(GTK_SPIN_BUTTON(spinMinSources))));
	sm->set(SettingsManager::AUTODROP_INTERVAL, Util::toString(gtk_spin_button_get_value(GTK_SPIN_BUTTON(spinIntervalCheck))));
	sm->set(SettingsManager::AUTODROP_INACTIVITY, Util::toString(gtk_spin_button_get_value(GTK_SPIN_BUTTON(spinInactivity))));
	sm->set(SettingsManager::AUTODROP_FILESIZE, Util::toString(gtk_spin_button_get_value(GTK_SPIN_BUTTON(spinSizeDrop))));
}
