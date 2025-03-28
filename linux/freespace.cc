/*
* Copyright © 2011-2025 BMDC
* This file is part of BMDC.
* BMDC is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 2 of the License, or (at your option) any later version.
*
* BMDC is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License along with Xfce-nameday-plugin. If not, see <https://www.gnu.org/licenses/>.
*/
#ifndef APPLE

//function for use fsusage
#ifdef WIN32
    #include <io.h>
    #include <dcpp/stdinc.h>
    #include <dcpp/Text.h>
#else //WIN32
extern "C" {
    #include "fsusage.h"
}
#endif //WIN32
#include "freespace.h"

#ifndef WIN32
unsigned long long FreeSpace::_aviable = 0;
unsigned long long FreeSpace::_total = 0;
#endif
bool FreeSpace::FreeDiscSpace ( std::string path,  unsigned long long * res, unsigned long long * res2) {
        if ( !res ) {
            return false;
        }

#ifdef WIN32
        ULARGE_INTEGER lpFreeBytesAvailableToCaller; // receives the number of bytes on
                                               // disk available to the caller
        ULARGE_INTEGER lpTotalNumberOfBytes;    // receives the number of bytes on disk
        ULARGE_INTEGER lpTotalNumberOfFreeBytes; // receives the free bytes on disk

        if ( GetDiskFreeSpaceExW( (const WCHAR*)dcpp::Text::utf8ToWide(path).c_str(), &lpFreeBytesAvailableToCaller,
                                &lpTotalNumberOfBytes,
                                &lpTotalNumberOfFreeBytes ) == true ) {
                *res = lpTotalNumberOfFreeBytes.QuadPart;
                *res2 = lpTotalNumberOfBytes.QuadPart;
                return true;
        } else {
            return false;
        }
#else //WIN32
        struct fs_usage fsp;
        if ( FS::get_fs_usage(path.c_str(),path.c_str(),&fsp) == 0 ) {
                *res = fsp.fsu_bavail*fsp.fsu_blocksize;
                *res2 =fsp.fsu_blocks*fsp.fsu_blocksize;
                return true;
        } else {
                return false;
        }
#endif //WIN32
}

#endif
