/*
 * Copyright © 2004-2015 Jens Oknelid, paskharen@gmail.com
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
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * In addition, as a special exception, compiling, linking, and/or
 * using OpenSSL with this program is allowed.
 */

#include "entry.hh"
#include <dcpp/stdinc.h>
#include <dcpp/DCPlusPlus.h>
#include <dcpp/Util.h>
#include "wulformanager.hh"

using namespace std;

Entry::Entry(const EntryType type, const string &ui, const string &id):
	xml(NULL),
	type(type),
	id(dcpp::Util::toString(type) + ":" + id)
{
	if(!ui.empty()) {
	// Load the Builder XML file, if applicable
	string file = WulforManager::get()->getPath() + "/ui/" + ui + ".glade.ui";
#if !GTK_CHECK_VERSION(3, 10, 0)
	GError *error = NULL;
	xml = gtk_builder_new();
	gtk_builder_add_from_file(xml, file.c_str(), &error);

	if(error != NULL)
	{
			g_print("[BMDC][GTKBUILDER][Error] file => %s , => %s\n", file.c_str(), error->message);
			g_error_free(error);
	}
#else
	xml = gtk_builder_new_from_resource( (string("/org/gtk/bmdc/ui/")+ui+".glade.ui").c_str());
	//xml = gtk_builder_new_from_file(file.c_str());
#endif		
  }
  //Do nothink if ui empty
}

Entry::~Entry()
{

}

Entry::EntryType Entry::getType()
{
	return type;
}

const string& Entry::getID()
{
	return id;
}

void Entry::remove()
{
	removeChildren();
	WulforManager::get()->deleteEntry_gui(this);
}

/*
 * Generates a unique ID to allow for duplicate entries
 */
string Entry::generateID()
{
	return dcpp::Util::toString((long)this);
}

GtkWidget *Entry::getWidget(const string &name)
{
	dcassert(xml && !name.empty());
	return GTK_WIDGET(gtk_builder_get_object(xml,name.c_str()));
}

void Entry::addChild(Entry *entry)
{
	children.insert(make_pair(entry->getID(), entry));
	WulforManager::get()->insertEntry_gui(entry);
}

Entry *Entry::getChild(const EntryType childType, const string &childId)
{
	string id = dcpp::Util::toString(childType) + ":" + childId;
	unordered_map<string, Entry *>::const_iterator it = children.find(id);

	if (it == children.end())
		return NULL;
	else
		return it->second;
}

void Entry::removeChild(const EntryType childType, const string &childId)
{
	Entry *entry = getChild(childType, childId);
	removeChild(entry);
}

void Entry::removeChild(Entry *entry)
{
	if (entry != NULL)
	{
		entry->removeChildren();
		children.erase(entry->getID());
		WulforManager::get()->deleteEntry_gui(entry);
	}
}

void Entry::removeChildren()
{
	while (!children.empty())
		removeChild(children.begin()->second);
}
