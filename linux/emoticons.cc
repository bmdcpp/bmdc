/*
* Copyright © 2009-2018 freedcpp, http://code.google.com/p/freedcpp
* Copyright © 2011-2025 BMDC
* This file is part of BMDC.
* BMDC is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 2 of the License, or (at your option) any later version.
*
* BMDC is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License along with Xfce-nameday-plugin. If not, see <https://www.gnu.org/licenses/>.
*/

#include "settingsmanager.hh"
#include "wulformanager.hh"
#include "GuiUtil.hh"
#include "../dcpp/Text.h"
#include "../dcpp/SimpleXML.h"
#include "emoticons.hh"

using namespace std;
using namespace dcpp;

Emoticons* Emoticons::start(const string &packName)
{
	return (new Emoticons(packName));
}

void Emoticons::stop()
{

}

Emoticons::Emoticons(const string &packName):
currPackName(packName)
{
	create();
}

Emoticons::~Emoticons()
{
	clean();
}

void Emoticons::create()
{
	clean();

	if (!SETTING(USE_EMOTS))
		return;

	string path = WulforManager::get()->getPath() + G_DIR_SEPARATOR_S + "emoticons" + G_DIR_SEPARATOR_S;
	string packName = currPackName;

	/* load current pack */
	if (load(path + packName + ".xml"))
	{
		useEmotions = true;
		return;
	}

	/* load next packs */
	StringList files = File::findFiles(path, "*.xml");

	for(auto it = files.begin(); it != files.end(); ++it)
	{
		string file = Util::getFileName(*it);
		string::size_type pos = file.rfind('.');
		file = file.substr(0, pos);

		if (file != packName)
		{
			if (load(path + file + ".xml"))
			{
				useEmotions = true;
				currPackName = file;
				return;
			}
		}
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

	useEmotions = false;
}

bool Emoticons::load(const string &file)
{
	if (file.empty())
		return false;

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

					if (emotName.empty() || g_utf8_strlen(emotName.c_str(), -1) > SIZE_NAME || filter.count(emotName))
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
		dcdebug("[BMDC]: %s...\n", e.what());
		return false;
	}

	return !pack.empty();
}
