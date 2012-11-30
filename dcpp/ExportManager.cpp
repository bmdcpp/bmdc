/*
 * Copyright © 2010-2012 Mank
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
#ifdef HAVE_LIBTAR

#include "ExportManager.h"
namespace dcpp {
	
void ExportManager::export_(const string &to, StringList &paths)
{
	Lock l(cs);
	map<string, StringList> map;
	map.insert(make_pair(to,paths));
	ofexporteddata.push_back(map);
}	
	
void ExportManager::exportData(const string &to, StringList &paths)	
{
	StringPairList files;
	try {	
	Lock l(cs);
	
	for(auto i = paths.cbegin(); i != paths.cend(); ++i) {
			files.push_back(make_pair(*i, Util::getFileName(*i)));
	}
	
	TarFile tar;
	tar.CreateTarredFile(to,files);
	LogManager::getInstance()->message(_("[ExportManager] Succesfull tarred exported data"));
	}	
	catch(...){ 
	}
}

string ExportManager::importData(const string &from)
{
	Lock l(cs);
	try {	
		TarFile tar;
		tar.DecompresTarredFile(from, Util::getPath(Util::PATH_USER_CONFIG));
	}catch(...){
		LogManager::getInstance()->message(_("[ExportManager] failed to read from .tar!"));
		return _("Failed");
	}
	return _("Import of path") + from + _("is Succesfull");
}

int ExportManager::run()
{
	setThreadPriority(Thread::LOW);
	while(true)
	{
		s.wait(500);
		if(stop)break;
		
		while(!ofexporteddata.empty())
		{
			map<string, StringList> ii = ofexporteddata.front();	
			ofexporteddata.pop_front();
			for(map<string, StringList>::iterator iit = ii.begin(); iit!=ii.end(); ++iit)
				exportData((*iit).first,(*iit).second);
		}
	}
	stop = true;
	return 0;
}

}//dcpp
#endif
