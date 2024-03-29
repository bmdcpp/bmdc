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

#ifndef DCPLUSPLUS_DCPP_A_D_L_SEARCH_H
#define DCPLUSPLUS_DCPP_A_D_L_SEARCH_H

#include "Util.h"
#include "GetSet.h"
#include "SettingsManager.h"
#include "StringSearch.h"
#include "Singleton.h"
#include "DirectoryListing.h"
#include "HintedUser.h"
#include <vector>

namespace dcpp {
	using namespace std;

class AdlSearchManager;

/// Class that represent an ADL search
class ADLSearch: public Flags
{
public:
	ADLSearch();

	/// The search string
	string searchString;
	/// Name of the destination directory (empty = 'ADLSearch')
	string destDir;
	//cmd
	string adlsComment;
	string kickString;
	// Maximum & minimum file sizes (in bytes).
	// Negative values means do not check.
	int64_t minFileSize;
	int64_t maxFileSize;

	// dest dir index
	unsigned long ddIndex;
	int adlsRaw;
	int adlsPoints;
	/// Search source type
	enum SourceType {
		TypeFirst = 0,
		OnlyFile = TypeFirst,
		OnlyDirectory,
		FullPath,
		TTHFile,
		TypeLast
	} sourceType;

	enum SizeType {
		SizeBytes	= TypeFirst,
		SizeKibiBytes,
		SizeMebiBytes,
		SizeGibiBytes,
		SizeTibiBytes,
	};

	enum DetectionType {
		D_FORBIDEN = 1,
		D_REGEX,
		D_CASESEN,
		D_FAV,
		D_OVERIDE

	} detectionType;

	SizeType typeFileSize;
	/// Active search
	bool isActive;

	/// Auto Queue Results
	bool isAutoQueue;

	SourceType StringToSourceType(const string& s);
	string SourceTypeToString(SourceType t);

	SizeType StringToSizeType(const string& s);
	string SizeTypeToString(SizeType t);
	int64_t GetSizeBase();

	///@BMDC++
	/* Forbiden */
	bool isForbidden;//
	bool overRidePoints;
	bool fromFavs;//
	bool isCaseSensitive;//
	bool isRegEx;//
private:
	friend class ADLSearchManager;

	/// Prepare search
	void prepare(ParamMap& params);

	/// Search for file match
	bool matchesFile(const string& f, const string& fp, int64_t size, /*BMDC++*/const TTHValue& root);
	/// Search for directory match
	bool matchesDirectory(const string& d);

	bool searchAll(const string& s);
	//BMDC++
	bool SearchAllTTH(const TTHValue& root) {
		return (&root)->toBase32() == searchString;
	}
	//END
};

/// Class that holds all active searches
class ADLSearchManager : public Singleton<ADLSearchManager>
{
public:
	// Destination directory indexing
	struct DestDir {
		DirectoryListing::Directory* dir;
		DirectoryListing::Directory* subdir;
		string name;
		bool fileAdded;
		DestDir() : name(), dir(nullptr), subdir(nullptr), fileAdded(false) { }
	};
	typedef vector<DestDir> DestDirList;

	ADLSearchManager();
	virtual ~ADLSearchManager();

	// Search collection
	vector<ADLSearch> collection;

	// Load/save search collection to XML file
	void load();
	void save();

	// Settings
	GETSET(HintedUser, user, User)
	GETSET(bool, breakOnFirst, BreakOnFirst)

	/// @remarks Used to add ADLSearch directories to an existing DirectoryListing
	void matchListing(DirectoryListing& aDirList);

private:
	// Recurse through the directories and files of a directory.
	void matchRecurse(DestDirList& aDestList, DirectoryListing& filelist, DirectoryListing::Directory* aDir, string& aPath);
	// Search for file match
	void matchesFile(DestDirList& destDirVector, DirectoryListing::File *currentFile, string& fullPath);
	// Search for directory match
	void matchesDirectory(DestDirList& destDirVector, DirectoryListing::Directory* currentDir, string& fullPath);
	// Step up directory
	void stepUpDirectory(DestDirList& destDirVector);

	// Prepare destination directory indexing
	void prepareDestinationDirectories(DestDirList& destDirs, DirectoryListing::Directory* root);
	// Finalize destination directories
	void finalizeDestinationDirectories(DestDirList& destDirs, DirectoryListing::Directory* root);

	static string getConfigFile();
};

} // namespace dcpp

#endif // !defined(DCPLUSPLUS_DCPP_A_D_L_SEARCH_H)
