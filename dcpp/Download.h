#ifndef DCPLUSPLUS_DCPP_DOWNLOAD_H_
#define DCPLUSPLUS_DCPP_DOWNLOAD_H_

#include "forward.h"
#include "Transfer.h"
#include "MerkleTree.h"
#include "Flags.h"
#include "Streams.h"

namespace dcpp {

/**
 * Comes as an argument in the DownloadManagerListener functions.
 * Use it to retrieve information about the ongoing transfer.
 */
class Download : public Transfer, public Flags {
public:
	enum {
		FLAG_ZDOWNLOAD = 1 << 1,
		FLAG_TREE_TRIED = 1 << 5,
		FLAG_TTH_CHECK = 1 << 6,
		FLAG_XML_BZ_LIST = 1 << 7,
		FLAG_TESTSUR = 1 << 9,
		FLAG_CHECK_FILE_LIST = 1 << 10,
		FLAG_SLOWUSER = 1 << 11,
		FLAG_OVERLAP = 1 << 12

	};

	Download(UserConnection& conn, QueueItem& qi, const string& path, bool supportsTrees) throw();

	virtual void getParams(const UserConnection& aSource, StringMap& params);

	virtual ~Download();

	/** @return Target filename without path. */
	string getTargetFileName() {
		return Util::getFileName(getPath());
	}

	/** @internal */
	const string& getDownloadTarget() {
		return (getTempTarget().empty() ? getPath() : getTempTarget());
	}

	/** @internal */
	TigerTree& getTigerTree() { return tt; }
	string& getPFS() { return pfs; }
	/** @internal */
	AdcCommand getCommand(bool zlib);

	GETSET(string, tempTarget, TempTarget);
	GETSET(OutputStream*, file, File);
	GETSET(uint64_t, lastTick, LastTick);//used this ??
	GETSET(bool, treeValid, TreeValid);
private:
	Download(const Download&);
	Download& operator=(const Download&);

	TigerTree tt;
	string pfs;
};

} // namespace dcpp

#endif /*DOWNLOAD_H_*/
