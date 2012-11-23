#include "stdinc.h"
#include "DCPlusPlus.h"
#include "HintedUser.h"
#ifndef _FAKE_CHECKER_
#define _FAKE_CHECKER_
namespace dcpp {
class FakeChecker
{
	public:
		FakeChecker(ClientManager *_cm): cm(_cm) { }

	void setSupports(const HintedUser& user, const string& aSupport);
	void setGenerator(const HintedUser& user, const string& aGenerator, const string& cid, const string& aBase);
	void setPkLock(const HintedUser& user, const string& aPk, const string& aLock);
	void setUnknownCommand(const HintedUser& user, const string& cmd);
	void setListSize(const HintedUser& user, int64_t listSize, bool adc);
	void setListLength(const HintedUser& user, const string& listLen);
	void fileListDisconnected(const HintedUser& user);


	void setCheating(const HintedUser& user, const string& _ccResponse, const string& _cheatString, int _actionId, bool _displayCheat,
		bool _badClient, bool _badFileList, bool _clientCheckComplete, bool _fileListCheckComplete);

	friend class ClientManager;//is this needed ??
	ClientManager *cm;
};
}//namespace dcpp
#endif
