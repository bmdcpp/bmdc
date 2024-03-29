/*
 * Copyright (C) Jacek Sieka, arnetheduck on gmail point com
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

/*
 * Automatic Directory Listing Search
 * Henrik Engström, henrikengstrom at home se
 */

#include "stdinc.h"
#include "compiler.h"
#include "nullptr.h"
#include "Util.h"
#include "RegEx.h"
#include "Text.h"
#include "ClientManager.h"
#include "File.h"
#include "LogManager.h"
#include "QueueManager.h"
#include "RawManager.h"
#include "SimpleXMLReader.h"
#include "SimpleXML.h"
#include "StringTokenizer.h"
#include "ActionRaw.h"
#include "Text.h"
#include "SettingsManager.h"
#include "ADLSearch.h"
#include "ActionRaw.h"
#include "CalcADLAction.h"

#include <vector>

namespace dcpp {

ADLSearch::ADLSearch() :
searchString(_("<Enter string>")),
isActive(true),
isAutoQueue(false),
sourceType(OnlyFile),
minFileSize(-1),
maxFileSize(-1),
typeFileSize(SizeBytes),
destDir("ADLSearch"),
ddIndex(0),
isForbidden(false),//BMDC++
overRidePoints(false),
adlsRaw(0),
adlsPoints(0),
adlsComment(),
kickString(),
fromFavs(false),
isCaseSensitive(false),
isRegEx(false)
{

}

ADLSearch::SourceType ADLSearch::StringToSourceType(const string& s) {
	if(Util::stricmp(s.c_str(), "Filename") == 0) {
		return OnlyFile;
	} else if(Util::stricmp(s.c_str(), "Directory") == 0) {
		return OnlyDirectory;
	} else if(Util::stricmp(s.c_str(), "Full Path") == 0) {
		return FullPath;
	}//BMDC++
	else if(Util::stricmp(s.c_str(), "TTH") == 0) {
		return TTHFile;
	//END
	}else {
        return OnlyFile;
	}
}

string ADLSearch::SourceTypeToString(SourceType t) {
	switch(t) {
	default:
	case OnlyFile:		return "Filename";
	case OnlyDirectory:	return "Directory";
	case FullPath:		return "Full Path";
	case TTHFile:       return "TTH";//BMDC++
	}
}

ADLSearch::SizeType ADLSearch::StringToSizeType(const string& s) {
	if(Util::stricmp(s.c_str(), "B") == 0) {
		return SizeBytes;
	} else if(Util::stricmp(s.c_str(), "KiB") == 0) {
		return SizeKibiBytes;
	} else if(Util::stricmp(s.c_str(), "MiB") == 0) {
		return SizeMebiBytes;
	} else if(Util::stricmp(s.c_str(), "GiB") == 0) {
		return SizeGibiBytes;
	} else if(Util::stricmp(s.c_str(), "TiB") == 0) {
		return SizeTibiBytes;
	} else {	
		return SizeBytes;
	}
}

string ADLSearch::SizeTypeToString(SizeType t) {
	switch(t) {
	default:
	case SizeBytes:		return "B";
	case SizeKibiBytes:	return "KiB";
	case SizeMebiBytes:	return "MiB";
	case SizeGibiBytes:	return "GiB";
	case SizeTibiBytes:	return "TiB";
	}
}

int64_t ADLSearch::GetSizeBase() {
	switch(typeFileSize) {
	default:
	case SizeBytes:		return (int64_t)1;
	case SizeKibiBytes:	return (int64_t)1024;
	case SizeMebiBytes:	return (int64_t)1024 * (int64_t)1024;
	case SizeGibiBytes:	return (int64_t)1024 * (int64_t)1024 * (int64_t)1024;
	case SizeTibiBytes:	return (int64_t)1024 * (int64_t)1024 * (int64_t)1024 * (int64_t)1024;
	}
}


bool ADLSearch::matchesFile(const string& f, const string& fp, int64_t size, const TTHValue& root) {
	// Check status
	if(!isActive) {
		return false;
	}

	// Check size for files
	if(size >= 0 && (sourceType == OnlyFile || sourceType == FullPath)) {
		if(minFileSize >= 0 && size < minFileSize * GetSizeBase()) {
			// Too small
			return false;
		}
		if(maxFileSize >= 0 && size > maxFileSize * GetSizeBase()) {
			// Too large
			return false;
		}
	}

	// Do search
	switch(sourceType) {
	default:
	case OnlyDirectory:	return false;
	case OnlyFile:		return searchAll(f);
	case FullPath:		return searchAll(fp);
	case TTHFile:       return SearchAllTTH(root);
	}
}

bool ADLSearch::matchesDirectory(const string& d) {
	// Check status
	if(!isActive) {
		return false;
	}
	if(sourceType != OnlyDirectory) {
		return false;
	}

	// Do search
	return searchAll(d);
}

bool ADLSearch::searchAll(const string& s) {
	if(isRegEx) {
		return RegEx::match<string>(s,searchString,isCaseSensitive);
	} else {
		// Match all substrings
		StringSearch ss(searchString);
		return ss.match(s);
	}
}

ADLSearchManager::ADLSearchManager() : breakOnFirst(false), user(UserPtr(), string())  {
	load();
}

ADLSearchManager::~ADLSearchManager() {
	save();
}

void ADLSearchManager::load() {
	// Clear current
	collection.clear();

	// Load file as a string
	try {
		SimpleXML xml;
		Util::migrate(getConfigFile());
		xml.fromXML(File(getConfigFile(), File::READ, File::OPEN).read());

		if(xml.findChild("ADLSearch")) {
			xml.stepIn();

			// Predicted several groups of searches to be differentiated
			// in multiple categories. Not implemented yet.
			if(xml.findChild("SearchGroup")) {
				xml.stepIn();

				// Loop until no more searches found
				while(xml.findChild("Search")) {
					xml.stepIn();

					// Found another search, load it
					ADLSearch search;

					if(xml.findChild("SearchString")) {
						search.searchString = xml.getChildData();
						if(xml.getBoolChildAttrib("RegEx")) {
							search.isRegEx = true;
						}
					}
					if(xml.findChild("SourceType")) {
						search.sourceType = search.StringToSourceType(xml.getChildData());
					}
					if(xml.findChild("DestDirectory")) {
						search.destDir = xml.getChildData();
					}
					if(xml.findChild("IsActive")) {
						search.isActive = (Util::toInt(xml.getChildData()) != 0);
					}
					if(xml.findChild("MaxSize")) {
						search.maxFileSize = Util::toInt64(xml.getChildData());
					}
					if(xml.findChild("MinSize")) {
						search.minFileSize = Util::toInt64(xml.getChildData());
					}
					if(xml.findChild("SizeType")) {
						search.typeFileSize = search.StringToSizeType(xml.getChildData());
					}
					if(xml.findChild("IsAutoQueue")) {
						search.isAutoQueue = (Util::toInt(xml.getChildData()) != 0);
					}
					if(xml.findChild("IsForbidden")) {
						//search.isForbidden = ((xml.getChildData()) != 0);
						if( !(xml.getChildData()).empty())
							search.setFlag(ADLSearch::D_FORBIDEN);
					}
					//BMDC++/RSX++
					if(xml.findChild("OverRidePoints")) {
						//search.overRidePoints = (Util::toInt(xml.getChildData()) != 0);
						if(!xml.getChildData().empty())
							search.setFlag(ADLSearch::D_OVERIDE);
					}
					if(xml.findChild("AdlsRaw")) {
						search.adlsRaw = RawManager::getInstance()->getValidAction(Util::toInt(xml.getChildData()));
					}
					if(xml.findChild("KickString")) {
						search.kickString = xml.getChildData();
					}
					if(xml.findChild("FromFavs")) {
						//search.fromFavs = (Util::toInt(xml.getChildData()) != 0);
						if(!xml.getChildData().empty())
							search.setFlag(ADLSearch::D_FAV);
					}
					if(xml.findChild("AdlsPoints")) {
						search.adlsPoints = Util::toInt(xml.getChildData());
					}
					//END

					// Add search to collection
					if(!search.searchString.empty()) {
						collection.push_back(search);
					}

					// Go to next search
					xml.stepOut();
				}
			}
		}
	}
	catch(const SimpleXMLException&) { }
	catch(const FileException&) { }
}

void ADLSearchManager::save() {
	// Prepare xml string for saving
	try {
		SimpleXML xml;

		xml.addTag("ADLSearch");
		xml.stepIn();

		// Predicted several groups of searches to be differentiated
		// in multiple categories. Not implemented yet.
		xml.addTag("SearchGroup");
		xml.stepIn();

		// Save all	searches
		for(vector<ADLSearch>::iterator i = collection.begin(); i != collection.end(); ++i) {
			ADLSearch& search = *i;
			if(search.searchString.empty()) {
				continue;
			}
			xml.addTag("Search");
			xml.stepIn();

			xml.addTag("SearchString", search.searchString);
			xml.addChildAttrib("RegEx", search.isRegEx);
			xml.addTag("SourceType", search.SourceTypeToString(search.sourceType));
			xml.addTag("DestDirectory", search.destDir);
			xml.addTag("IsActive", search.isActive);
			xml.addTag("MaxSize", search.maxFileSize);
			xml.addTag("MinSize", search.minFileSize);
			xml.addTag("SizeType", search.SizeTypeToString(search.typeFileSize));
			xml.addTag("IsAutoQueue", search.isAutoQueue);
			//BMDC++ //RSX++alike
			string type = "type";
			xml.addTag("IsForbidden", search.isSet(ADLSearch::D_FORBIDEN));

			xml.addTag("OverRidePoints", search.isSet(ADLSearch::D_OVERIDE));

			xml.addTag("AdlsRaw", RawManager::getInstance()->getValidAction(search.adlsRaw));

			xml.addTag("KickString", search.kickString);
			xml.addChildAttrib(type, string("string"));

			xml.addTag("FromFavs", search.isSet(ADLSearch::D_FAV));

			xml.addTag("AdlsPoints", search.adlsPoints);
			xml.addChildAttrib(type, string("int"));
			//END
			xml.stepOut();
		}

		xml.stepOut();

		xml.stepOut();

		// Save string to file
		try {
			File fout(getConfigFile(), File::WRITE, File::CREATE | File::TRUNCATE);
			fout.write(SimpleXML::utf8Header);
			fout.write(xml.toXML());
			fout.close();
		} catch(const FileException&) { }
	} catch(const SimpleXMLException&) { }
}

void ADLSearchManager::matchesFile(DestDirList& destDirVector, DirectoryListing::File *currentFile, string& fullPath) {
	// Add to any substructure being stored
	for(auto id = destDirVector.begin(); id != destDirVector.end(); ++id) {
		if(id->subdir != NULL) {
			DirectoryListing::File *copyFile = new DirectoryListing::File(*currentFile, true);
			dcassert(id->subdir->getAdls());

			id->subdir->files.insert(copyFile);
		}
		id->fileAdded = false;	// Prepare for next stage
	}

	// Prepare to match searches
	if(currentFile->getName().size() < 1) {
		return;
	}

	string filePath = fullPath + "\\" + currentFile->getName();
	// Match searches
	for(auto is = collection.begin(); is != collection.end(); ++is) {
		if(destDirVector[is->ddIndex].fileAdded) {
			continue;
		}
		if ( (is->matchesFile(currentFile->getName(), filePath, currentFile->getSize(),currentFile->getTTH()))
		) {
			DirectoryListing::File *copyFile = new DirectoryListing::File(*currentFile, true);
			destDirVector[is->ddIndex].dir->files.insert(copyFile);
			destDirVector[is->ddIndex].fileAdded = true;
			if(is->isForbidden) {
				copyFile->setPoints(is->adlsPoints);
				copyFile->setAdlsComment(is->adlsComment);

				copyFile->setOverRidePoints(is->isSet(ADLSearch::D_OVERIDE));

				copyFile->setAdlsRaw(is->adlsRaw);
				copyFile->setKickString(is->kickString);

				copyFile->setFromFavs(is->isSet(ADLSearch::D_FAV));
			}

			if(is->isAutoQueue){
				try {
					QueueManager::getInstance()->add(SETTING(DOWNLOAD_DIRECTORY) + currentFile->getName(),
						currentFile->getSize(), currentFile->getTTH(), getUser());
				} catch(const Exception&) { }
			}

			if(breakOnFirst) {
				// Found a match, search no more
				break;
			}
		}
	}
}

void ADLSearchManager::matchesDirectory(DestDirList& destDirVector, DirectoryListing::Directory* currentDir, string& fullPath) {
	// Add to any substructure being stored
	for(auto id = destDirVector.begin(); id != destDirVector.end(); ++id) {
		if(id->subdir != NULL) {
			DirectoryListing::Directory* newDir =
				new DirectoryListing::AdlDirectory(fullPath, id->subdir, currentDir->getName());
			id->subdir->directories.insert(newDir);
			id->subdir = newDir;
		}
	}

	// Prepare to match searches
	if(currentDir->getName().size() < 1) {
		return;
	}

	// Match searches
	for(auto is = collection.begin(); is != collection.end(); ++is) {
		if(destDirVector[is->ddIndex].subdir != NULL) {
			continue;
		}

		if( (is->matchesDirectory(currentDir->getName()) )) {
			destDirVector[is->ddIndex].subdir =
				new DirectoryListing::AdlDirectory(fullPath, destDirVector[is->ddIndex].dir, currentDir->getName());
			destDirVector[is->ddIndex].dir->directories.insert(destDirVector[is->ddIndex].subdir);

			//RSX++
			if(is->isForbidden) {
				destDirVector[is->ddIndex].subdir->setPoints(is->adlsPoints);
				destDirVector[is->ddIndex].subdir->setAdlsComment(is->adlsComment);
				destDirVector[is->ddIndex].subdir->setOverRidePoints(is->isSet(ADLSearch::D_OVERIDE));
				destDirVector[is->ddIndex].subdir->setAdlsRaw(is->adlsRaw);
				destDirVector[is->ddIndex].subdir->setKickString(is->kickString);
				destDirVector[is->ddIndex].subdir->setFromFavs(is->isSet(ADLSearch::D_FAV));
			}
			//END


			if(breakOnFirst) {
				// Found a match, search no more
				break;
			}
		}
	}
}

void ADLSearchManager::stepUpDirectory(DestDirList& destDirVector) {
	for(auto id = destDirVector.begin(); id != destDirVector.end(); ++id) {
		if(id->subdir != NULL) {
			id->subdir = id->subdir->getParent();
			if(id->subdir == id->dir) {
				id->subdir = NULL;
			}
		}
	}
}

void ADLSearchManager::prepareDestinationDirectories(DestDirList& destDirVector, DirectoryListing::Directory* root) {
	// Load default destination directory (index = 0)
	destDirVector.clear();
	vector<DestDir>::iterator id = destDirVector.insert(destDirVector.end(), DestDir());
	id->name = "ADLSearch";
	id->dir  = new DirectoryListing::Directory(root, "<<<" + id->name + ">>>", true, true);

	// Scan all loaded searches
	for(auto is = collection.begin(); is != collection.end(); ++is) {
		// Check empty destination directory
		if(is->destDir.size() == 0) {
			// Set to default
			is->ddIndex = 0;
			continue;
		}

		// Check if exists
		bool isNew = true;
		long ddIndex = 0;
		for(id = destDirVector.begin(); id != destDirVector.end(); ++id, ++ddIndex) {
			if(Util::stricmp(is->destDir.c_str(), id->name.c_str()) == 0) {
				// Already exists, reuse index
				is->ddIndex = ddIndex;
				isNew = false;
				break;
			}
		}

		if(isNew) {
			// Add new destination directory
			id = destDirVector.insert(destDirVector.end(), DestDir());
			id->name = is->destDir;
			id->dir  = new DirectoryListing::Directory(root, "<<<" + id->name + ">>>", true, true);
			is->ddIndex = ddIndex;
		}
	}
}

void ADLSearchManager::finalizeDestinationDirectories(DestDirList& destDirVector, DirectoryListing::Directory* root) {
	string szDiscard("<<<" + string(_("Discard")) + ">>>");

	// Add non-empty destination directories to the top level
	for(auto id = destDirVector.begin(); id != destDirVector.end(); ++id) {
		if(id->dir->files.size() == 0 && id->dir->directories.size() == 0) {
			delete (id->dir);
		} else if(Util::stricmp(id->dir->getName(), szDiscard) == 0) {
			delete (id->dir);
		} else {
			root->directories.insert(id->dir);
		}
	}
}

void ADLSearchManager::matchListing(DirectoryListing& aDirList) {

	setUser(aDirList.getUser());
	auto root = aDirList.getRoot();

	DestDirList destDirs;
	prepareDestinationDirectories(destDirs, aDirList.getRoot());
	setBreakOnFirst(SETTING(ADLS_BREAK_ON_FIRST));

	string path(root->getName());
	matchRecurse(destDirs, aDirList, root, path);

	finalizeDestinationDirectories(destDirs, root);
}

void ADLSearchManager::matchRecurse(DestDirList& aDestList, DirectoryListing& filelist, DirectoryListing::Directory* aDir, string& aPath) {
	for(auto dirIt = aDir->directories.begin(); dirIt != aDir->directories.end(); ++dirIt) {
		if(filelist.getAbort()) { throw Exception(); }
		string tmpPath = aPath + "\\" + (*dirIt)->getName();
		matchesDirectory(aDestList, (*dirIt), tmpPath);
		matchRecurse(aDestList, filelist,(*dirIt), tmpPath);
	}

	for(auto fileIt= aDir->files.begin();fileIt !=aDir->files.end();++fileIt) {
		if(filelist.getAbort()) { throw Exception(); }
		matchesFile(aDestList,(*fileIt), aPath);
	}

	stepUpDirectory(aDestList);
}

string ADLSearchManager::getConfigFile() {
	 return Util::getPath(Util::PATH_USER_CONFIG) + "ADLSearch.xml";
}

} // namespace dcpp
