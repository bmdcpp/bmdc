 /*
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
// @because GtkFactory -> GtkIconTheme and stock -> icon_name
#include <gtk/gtk.h>
#if GTK_CHECK_VERSION(3,9,0)
	#define BMDC_STOCK_CANCEL "_Cancel"
	#define BMDC_STOCK_REMOVE "list-remove"
	#define BMDC_STOCK_OPEN "_Open"
	#define BMDC_STOCK_OK "_OK"
	#define BMDC_STOCK_NO "_No"
	#define BMDC_STOCK_YES "_Yes"
	#define BMDC_STOCK_DIALOG_QUESTION "dialog-question"
	#define BMDC_STOCK_FIND  "edit-find"
	#define BMDC_STOCK_FILE "text-x-generic"
	#define BMDC_STOCK_DIRECTORY "folder"
	#define BMDC_STOCK_GO_DOWN "go-down"
	#define BMDC_STOCK_GO_UP "go-up"
	#define BMDC_STOCK_HOME "go-home"
	#define BMDC_STOCK_CONVERT "convert"
	#define BMDC_STOCK_PREFERENCES  "preferences-system"
	#define BMDC_STOCK_NETWORK "network-workgroup"
	#define BMDC_STOCK_CONNECT "gtk-connect"
	#define BMDC_STOCK_QUIT "application-exit"
#else
	#pragma GCC diagnostic push
	#pragma GCC diagnostic ignored "-Wdeprecated-declarations"	
	#define BMDC_STOCK_CANCEL GTK_STOCK_CANCEL
	#define BMDC_STOCK_REMOVE GTK_STOCK_REMOVE 
	#define BMDC_STOCK_OPEN GTK_STOCK_OPEN
	#define BMDC_STOCK_OK GTK_STOCK_OK
	#define BMDC_STOCK_NO GTK_STOCK_NO
	#define BMDC_STOCK_YES GTK_STOCK_YES
	#define BMDC_STOCK_DIALOG_QUESTION GTK_STOCK_DIALOG_QUESTION
	#define BMDC_STOCK_FIND  GTK_STOCK_FIND
	#define BMDC_STOCK_FILE GTK_STOCK_FILE
	#define BMDC_STOCK_DIRECTORY GTK_STOCK_DIRECTORY
	#define BMDC_STOCK_GO_DOWN GTK_STOCK_GO_DOWN
	#define BMDC_STOCK_GO_UP GTK_STOCK_GO_UP
	#define BMDC_STOCK_HOME GTK_STOCK_HOME
	#define BMDC_STOCK_CONVERT GTK_STOCK_CONVERT
	#define BMDC_STOCK_PREFERENCES  GTK_STOCK_PREFERENCES
	#define BMDC_STOCK_NETWORK GTK_STOCK_NETWORK
	#define BMDC_STOCK_CONNECT GTK_STOCK_CONNECT
	#define BMDC_STOCK_QUIT GTK_STOCK_QUIT
	#pragma GCC diagnostic pop		
#endif


