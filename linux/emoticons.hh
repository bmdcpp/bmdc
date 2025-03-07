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

#ifndef EMOTICONS_HH
#define EMOTICONS_HH
#include <set>
#include <gtk/gtk.h>

#define EMOTICONS_MAX 255
#define SIZE_NAME 255

class Emot
{
	public:

		typedef std::vector<Emot *> List;
		typedef List::const_iterator Iter;

		Emot(GList *names, std::string file, GdkPixbuf *pixbuf = NULL) :
			names(names), file(file), pixbuf(pixbuf) {}
		~Emot() {}

		GList* getNames() {return names;}
		std::string getFile() {return file;}
		GdkPixbuf* getPixbuf() {return pixbuf;}

	private:
		GList *names;
		std::string file;
		GdkPixbuf *pixbuf;
};

class Emoticons
{
	public:
		static Emoticons* start(const std::string &packName = SETTING(EMOT_PACK));
		static void stop();

		Emoticons(const std::string &packName = SETTING(EMOT_PACK));
		~Emoticons();

		// GUI functions
		Emot::List& getPack_gui() {return pack;}
		int getCountFile_gui() {return countfile;}
		bool useEmoticons_gui() {return useEmotions;}
		std::string getCurrPackName_gui() {return currPackName;}
		void setCurrPackName_gui(const std::string &name) {currPackName = name;}
		void reloadPack_gui() {create();}
	private:

		bool load(const std::string &file);
		void create();
		void clean();
		bool useEmotions;
		int countfile;
		Emot::List pack;
		std::set<std::string> filter;
		std::string currPackName;
};

#else
class Emoticons;
#endif
