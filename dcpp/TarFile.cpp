/*
 * Copyright (C) 2011-2012 Mank, Mank1 at seznam dot cz
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */
#ifdef HAVE_LIBTAR
#include "stdinc.h"
#include "format.h"
#include "DCPlusPlus.h"
#include "LogManager.h"
#include <string>
#include <stdio.h>
#include "File.h"
#include "TarFile.h"
#include <libtar.h>
namespace dcpp {
	
void TarFile::CreateTarredFile(const string _path, const StringPairList& files)
{
	TAR *t;
	char *path = const_cast<char*>(_path.c_str());
	int e = tar_open(&t,path,NULL, O_WRONLY | O_CREAT, 0644, TAR_GNU);
	if(e ==-1){
		dcdebug("Error %s\n",strerror(e));
		return;
	}
	for(StringPairList::const_iterator i = files.begin(); i != files.end(); ++i) {
			char *rpath = (char *)i->first.c_str();
			char *tpath = (char *)i->second.c_str();
			int x = tar_append_file(t,rpath,tpath);	
			if(x != 0)
				continue;
	}
	close(tar_fd(t));
}

void TarFile::DecompresTarredFile(const string _file, const string& _prefix)
{
    TAR *t;
    char *path = const_cast<char*>(_file.c_str());
    int e = tar_open(&t,path,NULL, O_RDONLY, 0644, TAR_GNU);
    if(e == -1)
    {	
		dcdebug("Error %s\n",strerror(e));
		return;
	}
    
    char *prefix = const_cast<char*>(_prefix.c_str());
    tar_extract_all(t,prefix);
	close(tar_fd(t));
}

}//Namespace dcpp
#endif
