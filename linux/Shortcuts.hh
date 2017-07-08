#pragma once
#include "entry.hh"
#include "wulformanager.hh"

class ShortCuts: public Entry
{
	public:
	ShortCuts() : Entry(Entry::SHORTCUTS,"shortcuts")
	{
		
	
	}
	~ShortCuts() {}
	GtkWidget* getContainer(){return getWidget("shortcuts-bmdc");}
	void show()
	{
		GtkWidget *overlay = GTK_WIDGET (getWidget("shortcuts-bmdc"));
		gtk_window_set_transient_for (GTK_WINDOW (overlay), GTK_WINDOW (WulforManager::get()->getMainWindow()->getContainer()));
		g_object_set (overlay, "view-name", NULL, NULL);
		gtk_widget_show (overlay);
		
		g_signal_connect (overlay, "destroy",
                        G_CALLBACK (gtk_widget_destroyed), &overlay);
	}
	
};
