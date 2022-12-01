/*
 * Copyright (C) 2011 - 2025 - BMDC
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
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _BMDC_SPLASH_HH_
#define _BMDC_SPLASH_HH_

#include <iostream>
#include <gtk/gtk.h>
#include "../dcpp/Util.h"
#include "genres.h"

using namespace std;
using namespace dcpp;

class Splash
{
	public:
		Splash() :
		 Text("") ,percentage(0),win(NULL), label(NULL), box(NULL), image(NULL), progressbar(NULL) { }
		void show() {
			win = gtk_window_new();
			label = gtk_label_new("Loading...");
			progressbar = gtk_progress_bar_new ();
			box = gtk_box_new(GTK_ORIENTATION_VERTICAL,0);
			
			GResource* res = ::bmdc_get_resource();
			g_resources_register(res);
			image = gtk_image_new_from_resource("/org/bmdc-team/bmdc/icons/hicolor/96x96/apps/bmdc.png");
			
			gtk_box_append(GTK_BOX(box), image);
			gtk_box_append(GTK_BOX(box), label);
			gtk_box_append(GTK_BOX(box), progressbar);
			gtk_window_set_child(GTK_WINDOW(win), box);

			gtk_widget_show(win);

			update();
		}
		~Splash() {	win = NULL;label= NULL;box= NULL;image= NULL;progressbar= NULL; }

	void setText(const string &text)
	{
		if(text.empty()) return;
			Text = text;
		cout << "Loading: " << text << endl;
	}
	void setPercentage(const float& ii)
	{
		percentage = ii;
	}					

	void update()
	{
		gtk_label_set_text(GTK_LABEL(label),("Loading ..."+Text+" "+Util::toString(percentage*100)+" %").c_str());
		gtk_progress_bar_set_fraction (GTK_PROGRESS_BAR(progressbar),percentage);
		while (g_main_context_iteration(NULL, FALSE));
	}
	void destroy() { gtk_widget_hide(win); }

	private:
		float percentage;
		string Text;
		GtkWidget *win;
		GtkWidget *label;
		GtkWidget *box;
		GtkWidget *image;
		GtkWidget *progressbar;

};
#else
class Splash;
#endif
