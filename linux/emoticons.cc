/*
 * Copyright © 2009-2012 freedcpp, http://code.google.com/p/freedcpp
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

#include "settingsmanager.hh"
#include "wulformanager.hh"
#include "WulforUtil.hh"
#include <dcpp/Text.h>
#include <dcpp/SimpleXML.h>
#include <dcpp/SettingsManager.h>
#include "emoticons.hh"

using namespace std;
using namespace dcpp;

Emoticons *Emoticons::emoticons = NULL;

void Emoticons::start()
{
	dcassert(!emoticons);
	emoticons = new Emoticons();
}

void Emoticons::stop()
{
	dcassert(emoticons);
	delete emoticons;
	emoticons = NULL;
}

Emoticons* Emoticons::get()
{
	dcassert(emoticons);
	return emoticons;
}

Emoticons::Emoticons()
{
	//currPackName = SETTING(EMOT_PACK);//WGETS("emoticons-pack");
	create();
}

Emoticons::~Emoticons()
{
	clean();
//	SettingsManager::getInstance()->set(SettingsManager::EMOT_PACK,currPackName);
}

void Emoticons::create(string address)
{
	clean();

	if (!WGETB("emoticons-use"))
		return;

	string file = getCurrPackName_gui(address);
	string path = WulforManager::get()->getPath() + G_DIR_SEPARATOR_S + "emoticons" + G_DIR_SEPARATOR_S;
	string packName = file;

	/* load current pack */
	if (load(path + packName + ".xml"))
	{
		useEmotions = TRUE;
		return;
	}

	/* load next packs */
	StringList files = File::findFiles(path, "*.xml");

	for(auto it = files.begin(); it != files.end(); ++it)
	{
		file = Util::getFileName(*it);
		string::size_type pos = file.rfind('.');
		file = file.substr(0, pos);

		if (file != packName)
		{
			if (load(path + file + ".xml"))
			{
				useEmotions = TRUE;
				currPackName = file;
				return;
			}
		}
		dcdebug("%s",file.c_str());
	}

	currPackName = "";
}

void Emoticons::clean()
{
	if (!pack.empty())
	{
		for (Emot::Iter it = pack.begin(); it != pack.end(); ++it)
		{
			GList *list = (*it)->getNames();
			g_list_foreach(list, (GFunc)g_free, NULL);
			g_list_free(list);
			g_object_unref(G_OBJECT((*it)->getPixbuf()));

			delete *it;
		}

		pack.clear();
		filter.clear();
	}

	useEmotions = FALSE;
}

bool Emoticons::load(const string &file)
{
	if (file.empty())
		return FALSE;

	string path = WulforManager::get()->getPath() + G_DIR_SEPARATOR_S + "emoticons" + G_DIR_SEPARATOR_S;
	countfile = 0;

	try
	{
		SimpleXML xml;
		xml.fromXML(File(file, File::READ, File::OPEN).read());

		if (xml.findChild("emoticons-map"))
		{
			string map = xml.getChildAttrib("name");
			path += map + G_DIR_SEPARATOR_S;
			string emotName, emotPath, emotFile;

			xml.stepIn();

			while (xml.findChild("emoticon"))
			{
				GList *list = NULL;
				emotFile = xml.getChildAttrib("file");
				emotPath = Text::fromUtf8(path + emotFile);

				if (!g_file_test(emotPath.c_str(), G_FILE_TEST_EXISTS))
					continue;

				xml.stepIn();

				while (xml.findChild("name"))
				{
					emotName = xml.getChildAttrib("text");

					if (emotName.empty() || g_utf8_strlen(emotName.c_str(), -1) > Emot::SIZE_NAME || filter.count(emotName))
						continue;

					list = g_list_append(list, g_strdup(emotName.c_str()));
					filter.insert(emotName);
				}

				if (list != NULL)
				{
					Emot *emot = new Emot(list, emotFile, gdk_pixbuf_new_from_file(emotPath.c_str(), NULL));
					pack.push_back(emot);
					countfile++;
				}
					xml.stepOut();
			}
				xml.stepOut();
		}
	}
	catch (const Exception &e)
	{
		dcdebug("bmdc: %s...\n %s", e.getError().c_str(), path.c_str());
		return FALSE;
	}

	return !pack.empty();
}

void Emoticons::rebuildHubEmot(string address)
{
	string path = WulforManager::get()->getPath() + G_DIR_SEPARATOR_S + "emoticons" + G_DIR_SEPARATOR_S;
	for(auto i = hubs.begin();i!=hubs.end();++i)
	{
		dcdebug("%s - %s - %s",i->first.c_str(),address.c_str(),string(path + i->second + ".xml").c_str());
		auto f = load2(path + i->second + ".xml");
		if(f.second == 0)
				continue;
		hubs2.insert(make_pair(address,f.first));
		hubs3.insert(make_pair(address,f.second));
	}

}

pair<Emot::List,int> Emoticons::load2(const string &file)
{
	Emot::List packs;	
	std::set<std::string> filt;
	if (file.empty())
		return make_pair(vector<Emot *>(),0);;

	string path = WulforManager::get()->getPath() + G_DIR_SEPARATOR_S + "emoticons" + G_DIR_SEPARATOR_S;
	int count = 0;

	try
	{
		SimpleXML xml;
		xml.fromXML(File(file, File::READ, File::OPEN).read());

		if (xml.findChild("emoticons-map"))
		{
			string map = xml.getChildAttrib("name");
			path += map + G_DIR_SEPARATOR_S;
			string emotName, emotPath, emotFile;

			xml.stepIn();

			while (xml.findChild("emoticon"))
			{
				GList *list = NULL;
				emotFile = xml.getChildAttrib("file");
				emotPath = Text::fromUtf8(path + emotFile);

				if (!g_file_test(emotPath.c_str(), G_FILE_TEST_EXISTS))
					continue;

				xml.stepIn();

				while (xml.findChild("name"))
				{
					emotName = xml.getChildAttrib("text");

					if (emotName.empty() || g_utf8_strlen(emotName.c_str(), -1) > Emot::SIZE_NAME || filt.count(emotName))
						continue;

					list = g_list_append(list, g_strdup(emotName.c_str()));
					filt.insert(emotName);
				}

				if (list != NULL)
				{
					Emot *emot = new Emot(list, emotFile, gdk_pixbuf_new_from_file(emotPath.c_str(), NULL));
					packs.push_back(emot);
					count++;
				}
					xml.stepOut();
			}
				xml.stepOut();
		}
	}
	catch (const Exception &e)
	{
		dcdebug("bmdc: %s...\n %s", e.getError().c_str(), path.c_str());
		return make_pair(vector<Emot *>(),0);
	}
	dcdebug("%s %d",file.c_str(),count);
	return make_pair(packs,count);
}


