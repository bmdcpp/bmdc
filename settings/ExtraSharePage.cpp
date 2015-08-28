/**/
#include <dcpp/SettingsManager.h>
#include <dcpp/ShareManager.h>
#include <linux/wulformanager.hh>
#include "definitons.hh"
#include "seUtil.hh"
#include "ExtraSharePage.hh"

using namespace dcpp;
using namespace std;

const char* OSharingPage::page_name = "→ Other";

void OSharingPage::show(GtkWidget *parent, GtkWidget* old)
{
	grid = gtk_grid_new();
	check_follow =  gtk_check_button_new_with_label ("Follow symlink");
	check_hiden  = 	gtk_check_button_new_with_label ("Share Hidden files");
	gtk_grid_attach(GTK_GRID(grid),check_hiden,0,0,1,1);
	gtk_grid_attach(GTK_GRID(grid),check_follow,1,0,1,1);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_hiden), SETTING(SHARE_HIDDEN));
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_follow), SETTING(FOLLOW_LINKS));

	spin_slots = gtk_spin_button_new_with_range (1, 1000, 1);
	spin_slots_extra = gtk_spin_button_new_with_range (1, 90, 1);

	gtk_grid_attach(GTK_GRID(grid),gtk_label_new("Slots"),0,1,1,1);
	gtk_grid_attach(GTK_GRID(grid),spin_slots,1,1,1,1);
	gtk_grid_attach(GTK_GRID(grid),gtk_label_new("Extra Slots if speed below:"),0,2,1,1);
	gtk_grid_attach(GTK_GRID(grid),spin_slots_extra,1,2,1,1);

	gtk_spin_button_set_value(GTK_SPIN_BUTTON(spin_slots_extra), (double)SETTING(MIN_UPLOAD_SPEED));
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(spin_slots), (int)SETTING(SLOTS_PRIMARY));

	entry_skiplist_path = gen;
	entry_skiplist_ext = gen;
	entry_skiplist_reg = gen;

	gtk_grid_attach(GTK_GRID(grid),gtk_label_new("Skiplist extension: "),0,3,1,1);
	gtk_grid_attach(GTK_GRID(grid),entry_skiplist_ext,1,3,1,1);
	gtk_grid_attach(GTK_GRID(grid),gtk_label_new("Skiplist path: "),0,4,1,1);
	gtk_grid_attach(GTK_GRID(grid),entry_skiplist_path,1,4,1,1);

	gtk_grid_attach(GTK_GRID(grid),gtk_label_new("RegExp filename Skiplist: "),0,5,1,1);
	gtk_grid_attach(GTK_GRID(grid),entry_skiplist_reg,1,5,1,1);

	gtk_entry_set_text(GTK_ENTRY(entry_skiplist_ext),SETTING(SHARING_SKIPLIST_EXTENSIONS).c_str());
	gtk_entry_set_text(GTK_ENTRY(entry_skiplist_path),SETTING(SHARING_SKIPLIST_PATHS).c_str());
	gtk_entry_set_text(GTK_ENTRY(entry_skiplist_reg),SETTING(SHARING_SKIPLIST_REGEX).c_str());

	spin_size_low = gtk_spin_button_new_with_range (0, 9999, 1);
	spin_size_high = gtk_spin_button_new_with_range (0, 9999, 1);

	gtk_grid_attach(GTK_GRID(grid),gtk_label_new("Skip Size Below: "),0,6,1,1);
	gtk_grid_attach(GTK_GRID(grid),spin_size_low,1,6,1,1);

	gtk_grid_attach(GTK_GRID(grid),gtk_label_new("Skip Size Above: "),0,7,1,1);
	gtk_grid_attach(GTK_GRID(grid),spin_size_high,1,7,1,1);

	gtk_spin_button_set_value(GTK_SPIN_BUTTON(spin_size_low), (double)SETTING(SHARING_SKIPLIST_MINSIZE));
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(spin_size_high), (double)SETTING(SHARING_SKIPLIST_MAXSIZE));

	g_signal_connect(check_hiden, "toggled", G_CALLBACK(onShareHiddenPressed_gui), (gpointer)this);

	SEUtil::reAddItemCo(parent,old,grid);
}

gboolean OSharingPage::onShareHiddenPressed_gui(GtkToggleButton *togglebutton, gpointer data)
{
	OSharingPage *s = (OSharingPage *)data;

	bool show = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(s->check_hiden));
	s->shareHidden_client(show);
	return FALSE;
}

void OSharingPage::shareHidden_client(bool show)
{
	SettingsManager::getInstance()->set(SettingsManager::SHARE_HIDDEN, show);
	ShareManager::getInstance()->setDirty();
	ShareManager::getInstance()->refresh(TRUE, FALSE, FALSE);
}

void OSharingPage::write()
{
	SettingsManager *sm = SettingsManager::getInstance();

	const gchar* s_paths =	gtk_entry_get_text(GTK_ENTRY(entry_skiplist_path));
	if(s_paths)
		sm->set(SettingsManager::SHARING_SKIPLIST_PATHS, s_paths);

	const gchar* s_ext = gtk_entry_get_text(GTK_ENTRY(entry_skiplist_ext));
	if(s_ext)
		sm->set(SettingsManager::SHARING_SKIPLIST_EXTENSIONS, s_ext);
	const gchar* s_reg = gtk_entry_get_text(GTK_ENTRY(entry_skiplist_reg));	
	if(s_reg)
		sm->set(SettingsManager::SHARING_SKIPLIST_REGEX ,s_reg);

	sm->set(SettingsManager::FOLLOW_LINKS, gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_follow)));

	sm->set(SettingsManager::SLOTS_PRIMARY, gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(spin_slots)));

	sm->set(SettingsManager::MIN_UPLOAD_SPEED, gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(spin_slots_extra)));
	sm->set(SettingsManager::SHARING_SKIPLIST_MINSIZE,gtk_spin_button_get_value(GTK_SPIN_BUTTON(spin_size_low)));
	sm->set(SettingsManager::SHARING_SKIPLIST_MAXSIZE,gtk_spin_button_get_value(GTK_SPIN_BUTTON(spin_size_high)));
	ShareManager::getInstance()->updateFilterCache();
}
	
