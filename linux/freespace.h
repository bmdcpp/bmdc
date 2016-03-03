/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#pragma once
#include <string>
#include <errno.h>
#include <cstring>
#include <cstdio>
#include <errno.h>
#include <mntent.h> /* for getmntent(), et al. */
#include <dcpp/Util.h>

struct FreeSpace {
        static bool FreeDiscSpace ( std::string path, unsigned long long * res, unsigned long long * res2);
        
        static std::string process_mounts(const char *filename)
		{
			FILE *fp;
			struct mntent *fs;
			_aviable = 0;
			_total =0;

			fp = setmntent(filename, "r");	/* read only */
			if (fp == NULL) {
					fprintf(stderr, "%s: could not open: %s\n",
					filename, strerror(errno));
				}

			std::string s = "";
			while ((fs = getmntent(fp)) != NULL)
				print_mount(s,fs);

			endmntent(fp);
			return s;
}
/* print_mount --- print a single mount entry */
static void print_mount(std::string &s,const struct mntent *fs)
{
	if(strcmp(fs->mnt_type,MNTTYPE_SWAP) == 0 ||  strcmp(fs->mnt_type,MNTTYPE_IGNORE) == 0)
			return;
	unsigned long long  aviable = 0,total = 0;
			
	if(FreeDiscSpace(fs->mnt_dir,&aviable,&total) == false){
			aviable = total = 0;
	}	
		char buf[1000];
		//@some unneeded FS
		if(strcmp (fs->mnt_fsname,"none") == 0)
			return;
		if(strcmp (fs->mnt_fsname,"dev")	== 0)
			return;
		if(strcmp (fs->mnt_fsname,"run")	== 0)
			return;	
		if(strcmp (fs->mnt_fsname,"tmpfs")	== 0)
			return;	
		if(strcmp (fs->mnt_fsname,"rootfs")	== 0)
			return;		
		if(strcmp (fs->mnt_fsname,"udev")	== 0)
			return;	
		if(strcmp (fs->mnt_fsname,"shm")	== 0)
			return;
		if(strcmp (fs->mnt_fsname,"cgroup_root") == 0)
			return;
		if(strcmp (fs->mnt_type,"overlay") == 0)
			return;
		if(strcmp (fs->mnt_type,"overlayfs") == 0)
			return;	
			
		if(std::string(fs->mnt_fsname).find("fuse") != std::string::npos)	
			return;
			
		if(aviable == 0 || total == 0)
				return;
		
		sprintf(buf,"-= Mount => %s dir =>  %s  => type => %s Free => %s / Total => %s =-\n",
						fs->mnt_fsname,
						fs->mnt_dir,
						fs->mnt_type,
						dcpp::Util::formatBytes(aviable).c_str(),
						dcpp::Util::formatBytes(total).c_str()
					);
		_aviable += aviable;
		_total += total;
		s+=std::string(buf);
}

        static unsigned long long _aviable;
        static unsigned long long _total;
        
};

