/* 
* (C) 2025 BMDC
* This file is part of BMDC.
* BMDC is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 2 of the License, or (at your option) any later version.
*
* BMDC is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License along with Xfce-nameday-plugin. If not, see <https://www.gnu.org/licenses/>.
*/

#define gen gtk_entry_new()

#define sw_new gtk_scrolled_window_new()

#define b_file_dialog_widget(a) gtk_file_chooser_dialog_new (a, NULL, GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER,\
                                      "_Cancel",\
                                      GTK_RESPONSE_CANCEL,\
                                      "_Open",\
                                      GTK_RESPONSE_OK,\
                                      NULL);
