/*
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 * 
 */

#ifdef _WIN32

#include "diskinfo.hh"
#include <direct.h>
#include <dcpp/Util.h>
#include <dcpp/Text.h>
#include <dcpp/version.h>

using namespace std;
using namespace dcpp;

StringList DiskInfo::findVolumes() {

  BOOL  found;
  TCHAR   buf[MAX_PATH];  
  HANDLE  hVol;
 StringList volumes;

   hVol = FindFirstVolume(buf, MAX_PATH);

   if(hVol != INVALID_HANDLE_VALUE) {
		volumes.push_back(buf);

		found = FindNextVolume(hVol, buf, MAX_PATH);

		//while we find drive volumes.
		while(found) { 
			volumes.push_back(buf);
			found = FindNextVolume(hVol, buf, MAX_PATH); 
		}
   
	found = FindVolumeClose(hVol);
   }

    return volumes;
}

string DiskInfo::diskSpaceInfo(bool onlyTotal /* = false */) {
	tstring ret = Util::emptyStringT;
	int64_t free = 0, totalFree = 0, size = 0, totalSize = 0, netFree = 0, netSize = 0;
	TStringList volumes = findVolumes();

   for(auto i = volumes.begin(); i != volumes.end(); i++) {
	   if(GetDriveType((*i).c_str()) == DRIVE_CDROM || GetDriveType((*i).c_str()) == DRIVE_REMOVABLE)
		   continue;
	   if(GetDiskFreeSpaceEx((*i).c_str(), NULL, (PULARGE_INTEGER)&size, (PULARGE_INTEGER)&free)){
				totalFree += free;
				totalSize += size;
		}
   }

   //check for mounted Network drives
   ULONG drives = _getdrives();
   TCHAR drive[3] = { _T('A'), _T(':'), _T('\0') };
   
	while(drives != 0) {
		if(drives & 1 && ( GetDriveType(drive) != DRIVE_CDROM && GetDriveType(drive) != DRIVE_REMOVABLE && GetDriveType(drive) == DRIVE_REMOTE) ){
			if(GetDiskFreeSpaceEx(drive, NULL, (PULARGE_INTEGER)&size, (PULARGE_INTEGER)&free)){
				netFree += free;
				netSize += size;
			}
		}
		++drive[0];
		drives = (drives >> 1);
	}
   
	if(totalSize != 0) {
		if(!onlyTotal) {
			ret += _T("\r\n\t Local drive space (free/total): ") + Text::toT(Util::formatBytes(totalFree)) + _T("/") + Text::toT(Util::formatBytes(totalSize));
			if(netSize != 0) {
				ret +=  _T("\r\n\t Network drive space (free/total): ") + Text::toT(Util::formatBytes(netFree)) + _T("/") + Text::toT(Util::formatBytes(netSize));
				ret +=  _T("\r\n\t Total drive space (free/total): ") + Text::toT(Util::formatBytes((netFree+totalFree))) + _T("/") + Text::toT(Util::formatBytes(netSize+totalSize));
			}
		} else {
			ret += Text::toT(Util::formatBytes(totalFree)) + _T("/") + Text::toT(Util::formatBytes(totalSize));
		}
	} else {
		return _T("Error in determining HDD space");
	}

	return Text::fromT(ret);
}

string DiskInfo::diskInfoList() {
	tstring result = Util::emptyStringT;		
	TCHAR buf[MAX_PATH];
	int64_t free = 0, size = 0 , totalFree = 0, totalSize = 0;
	int disk_count = 0;
   
	std::vector<tstring> results; //add in vector for sorting, nicer to look at :)
	// lookup drive volumes.
	auto volumes = findVolumes();

	for(auto i = volumes.begin(); i != volumes.end(); i++) {
		if(GetDriveType((*i).c_str()) == DRIVE_CDROM || GetDriveType((*i).c_str()) == DRIVE_REMOVABLE)
			continue;
	    
		if((GetVolumePathNamesForVolumeName((*i).c_str(), buf, 256, NULL) != 0) &&
			(GetDiskFreeSpaceEx((*i).c_str(), NULL, (PULARGE_INTEGER)&size, (PULARGE_INTEGER)&free) !=0)){
			tstring mountpath = buf; 
			if(!mountpath.empty()) {
				totalFree += free;
				totalSize += size;
				results.push_back((_T("Mount path: ") + mountpath + _T("  \t\tDrive space (free/total) ") + Text::toT(Util::formatBytes(free)) + _T("/") +  Text::toT(Util::formatBytes(size))));
			}
		}
	}
      
	// and a check for mounted Network drives, todo fix a better way for network space
	ULONG drives = _getdrives();
	TCHAR drive[3] = { _T('A'), _T(':'), _T('\0') };
   
	while(drives != 0) {
		if(drives & 1 && ( GetDriveType(drive) != DRIVE_CDROM && GetDriveType(drive) != DRIVE_REMOVABLE && GetDriveType(drive) == DRIVE_REMOTE) ){
			if(GetDiskFreeSpaceEx(drive, NULL, (PULARGE_INTEGER)&size, (PULARGE_INTEGER)&free)){
				totalFree += free;
				totalSize += size;
				results.push_back((_T("Network mount path: ") + (tstring)drive + _T("  \tDrive space (free/total) ") + Text::toT(Util::formatBytes(free)) + _T("/") +  Text::toT(Util::formatBytes(size))));
			}
		}

		++drive[0];
		drives = (drives >> 1);
	}

	result += _T("\r\n");
	result += _T("\r\n_[ ") _T(APPNAME) _T(" ") _T(VERSIONSTRING) _T(" Drive Statistics ]_");

	sort(results.begin(), results.end()); //sort it
	for(auto i = results.begin(); i != results.end(); ++i) {
		disk_count++;
		result += _T("\r\n |\t ") + *i; 
	}
	result += _T("\r\n |\t \r\n | \t Total drive space (free/total): ") + Text::toT(Util::formatBytes((totalFree))) + _T("/") + Text::toT(Util::formatBytes(totalSize));
	result += _T("\r\n |\t Total drive count: ") + Text::toT(Util::toString(disk_count));
	result += _T("\r\n");
   
	results.clear();

   return Text::fromT(result);
}
#endif
