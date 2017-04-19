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

#include "stdinc.h"
#include "HashManager.h"

#include "File.h"
#include "FileReader.h"
#include "LogManager.h"
#include "ScopedFunctor.h"
#include "SimpleXML.h"
#include "SFVReader.h"
#include "ZUtils.h"

#ifdef USE_XATTR
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <attr/attributes.h>
#endif

namespace dcpp {

using std::swap;

/* Version history:
- Version 1: DC++ 0.307 to 0.68.
- Version 2: DC++ 0.670 to DC++ 0.802. Improved efficiency.
- Version 3: from DC++ 0.810 on. Changed the file registry to be case-sensitive. */
#define HASH_FILE_VERSION_STRING "3"
static const uint32_t HASH_FILE_VERSION = 3;
const int64_t HashManager::MIN_BLOCK_SIZE = 64 * 1024;

const string HashManager::StreamStore::g_streamName(".gltth");

void HashManager::StreamStore::setCheckSum(TTHStreamHeader& p_header) {
    uint32_t l_sum = 0;

    for (size_t i = 0; i < sizeof(TTHStreamHeader) / sizeof(uint32_t); ++i)
        l_sum ^= ((uint32_t*) & p_header)[i];

    p_header.checksum ^= l_sum;
}

bool HashManager::StreamStore::validateCheckSum(const TTHStreamHeader& p_header) {
    if (p_header.magic != g_MAGIC)
        return false;

    uint32_t l_sum = 0;

    for (size_t i = 0; i < sizeof(TTHStreamHeader) / sizeof(uint32_t); i++)
        l_sum ^= ((uint32_t*) & p_header)[i];

    return (l_sum == 0);
}


#ifdef USE_XATTR
static const uint64_t SIGNIFIC_VALUE    = 10000000;
static const uint64_t NTFS_TIME_OFFSET  = ((uint64_t)(369 * 365 + 89) * 24 * 3600 * SIGNIFIC_VALUE);

static uint64_t getTimeStamp(const string &fname){
    struct stat st;

    /* WARNING: this is not completly portable conversion!
       For more information about portable conversion of linux time to windows filetime see
       ntfs-3g_ntfsprogs/include/ntfs-3g/ntfstime.h from NTFS-3G sources. */
    if (::stat(fname.c_str(), &st) == 0)
        return (uint64_t)st.st_mtime * SIGNIFIC_VALUE + NTFS_TIME_OFFSET + st.st_mtim.tv_nsec/100;

    return 0;
}

#endif // USE_XATTR

bool HashManager::StreamStore::loadTree(const string& p_filePath, TigerTree& tree, int64_t p_aFileSize)
{
#ifdef USE_XATTR
    const int64_t fileSize  = (p_aFileSize == -1) ?  File::getSize(p_filePath) : p_aFileSize;
    const size_t hdrSz      = sizeof(TTHStreamHeader);
    int totalSz    = ATTR_MAX_VALUELEN;
    TTHStreamHeader h;

    uint8_t buf[ATTR_MAX_VALUELEN];

    if (attr_get(p_filePath.c_str(), g_streamName.c_str(), (char*)buf, &totalSz, 0) == 0) {
        memcpy(&h, buf, hdrSz);
        //Note: see http://stackoverflow.com/questions/8132399/how-to-printf-uint64-t
		printf("%s: timestamps header%" PRIu64 ", current%" PRIu64 ", difference(should be zero)%" PRIu64 "\n",
               p_filePath.c_str(), h.timeStamp, getTimeStamp(p_filePath), h.timeStamp - getTimeStamp(p_filePath));
               
        if ( (h.timeStamp == getTimeStamp(p_filePath) && validateCheckSum(h))){ // File was modified and we should reset attr.
            deleteStream(p_filePath);
            return false;
        }

        TigerTree p_Tree = TigerTree(fileSize, /*h.blockSize*/HashManager::MIN_BLOCK_SIZE , buf + hdrSz);

        if (p_Tree.getRoot() == h.root){
            tree = p_Tree;
            return true;
        }
        else {
            return false;
        }    
    }
#endif //USE_XATTR
    return false;
}

bool HashManager::StreamStore::saveTree(const string& p_filePath, const TigerTree& p_Tree)
{
#ifdef USE_XATTR
    TTHStreamHeader h;
	printf("XATTRSET:%s-%s",p_filePath.c_str(),p_Tree.getRoot().toBase32().c_str());
    h.fileSize = File::getSize(p_filePath);
    h.timeStamp = getTimeStamp(p_filePath);
    h.root = p_Tree.getRoot();
    h.blockSize = p_Tree.getBlockSize();
    h.magic = g_MAGIC;

    setCheckSum(h);
    {
        const size_t sz = sizeof(TTHStreamHeader) + p_Tree.getLeaves().size() * TTHValue::BYTES;
        uint8_t *buf = new uint8_t[sz];

        memcpy(buf, &h, sizeof(TTHStreamHeader));
        memcpy(buf + sizeof(TTHStreamHeader), p_Tree.getLeaves()[0].data, p_Tree.getLeaves().size() * TTHValue::BYTES);

        int ret = attr_set(p_filePath.c_str(), g_streamName.c_str(), (char*)buf, sz, 0);
        delete [] buf;
        return ret == 0;//NOTE: Succesfull state
    }
#endif //USE_XATTR
    return false;
}

void HashManager::StreamStore::deleteStream(const string& p_filePath)
{
#ifdef USE_XATTR
    printf("Resetting Xattr for %s\n", p_filePath.c_str());
    attr_remove(p_filePath.c_str(), g_streamName.c_str(), 0);
#endif //USE_XATTR
}

TTHValue* HashManager::getTTH(const string& aFileName, int64_t aSize, uint32_t aTimeStamp) noexcept {
	//string fpath = Util::getFilePath(aFileName);
	Lock l(cs);
	
	TTHValue *tth = store.getTTH(aFileName, aSize, aTimeStamp);
	if(!tth) {
		//TTH is NULL create new variable
		try {
		tth = new TTHValue();
		}catch(...){ }
		//true... is find , false is not find
		if(m_streamstore.loadTree(aFileName,*(TigerTree*)tth,-1)) {
			printf ("%s: hash [%s] was loaded from Xattr.\n", aFileName.c_str(), tth->toBase32().c_str());
			return tth;
		} else	{
			//hash is still NULL. hash file NOW!
			hasher.hashFile(aFileName, aSize);
		} 
	}
	//hash value found
	return tth;
}

bool HashManager::getTree(const TTHValue& root, TigerTree& tt) {
	Lock l(cs);
	return store.getTree(root, tt);
}

int64_t HashManager::getBlockSize(const TTHValue& root) {
	Lock l(cs);
	return store.getBlockSize(root);
}

void HashManager::hashDone(const string& aFileName, uint32_t aTimeStamp, const TigerTree& tth, int64_t speed, int64_t size) {
	try {
		Lock l(cs);
		store.addFile(aFileName, aTimeStamp, tth, true);
		m_streamstore.saveTree(aFileName, tth);
	} catch (const Exception& e) {
		LogManager::getInstance()->message(_("Hashing failed: ")+ e.getError(), LogManager::Sev::HIGH);
		return;
	}

	fire(HashManagerListener::TTHDone(), aFileName, tth.getRoot());

	if(speed > 0) {
		char buf[4024];
		sprintf(buf,_("Finished hashing: %s (%s at %s/s)"),Util::addBrackets(aFileName).c_str(),Util::formatBytes(size).c_str(),Util::formatBytes(speed).c_str());
		LogManager::getInstance()->message(string(buf));
	} else if(size >= 0) {
		char buf[4024];
		sprintf(buf,_("Finished hashing: %s (%s)"), Util::addBrackets(aFileName).c_str(),Util::formatBytes(size).c_str());
		LogManager::getInstance()->message(string(buf));
	} else {
		LogManager::getInstance()->message(_("Finished hashing: ") + Util::addBrackets(aFileName));
	}
}

void HashManager::HashStore::addFile(const string& aFileName, uint32_t aTimeStamp, const TigerTree& tth, bool aUsed) {
	addTree(tth);

	string fname = Util::getFileName(aFileName), fpath = Util::getFilePath(aFileName);

	vector<FileInfo>& fileList = fileIndex[fpath];

	auto j = find(fileList.begin(), fileList.end(), fname);
	if (j != fileList.end()) {
		fileList.erase(j);
	}

	fileList.emplace_back(fname, tth.getRoot(), aTimeStamp, aUsed);
	dirty = true;
}

void HashManager::HashStore::addTree(const TigerTree& tt) noexcept {
	if (treeIndex.find(tt.getRoot()) == treeIndex.end()) {
		try {
			File f(getDataFile(), File::READ | File::WRITE, File::OPEN);
			int64_t index = saveTree(f, tt);
			treeIndex.emplace(tt.getRoot(), TreeInfo(tt.getFileSize(), index, tt.getBlockSize()));
			dirty = true;
		} catch (const FileException& e) {
			LogManager::getInstance()->message(_("Error saving hash data: ") + e.getError(), LogManager::Sev::HIGH);
		}
	}
}

int64_t HashManager::HashStore::saveTree(File& f, const TigerTree& tt) {
	if (tt.getLeaves().size() == 1)
		return SMALL_TREE;

	f.setPos(0);
	int64_t pos = 0;
	size_t n = sizeof(pos);
	if (f.read(&pos, n) != sizeof(pos))
		throw HashException(_("Unable to read hash data file"));

	// Check if we should grow the file, we grow by a meg at a time...
	int64_t datsz = f.getSize();
	if ((pos + (int64_t) (tt.getLeaves().size() * TTHValue::BYTES)) >= datsz) {
		f.setPos(datsz + 1024 * 1024);
		f.setEOF();
	}
	f.setPos(pos);dcassert(tt.getLeaves().size()> 1);
	f.write(tt.getLeaves()[0].data, (tt.getLeaves().size() * TTHValue::BYTES));
	int64_t p2 = f.getPos();
	f.setPos(0);
	f.write(&p2, sizeof(p2));
	return pos;
}

bool HashManager::HashStore::loadTree(File& f, const TreeInfo& ti, const TTHValue& root, TigerTree& tt) {
	if (ti.getIndex() == SMALL_TREE) {
		tt = TigerTree(ti.getSize(), ti.getBlockSize(), root);
		return true;
	}
	try {
		f.setPos(ti.getIndex());
		size_t datalen = TigerTree::calcBlocks(ti.getSize(), ti.getBlockSize()) * TTHValue::BYTES;
		uint8_t* buf = new uint8_t[datalen];
		f.read(&buf[0], datalen);
		tt = TigerTree(ti.getSize(), ti.getBlockSize(), &buf[0]);
		delete [] buf;
		if (!(tt.getRoot() == root))
			return false;
	} catch (const Exception&) {
		return false;
	}

	return true;
}

bool HashManager::HashStore::getTree(const TTHValue& root, TigerTree& tt) {
	auto i = treeIndex.find(root);
	if (i == treeIndex.end())
		return false;
	try {
		File f(getDataFile(), File::READ, File::OPEN);
		return loadTree(f, i->second, root, tt);
	} catch (const Exception&) {
		return false;
	}
}

int64_t HashManager::HashStore::getBlockSize(const TTHValue& root) const {
	auto i = treeIndex.find(root);
	return i == treeIndex.end() ? 0 : i->second.getBlockSize();
}

TTHValue* HashManager::HashStore::getTTH(const string& aFileName, int64_t aSize, uint32_t aTimeStamp) noexcept {
	string fname = Util::getFileName(aFileName), fpath = Util::getFilePath(aFileName);

	auto i = fileIndex.find(fpath);
	if (i != fileIndex.end()) {
		auto j = find(i->second.begin(), i->second.end(), fname);
		if (j != i->second.end()) {
			FileInfo& fi = *j;
			TTHValue* root = const_cast<TTHValue*>(&(fi.getRoot()));
			auto ti = treeIndex.find(*root);
			if(ti != treeIndex.end() && ti->second.getSize() == aSize && fi.getTimeStamp() == aTimeStamp) {
				fi.setUsed(true);
				return root;
			}

			// the file size or the timestamp has changed
			i->second.erase(j);
			dirty = true;
		}
	}
	return nullptr;
}

void HashManager::HashStore::rebuild() {
	try {
		decltype(fileIndex) newFileIndex;
		decltype(treeIndex) newTreeIndex;

		for (auto& i: fileIndex) {
			for (auto& j: i.second) {
				if (!j.getUsed())
					continue;

				auto k = treeIndex.find(j.getRoot());
				if (k != treeIndex.end()) {
					newTreeIndex[j.getRoot()] = k->second;
				}
			}
		}

		string tmpName = getDataFile() + ".tmp";
		string origName = getDataFile();

		createDataFile(tmpName);

		{
			File in(origName, File::READ, File::OPEN);
			File out(tmpName, File::READ | File::WRITE, File::OPEN);

			for (auto i = newTreeIndex.begin(); i != newTreeIndex.end();) {
				TigerTree tree;
				if (loadTree(in, i->second, i->first, tree)) {
					i->second.setIndex(saveTree(out, tree));
					++i;
				} else {
					newTreeIndex.erase(i++);
				}
			}
		}

		for (auto& i: fileIndex) {
			decltype(fileIndex)::mapped_type newFileList;

			for (auto& j: i.second) {
				if (newTreeIndex.find(j.getRoot()) != newTreeIndex.end()) {
					newFileList.push_back(j);
				}
			}

			if(!newFileList.empty()) {
				newFileIndex[i.first] = move(newFileList);
			}
		}

		File::deleteFile(origName);
		File::renameFile(tmpName, origName);
		treeIndex = newTreeIndex;
		fileIndex = newFileIndex;
		dirty = true;
		save();
	} catch (const Exception& e) {
		LogManager::getInstance()->message(_("Hashing failed: ") + e.getError(), LogManager::Sev::HIGH);
	}
}

void HashManager::HashStore::save() {
	if (dirty) {
		try {
			File ff(getIndexFile() + ".tmp", File::WRITE, File::CREATE | File::TRUNCATE);
			BufferedOutputStream<false> f(&ff);

			string tmp;
			string b32tmp;

			f.write(SimpleXML::utf8Header);
			f.write(LIT("<HashStore Version=\"" HASH_FILE_VERSION_STRING "\">\r\n"));

			f.write(LIT("\t<Trees>\r\n"));

			for (auto& i: treeIndex) {
				const TreeInfo& ti = i.second;
				f.write(LIT("\t\t<Hash Type=\"TTH\" Index=\""));
				f.write(Util::toString(ti.getIndex()));
				f.write(LIT("\" BlockSize=\""));
				f.write(Util::toString(ti.getBlockSize()));
				f.write(LIT("\" Size=\""));
				f.write(Util::toString(ti.getSize()));
				f.write(LIT("\" Root=\""));
				b32tmp.clear();
				f.write(i.first.toBase32(b32tmp));
				f.write(LIT("\"/>\r\n"));
			}

			f.write(LIT("\t</Trees>\r\n\t<Files>\r\n"));

			for (auto& i: fileIndex) {
				const string& dir = i.first;
				for (auto& fi: i.second) {
					f.write(LIT("\t\t<File Name=\""));
					f.write(SimpleXML::escape(dir + fi.getFileName(), tmp, true));
					f.write(LIT("\" TimeStamp=\""));
					f.write(Util::toString(fi.getTimeStamp()));
					f.write(LIT("\" Root=\""));
					b32tmp.clear();
					f.write(fi.getRoot().toBase32(b32tmp));
					f.write(LIT("\"/>\r\n"));
				}
			}
			f.write(LIT("\t</Files>\r\n</HashStore>"));
			f.flush();
			ff.close();
			File::deleteFile( getIndexFile());
			File::renameFile(getIndexFile() + ".tmp", getIndexFile());

			dirty = false;
		} catch (const FileException& e) {
			LogManager::getInstance()->message(_("Error saving hash data: ") + e.getError(), LogManager::Sev::HIGH);
		}
	}
}

string HashManager::HashStore::getIndexFile() { return Util::getPath(Util::PATH_USER_CONFIG) + "HashIndex.xml"; }
string HashManager::HashStore::getDataFile() { return Util::getPath(Util::PATH_USER_CONFIG) + "HashData.dat"; }

class HashLoader: public SimpleXMLReader::CallBack {
public:
	HashLoader(HashManager::HashStore& s, const CountedInputStream<false>& countedStream, uint64_t fileSize, function<void (float)> progressF) :
		store(s),
		countedStream(countedStream),
		streamPos(0),
		fileSize(fileSize),
		progressF(progressF),
		version(HASH_FILE_VERSION),
		inTrees(false),
		inFiles(false),
		inHashStore(false)
	{ }
	void startTag(const string& name, StringPairList& attribs, bool simple);
	void endTag(const string&){}
private:
	HashManager::HashStore& store;

	const CountedInputStream<false>& countedStream;
	uint64_t streamPos;
	uint64_t fileSize;
	function<void (float)> progressF;

	int version;
	string file;

	bool inTrees;
	bool inFiles;
	bool inHashStore;
};

void HashManager::HashStore::load(function<void (float)> progressF) {
	try {
		Util::migrate(getIndexFile());

		File f(getIndexFile(), File::READ, File::OPEN);
		CountedInputStream<false> countedStream(&f);
		HashLoader l(*this, countedStream, f.getSize(), progressF);
		SimpleXMLReader(&l).parse(countedStream);
		f.flush();
		f.close();
	} catch (const Exception&) {
		// ...
	}
}

namespace {
/* version 2 files were stored in lower-case; carry the file registration over only if the file can
be found, and if it has no case-insensitive duplicate. */

#ifdef _WIN32

/* we are going to use GetFinalPathNameByHandle to retrieve a properly cased path out of the
lower-case one that the version 2 file registry has provided us with. that API is only available
on Windows >= Vista. */
typedef DWORD (WINAPI *t_GetFinalPathNameByHandle)(HANDLE, LPTSTR, DWORD, DWORD);
t_GetFinalPathNameByHandle initGFPNBH() {
	static bool init = false;
	static t_GetFinalPathNameByHandle GetFinalPathNameByHandle = nullptr;

	if(!init) {
		init = true;

		auto lib = ::LoadLibrary(_T("kernel32.dll"));
		if(lib) {
			GetFinalPathNameByHandle = reinterpret_cast<t_GetFinalPathNameByHandle>(
				::GetProcAddress(lib, "GetFinalPathNameByHandleW"));
		}
	}

	return GetFinalPathNameByHandle;
}

bool upgradeFromV2(string& file) {
	auto GetFinalPathNameByHandle = initGFPNBH();
	if(!GetFinalPathNameByHandle) {
		return false;
	}

	WIN32_FIND_DATA data;
	// FindFirstFile does a case-insensitive search by default
	auto handle = ::FindFirstFile(Text::toT(file).c_str(), &data);
	if(handle == INVALID_HANDLE_VALUE) {
		// file not found
		return false;
	}
	if(::FindNextFile(handle, &data)) {
		// found a dupe
		::FindClose(handle);
		return false;
	}
	::FindClose(handle);

	// don't use dcpp::File as that would be case-sensitive
	handle = ::CreateFile((Text::toT(Util::getFilePath(file)) + data.cFileName).c_str(),
		GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, 0, nullptr);
	if(handle == INVALID_HANDLE_VALUE) {
		return false;
	}

	string buf(file.size() * 2, 0);
	buf.resize(GetFinalPathNameByHandle(handle, &buf[0], buf.size(), VOLUME_NAME_NT));

	::CloseHandle(handle);

	if(buf.empty()) {
		return false;
	}
	// GetFinalPathNameByHandle prepends "\\?\"; remove it.
	if(buf.size() >= 4 && buf.substr(0, 4) == "\\\\?\\") {
		buf.erase(0, 4);
	}

	auto buf8 = Text::fromT(buf);
	if(Text::toLower(buf8) == file) {
		file = move(buf8);
		return true;
	}

	return false;
}

#else

bool upgradeFromV2(string& ) {
	/// @todo implement this on Linux; by default, force re-hashing.
	return false;
}

#endif

}

static const string sHashStore = "HashStore";
static const string sversion = "version"; // Oops, v1 was like this
static const string sVersion = "Version";
static const string sTrees = "Trees";
static const string sFiles = "Files";
static const string sFile = "File";
static const string sName = "Name";
static const string sSize = "Size";
static const string sHash = "Hash";
static const string sType = "Type";
static const string sTTH = "TTH";
static const string sIndex = "Index";
static const string sBlockSize = "BlockSize";
static const string sTimeStamp = "TimeStamp";
static const string sRoot = "Root";

void HashLoader::startTag(const string& name, StringPairList& attribs, bool simple) {
	ScopedFunctor([this] {
		auto readBytes = countedStream.getReadBytes();
		if(readBytes != streamPos) {
			streamPos = readBytes;
		    if(progressF)	
				progressF(static_cast<float>(readBytes) / static_cast<float>(fileSize));
		}
	});

	if (!inHashStore && name == sHashStore) {
		version = Util::toInt(getAttrib(attribs, sVersion, 0));
		if (version == 0) {
			version = Util::toInt(getAttrib(attribs, sversion, 0));
		}
		inHashStore = !simple;
	} else if (inHashStore && (version == 2 || version == 3)) {
		if (inTrees && name == sHash) {
			const string& type = getAttrib(attribs, sType, 0);
			int64_t index = Util::toInt64(getAttrib(attribs, sIndex, 1));
			int64_t blockSize = Util::toInt64(getAttrib(attribs, sBlockSize, 2));
			int64_t size = Util::toInt64(getAttrib(attribs, sSize, 3));
			const string& root = getAttrib(attribs, sRoot, 4);
			if (!root.empty() && type == sTTH && (index >= 8 || index == HashManager::SMALL_TREE) && blockSize >= 1024) {
				store.treeIndex[TTHValue(root)] = HashManager::HashStore::TreeInfo(size, index, blockSize);
			}
		} else if (inFiles && name == sFile) {
			file = getAttrib(attribs, sName, 0);
			uint32_t timeStamp = Util::toUInt32(getAttrib(attribs, sTimeStamp, 1));
			const auto& root = getAttrib(attribs, sRoot, 2);
			if(!file.empty() && timeStamp > 0 && !root.empty() && (version != 2 || upgradeFromV2(file))) {
				string fname = Util::getFileName(file), fpath = Util::getFilePath(file);
				store.fileIndex[fpath].emplace_back(fname, TTHValue(root), timeStamp, false);
			}
		} else if (name == sTrees) {
			inTrees = !simple;
		} else if (name == sFiles) {
			inFiles = !simple;
		}
	}
}

HashManager::HashStore::HashStore() :
	dirty(false) {

	Util::migrate(getDataFile());

	if (File::getSize(getDataFile()) <= static_cast<int64_t> (sizeof(int64_t))) {
		try {
			createDataFile( getDataFile());
		} catch (const FileException&) {
			// ?
		}
	}
}

/**
 * Creates the data files for storing hash values.
 * The data file is very simple in its format. The first 8 bytes
 * are filled with an int64_t (little endian) of the next write position
 * in the file counting from the start (so that file can be grown in chunks).
 * We start with a 1 mb file, and then grow it as needed to avoid fragmentation.
 * To find data inside the file, use the corresponding index file.
 * Since file is never deleted, space will eventually be wasted, so a rebuild
 * should occasionally be done.
 */
void HashManager::HashStore::createDataFile(const string& name) {
	try {
		File dat(name, File::WRITE, File::CREATE | File::TRUNCATE);
		dat.setPos(1024 * 1024);
		dat.setEOF();
		dat.setPos(0);
		int64_t start = sizeof(start);
		dat.write(&start, sizeof(start));
		dat.flush();
		dat.close();
	} catch (const FileException& e) {
		LogManager::getInstance()->message(_("Error creating hash data file: ") + e.getError(), LogManager::Sev::HIGH);
	}
}

void HashManager::Hasher::hashFile(const string& fileName, int64_t size) noexcept {
	Lock l(cs);
	if(w.insert(make_pair(fileName, size)).second) {
		if(paused > 0)
			paused++;
		else
			s.signal();
	}
}

bool HashManager::Hasher::pause() noexcept {
	Lock l(cs);
	return paused++;
}

void HashManager::Hasher::resume() noexcept {
	Lock l(cs);
	while(--paused > 0)
		s.signal();
}

bool HashManager::Hasher::isPaused() const noexcept {
	Lock l(cs);
	return paused > 0;
}

void HashManager::Hasher::stopHashing(const string& baseDir) {
	Lock l(cs);
	for(auto i = w.begin(); i != w.end();) {
		if(strncmp(baseDir.c_str(), i->first.c_str(), baseDir.size()) == 0) {
			w.erase(i++);
		} else {
			++i;
		}
	}
}

void HashManager::Hasher::getStats(string& curFile, uint64_t& bytesLeft, size_t& filesLeft) const {
	Lock l(cs);
	curFile = currentFile;
	filesLeft = w.size();
	if (running)
		filesLeft++;
	bytesLeft = 0;
	for (auto& i: w) {
		bytesLeft += i.second;
	}
	bytesLeft += currentSize;
}

void HashManager::Hasher::instantPause() {
	bool wait = false;
	{
		Lock l(cs);
		if(paused > 0) {
			paused++;
			wait = true;
		}
	}
	if(wait)
		s.wait();
}

int HashManager::Hasher::run() {
	setThreadPriority(Thread::IDLE);
	static StreamStore streamstore;
	string fname;

	for(;;) {
		s.wait();
		if(stop)
			break;
		if(rebuild) {
			HashManager::getInstance()->doRebuild();
			rebuild = false;
			LogManager::getInstance()->message(_("Hash database rebuilt"));
			continue;
		}
		{
			Lock l(cs);
			if(!w.empty()) {
				currentFile = fname = w.begin()->first;
				currentSize = w.begin()->second;
				w.erase(w.begin());
			} else {
				fname.clear();
			}
		}
		running = true;

		if(!fname.empty()) {
			try {
				uint64_t start = GET_TICK();

				File f(fname, File::READ, File::OPEN);
				int64_t size = f.getSize();
				time_t timestamp = f.getLastModified();

				int64_t sizeLeft = size;
				int64_t bs = max(TigerTree::calcBlockSize(size, 10), MIN_BLOCK_SIZE);

				TigerTree tt(bs);

				CRC32Filter crc32;
				SFVReader sfv(fname);
				CRC32Filter* xcrc32 = 0;
				if(sfv.hasCRC())
					xcrc32 = &crc32;

				uint64_t lastRead = GET_TICK();

				FileReader fr(true);

				fr.read(fname, [&](const void* buf, size_t n) -> bool {
					if(SETTING(MAX_HASH_SPEED)> 0) {
						uint64_t now = GET_TICK();
						uint64_t minTime = n * 1000LL / (SETTING(MAX_HASH_SPEED) * 1024LL * 1024LL);
						if(lastRead + minTime > now) {
							Thread::sleep(minTime - (now - lastRead));
						}
						lastRead = lastRead + minTime;
					} else {
						lastRead = GET_TICK();
					}

					tt.update(buf, n);
					if(xcrc32)
						(*xcrc32)(buf, n);

					{
						Lock l(cs);
						currentSize = max(static_cast<uint64_t>(currentSize - n), static_cast<uint64_t>(0));
					}
					sizeLeft -= n;

					instantPause();
					if (sizeLeft == File::getSize(fname)) {
						streamstore.saveTree(fname, tt);
					}
					return !stop;
				});

				f.close();
				tt.finalize();
				uint64_t end = GET_TICK();
				uint64_t speed = 0;
				if(end > start) {
					speed = size * 1000 / (end - start);
				}

				if( (SETTING(SFV_CHECK) == true) && xcrc32 && xcrc32->getValue() != sfv.getCRC()) {
					LogManager::getInstance()->message(Util::addBrackets(fname)+_(" not shared; calculated CRC32 does not match the one found in SFV file."), LogManager::Sev::HIGH);
				} if (streamstore.loadTree(/*Util::getFilePath(fname)+PATH_SEPARATOR_STR+*/fname, tt, -1)) {
					printf ("%s: hash [%s] was loaded from Xattr.\n", fname.c_str(), tt.getRoot().toBase32().c_str());
					HashManager::getInstance()->hashDone(fname, (int64_t)timestamp, tt, speed, size);
				}
				else {
					HashManager::getInstance()->hashDone(fname, (int64_t)timestamp, tt, speed, size);
					
				}
			} catch(const FileException& e) {
				LogManager::getInstance()->message(_("Error hashing : ") + Util::addBrackets(fname) +":"+ e.getError(), LogManager::Sev::HIGH);
			}
		}
		{
			Lock l(cs);
			currentFile.clear();
			currentSize = 0;
		}
		running = false;
	}
	return 0;
}

HashManager::HashPauser::HashPauser() {
	resume = !HashManager::getInstance()->pauseHashing();
}

HashManager::HashPauser::~HashPauser() {
	if(resume)
		HashManager::getInstance()->resumeHashing();
}

bool HashManager::pauseHashing() noexcept {
	Lock l(cs);
	return hasher.pause();
}

void HashManager::resumeHashing() noexcept {
	Lock l(cs);
	hasher.resume();
}

bool HashManager::isHashingPaused() const noexcept {
	Lock l(cs);
	return hasher.isPaused();
}

} // namespace dcpp
