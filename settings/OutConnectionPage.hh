/**/
/*
#ifndef _OUT_CONN_SETTINGS_
#define _OUT_CONN_SETTINGS_

#include <gtk/gtk.h>
#include "SettingsPage.hh"

class OutConnectionPage: public SettingsPage
{
		public:
			void show(GtkWidget *parent, GtkWidget* old);
			const char* get_name_page()
			{ return name_page; }
			void write();
			virtual GtkWidget* getTopWidget(){ return frame;}
		private:
			static const char* name_page;
			GtkWidget* radio_direct,*radio_sock,
			*label_ip,*label_port,
			*label_username,*label_password,
			*entry_ip,*entry_sport,
			*entry_username,*entry_password,
			*check_hostname,
			*frame;
			static void onOutDirect_gui(GtkToggleButton *button, gpointer data);
			static void onSocks5_gui(GtkToggleButton *button, gpointer data);

};
#else
class OutConnectionPage;
#endif
*/