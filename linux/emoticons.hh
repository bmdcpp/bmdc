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

#ifndef EMOTICONS_HH
#define EMOTICONS_HH
#include <dcpp/stdinc.h>
#include <dcpp/DCPlusPlus.h>

#include <gtk/gtk.h>
#include <map>
#include <set>
#define EMOTICONS_MAX 48

class Emot
{
	public:
		enum {SIZE_NAME = 24};
		typedef std::vector<Emot *> List;
		typedef List::const_iterator Iter;

		Emot(GList *names, std::string file, GdkPixbuf *pixbuf = NULL) :
			names(names), file(file), pixbuf(pixbuf) {}
		~Emot() {}

		GList* getNames() { return names;}
		std::string getFile() const {return file;}
		GdkPixbuf* getPixbuf() const { return pixbuf;}

	private:
		GList *names;
		std::string file;
		GdkPixbuf *pixbuf;
};

class Emoticons
{
	public:
		static void start();
		static void stop();
		static Emoticons* get();

		Emoticons();
		~Emoticons();

		// GUI functions
		Emot::List& getPack_gui(const std::string address = dcpp::Util::emptyString) {
				if(address.empty())
					return pack;
				else if(hubs2.find(address) != hubs2.end())
					return hubs2.find(address)->second;
				else return pack;
		}
		int getCountFile_gui(const std::string address = dcpp::Util::emptyString) const {
				if(address.empty())
					return countfile;
				else if(hubs3.find(address)!=hubs3.end())
					 return hubs3.find(address)->second;
				else return countfile;
		}
		bool useEmoticons_gui() const {return useEmotions;}
		std::string getCurrPackName_gui(const std::string address = dcpp::Util::emptyString) const {
			if(address.empty()) { dcdebug("curr");
				return currPackName; } 
			else if( hubs.find(address) != hubs.end()){
				return hubs.find(address)->second;
			}
			return currPackName;
		}
		void setCurrPackName_gui(const std::string &name, const std::string &address = dcpp::Util::emptyString) {
			if(address.empty())
				currPackName = name;
			hubs.insert(make_pair(address,name));
		}
		void reloadPack_gui(std::string address = dcpp::Util::emptyString) {clean();create(address);}
		
		void rebuildHubEmot(std::string address = dcpp::Util::emptyString);
	private:
		static Emoticons *emoticons;

		bool load(const std::string &file);
		std::pair<Emot::List,int> load2(const std::string &file);
		void create(std::string address = dcpp::Util::emptyString);
		void clean();

		bool useEmotions;
		int countfile;
		Emot::List pack;
		std::set<std::string> filter;
		std::string currPackName;
		std::map<std::string, std::string> hubs;
		std::map<std::string, Emot::List> hubs2;
		std::map<std::string, int> hubs3;
};

#else
class Emoticons;
#endif
