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
 
#ifdef HAVE_LIBTAR

#ifndef _EXPORT_DIALOG_HH
#define _EXPORT_DIALOG_HH

#include "../dcpp/stdinc.h"
#include "../dcpp/DCPlusPlus.h"
#include "../dcpp/RegEx.h"
#include "treeview.hh"
#include "dialogentry.hh"

class ExportDialog:
	public DialogEntry
{
	public:
		ExportDialog(GtkWindow *parent);
		~ExportDialog();
	
	private:
		static void onButtonExportedClicked(GtkWidget *widget, gpointer data);
		static void onGetPathGui(GtkWidget *widget, gpointer data);
		static void onToggledClicked_gui(GtkCellRendererToggle *cell, gchar *path, gpointer data);
		
		TreeView exportView;
		GtkListStore* exportStore;
		GtkTreeSelection *exportSelection;
	
};
#else
class ExportDialog;
#endif

#endif
