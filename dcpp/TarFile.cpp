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
	
void TarFile::CreateTarredFile(const string& _path, const StringPairList& files)
{
	TAR *t;
	const char *path = _path.c_str();
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

void TarFile::DecompresTarredFile(const string& _file, const string& _prefix)
{
    TAR *t;
    const char *path = _file.c_str();
    int e = tar_open(&t,path,NULL, O_RDONLY, 0644, TAR_GNU);
    if(e == -1)
    {	
		dcdebug("Error %s\n",strerror(e));
		return;
	}
    
    const char *prefix = _prefix.c_str();
    tar_extract_all(t,prefix);
	close(tar_fd(t));
}

}//dcpp;
