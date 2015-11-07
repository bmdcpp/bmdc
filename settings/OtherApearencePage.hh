#ifndef _OAPP_PAGE_
#define _OAPP_PAGE_
#include <gtk/gtk.h>
#include <vector>
#include <string>
#include "SettingsPage.hh"
/*-----------------------------------------------------------------------------*/
class OApearencePage: public SettingsPage
{
	public:
		OApearencePage();
		~OApearencePage(){ }
		void show(GtkWidget *parent, GtkWidget* old);
		const char* get_name_page()
		{ return page_name;};
		void write();
		GtkWidget* getTopWidget(){return grid;}
	private:
		std::string getNameAction(int num);
		static const char* page_name;
		GtkWidget *entry_country,*entry_timestamp,
		*entry_away, *entry_ripe,*entry_ratio,*entry_chat_info,
		*grid;
		GtkComboBoxText *actionUL;
		std::vector<std::string> usersAction;

};
#else
class OApearencePage;
#endif
/*-------------------------------------------------------------------------------*/
