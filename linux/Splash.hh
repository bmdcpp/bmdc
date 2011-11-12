/*(C) Mank <Mank1 at seznam dot cz */
#ifndef SPLASH_HH
#define SPLASH_HH
using namespace std;
class Splash
{
	public:
		Splash() { }
		void show() {
			win = gtk_window_new(GTK_WINDOW_TOPLEVEL);
			gtk_window_set_decorated(GTK_WINDOW(win),FALSE);
			gtk_window_set_default_size(GTK_WINDOW(win),350,20);
			gtk_window_set_skip_taskbar_hint(GTK_WINDOW(win),TRUE);
			gtk_window_set_keep_above(GTK_WINDOW(win), TRUE);
			gtk_window_set_position(GTK_WINDOW(win),GTK_WIN_POS_CENTER);
			label = gtk_label_new("Loading...");
			box = gtk_vbox_new(TRUE, 0);
			image = gtk_image_new_from_file(_DATADIR "/icons/hicolor/scalable/apps/bmdc.svg");
			gtk_container_add(GTK_CONTAINER(box),image);
			gtk_container_add(GTK_CONTAINER(box),label);
			gtk_container_add(GTK_CONTAINER(win),box);
			gtk_widget_show_now(image);
			gtk_widget_show_now(label);
			gtk_widget_show_now(box);
			gtk_widget_show_now(win);
			update();	
		}
		~Splash() {	}
	
	void setText(const string &text) { if(text.empty())return;
								Text = text;
							}
	
	void update() { 	
						gtk_label_set_text(GTK_LABEL(label),("Loading ..."+Text).c_str()); 
						 while (gtk_events_pending ())
							gtk_main_iteration_do (FALSE);
						::sleep(2);	
				 }
	void destroy() {gtk_widget_destroy(win);}
	private: 
		string Text;
		GtkWidget *win;
		GtkWidget *label;
		GtkWidget *box;
		GtkWidget *image;
		
};
#else
class Splash;
#endif
