/*
* Copyright © 2011-2025 BMDC
* This file is part of BMDC.
* BMDC is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 2 of the License, or (at your option) any later version.
*
* BMDC is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License along with Xfce-nameday-plugin. If not, see <https://www.gnu.org/licenses/>.
*/
#ifdef HAVE_LIBTAR

#include "stdinc.h"
#include "format.h"
#include "DCPlusPlus.h"
#include "LogManager.h"
#include <string>
#include <stdio.h>
#include <libtar.h>
#include "File.h"
#include "TarFile.h"


namespace dcpp {

void TarFile::CreateTarredFile(const string& _path, const StringPairList& files)
{
	TAR *t;
	const char *path = _path.c_str();
	int e = tar_open(&t,(char *)path,NULL, O_WRONLY | O_CREAT, 0644, TAR_GNU);
	if(e == -1){
		dcdebug("Error %d\n",e);
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

void TarFile::DecompresTarredFile(const string& _file, const string& _prefix)
{
    TAR *t;
    const char *path = _file.c_str();
    int e = tar_open(&t,(char *)path,NULL, O_RDONLY, 0644, TAR_GNU);
    if(e == -1)
    {
		//dcdebug("Error %s\n",strerror(e));
		dcdebug("Error %d\n",e);
		return;
	}

    char *prefix = const_cast<char*>(_prefix.c_str());
    tar_extract_all(t,prefix);
	close(tar_fd(t));
}

}//Namespace dcpp
#endif
