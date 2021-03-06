/*
 * Copyright (C) 2001-2017 Jacek Sieka, arnetheduck on gmail point com
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

#ifndef DCPLUSPLUS_DCPP_DIRECTORY_LISTING_H
#define DCPLUSPLUS_DCPP_DIRECTORY_LISTING_H

#include <set>
#include "forward.h"
#include "HintedUser.h"
#include "MerkleTree.h"
#include "GetSet.h"
#include "Util.h"
#include "MediaInfo.h"

namespace dcpp {

using std::set;

class ListLoader;

class DirectoryListing 
{
public:
	class Directory;

	class File  {
	public:
		typedef File* Ptr;
		typedef vector<Ptr> List;
		typedef List::const_iterator Iter;

		File(Directory* aDir, const string& aName, int64_t aSize, const TTHValue& aTTH) noexcept :
			parent(aDir),tthRoot(aTTH), name(aName),kickString(),fullFileName(aName), size(aSize), points(0), 
			adlsRaw(0),ts(0), adls(false),	overRidePoints(false), fromFavs(false)
		{
		}

		File(const File& rhs, bool _adls = false) :
			name(rhs.name), size(rhs.size), parent(rhs.parent), tthRoot(rhs.tthRoot), adls(_adls),
			points(rhs.points), overRidePoints(rhs.overRidePoints), fromFavs(rhs.fromFavs), 
			adlsRaw(rhs.adlsRaw), kickString(rhs.kickString),fullFileName(rhs.fullFileName),ts(rhs.ts)
		{
		}

		void save(OutputStream& stream, string& indent, string& tmp) const;

		MediaInfo m_media;
		GETSET(Directory*, parent, Parent);
		GETSET(TTHValue, tthRoot, TTH);
		GETSET(string, name, Name);
		GETSET(string, adlsComment ,AdlsComment);
		GETSET(string, kickString, KickString)
		GETSET(string, fullFileName ,FullFileName);
		GETSET(int64_t, size, Size);
		GETSET(int64_t , points, Points);
		GETSET(int64_t, adlsRaw, AdlsRaw);
		GETSET(uint32_t, ts, TS);
		GETSET(bool, adls, Adls);
		GETSET(bool, overRidePoints ,OverRidePoints);
		GETSET(bool, fromFavs, FromFavs);
		
	};

	class Directory  {
	public:
		template<typename T> struct Less {
			bool operator()(typename T::Ptr a, typename T::Ptr b) const { return compare(a->getName(), b->getName()) < 0; }
		};
		typedef Directory* Ptr;
		typedef vector<Ptr> List;
		typedef set<File::Ptr, Less<File> > FList;
		typedef List::iterator Iter;
		typedef unordered_set<TTHValue> TTHSet;

		set<Ptr, Less<Directory> > directories;
		set<File::Ptr, Less<File> > files;

		Directory(Directory* aParent, const string& aName, bool _adls, bool aComplete) :
			parent(aParent),name(aName), adlsComment(""), 
			kickString(""), fullFileName("") ,adls(_adls), complete(aComplete),
			points(0), adlsRaw(0), overRidePoints(false),  fromFavs(false)
			{ }

		virtual ~Directory();

		size_t getTotalFileCount(bool adls = false);
		int64_t getTotalSize(bool adls = false);
		void filterList(DirectoryListing& dirList);
		void filterList(TTHSet& l);
		void getHashList(TTHSet& l);
		void save(OutputStream& stream, string& indent, string& tmp) const;
		void setAllComplete(bool complete);

		size_t getFileCount() const { return files.size(); }

		int64_t getSize() const {
			int64_t x = 0;
			for(auto& i: files) {
				x += i->getSize();
			}
			return x;
		}//Flink
		uint16_t getBitrate() const
		{
			uint16_t x = 0;
			for (auto i = files.begin(); i != files.end(); ++i)
			{
				x = std::max((*i)->m_media.m_bitrate, x);
			}
			return x;
		}
		uint32_t getTS() const
		{
			uint32_t x = 0;
			for (auto i = files.begin(); i != files.end(); ++i)
			{
				x = std::max((*i)->getTS(), x);
			}
			return x;
		}
		
		GETSET(Directory*, parent, Parent);
		GETSET(string, name, Name);
		GETSET(string, adlsComment ,AdlsComment);
		GETSET(string, kickString, KickString)
		GETSET(string, fullFileName ,FullFileName);
		GETSET(int64_t , points, Points);
		GETSET(int64_t, adlsRaw, AdlsRaw);
		GETSET(bool, adls, Adls);
		GETSET(bool, complete, Complete);
		GETSET(bool, overRidePoints ,OverRidePoints);				
		GETSET(bool, fromFavs, FromFavs);
	};

	class AdlDirectory : public Directory {
	public:
		AdlDirectory(const string& aFullPath, Directory* aParent, const string& aName) : Directory(aParent, aName, true, true), fullPath(aFullPath) { }

		GETSET(string, fullPath, FullPath);
	};

	DirectoryListing(const HintedUser& aUser);
	~DirectoryListing();

	void loadFile(const string& path);

	string updateXML(const std::string&);
	string loadXML(InputStream& xml, bool updating);

	/** write an XML representation of this file list to the specified file. */
	void save(const string& path) const;
	/** recursively mark directories and sub-directories as complete or incomplete. */
	void setComplete(bool complete);

	void download(const string& aDir, const string& aTarget, bool highPrio);
	void download(Directory* aDir, const string& aTarget, bool highPrio);
	void download(File* aFile, const string& aTarget, bool view, bool highPrio);

	string getPath(const Directory* d) const;
	string getPath(const File* f) const { return getPath(f->getParent()); }

	/** returns the local path of the file when browsing own file list */
	StringList getLocalPaths(const File* f) const;
	/** returns the local paths of the directory when browsing own file list */
	StringList getLocalPaths(const Directory* d) const;

	int64_t getTotalSize(bool adls = false) { return root->getTotalSize(adls); }
	size_t getTotalFileCount(bool adls = false) { return root->getTotalFileCount(adls); }

	const Directory* getRoot() const { return root; }
	Directory* getRoot() { return root; }

	static UserPtr getUserFromFilename(const string& fileName);
	DirectoryListing::Directory::List getForbiddenDirs();
	DirectoryListing::File::List getForbiddenFiles();

	GETSET(HintedUser, user, User);
private:
	friend class ListLoader;

	Directory* root;
	string base;
public:
	GETSET(bool, abort, Abort);
private:	
	Directory* find(const string& aName, Directory* current) const;
};

inline bool operator==(DirectoryListing::Directory::Ptr a, const string& b) { return Util::stricmp(a->getName(), b) == 0; }
inline bool operator==(DirectoryListing::File::Ptr a, const string& b) { return Util::stricmp(a->getName(), b) == 0; }

} // namespace dcpp

#endif // !defined(DIRECTORY_LISTING_H)
