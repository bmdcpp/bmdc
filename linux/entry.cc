/*
* Copyright © 2004-2018 Jens Oknelid, paskharen@gmail.com
* Copyright © 2011-2025 BMDC
* This file is part of BMDC.
* BMDC is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 2 of the License, or (at your option) any later version.
*
* BMDC is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License along with Xfce-nameday-plugin. If not, see <https://www.gnu.org/licenses/>.
*/

#include "entry.hh"
#include "../dcpp/stdinc.h"
#include "../dcpp/DCPlusPlus.h"
#include "../dcpp/Util.h"
#include "wulformanager.hh"

using namespace std;

Entry::Entry(const EntryType type, const string &ui, const string &id):
	xml(NULL),
	type(type),
	id(std::to_string(type) + ":" + id)
{
	if(!ui.empty()) // Load the Builder XML resource, if applicable
	{
		xml = gtk_builder_new_from_resource( (string("/org/bmdc-team/bmdc/ui/") + ui + ".ui").c_str());
	}
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

GtkWidget* Entry::getWidget(const string& name)
{
	GtkWidget* wid = GTK_WIDGET(gtk_builder_get_object(xml,name.c_str()));
	return wid;
}

void Entry::addChild(Entry *entry)
{
	children.insert(make_pair(entry->getID(), entry));
	WulforManager::get()->insertEntry_gui(entry);
}

Entry *Entry::getChild(const EntryType childType, const string &childId)
{
	string id = std::to_string(childType) + ":" + childId;
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

