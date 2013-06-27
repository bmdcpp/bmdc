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

struct FreeSpace {
        static bool FreeDiscSpace ( std::string path, unsigned long long * res, unsigned long long * res2);
        
        static std::string process_mounts(const char *filename)
		{
			FILE *fp;
			struct mntent *fs;

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

static std::string formatBytes(int64_t aBytes) {
    char buf[128];
    if(aBytes < 1024) {
        snprintf(buf, sizeof(buf), "%d B", (int)(aBytes&0xffffffff));
    } else if(aBytes < 1024*1024) {
        snprintf(buf, sizeof(buf), "%.02f KiB", (double)aBytes/(1024.0));
    } else if(aBytes < 1024*1024*1024) {
        snprintf(buf, sizeof(buf), "%.02f MiB", (double)aBytes/(1024.0*1024.0));
    } else if(aBytes < (int64_t)1024*1024*1024*1024) {
        snprintf(buf, sizeof(buf), "%.02f GiB", (double)aBytes/(1024.0*1024.0*1024.0));
    } else if(aBytes < (int64_t)1024*1024*1024*1024*1024) {
        snprintf(buf, sizeof(buf), "%.02f TiB", (double)aBytes/(1024.0*1024.0*1024.0*1024.0));
    } else {
        snprintf(buf, sizeof(buf), "%.02f PiB", (double)aBytes/(1024.0*1024.0*1024.0*1024.0*1024.0));
    }

    return buf;
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
		if(strcmp (fs->mnt_fsname,"cgroup_root")	== 0)
			return;			
			
		if(std::string(fs->mnt_fsname).find("fuse") != std::string::npos)	
			return;
		if(aviable == 0 || total == 0)
				return;
		
		sprintf(	buf,"-= Mount => %s dir =>  %s  => type => %s Free => %s / Total => %s =-\n",
						fs->mnt_fsname,
						fs->mnt_dir,
						fs->mnt_type,
						formatBytes(aviable).c_str(),
						formatBytes(total).c_str()
					);
					_aviable += aviable;
					_total += total;
					
			
	s+=std::string(buf);
}

        static unsigned long long _aviable;
        static unsigned long long _total;
        
};

