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

#define gen gtk_entry_new()

#define sw_new gtk_scrolled_window_new(NULL,NULL)

#define b_file_dialog_widget(a) gtk_file_chooser_dialog_new (a, NULL, GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER,\
                                      "_Cancel",\
                                      GTK_RESPONSE_CANCEL,\
                                      "_Open",\
                                      GTK_RESPONSE_OK,\
                                      NULL);
