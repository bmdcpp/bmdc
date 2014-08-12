#if !defined __DEBUGMANAGER_H
#define __DEBUGMANAGER_H

#include "Singleton.h"
#include "TimerManager.h"

namespace dcpp {

class DebugManagerListener {
public:
	template<int I>	struct X { enum { TYPE = I };  };
	virtual ~DebugManagerListener() { }
	
	typedef X<0> DebugCommand;
	typedef X<1> DebugDetection;

	virtual void on(DebugCommand, const string&, uint8_t, uint8_t, const string&) noexcept { }
	virtual void on(DebugDetection, const string&) noexcept { }
};

class DebugManager : public Singleton<DebugManager>, public Speaker<DebugManagerListener>
{
	friend class Singleton<DebugManager>;
	DebugManager() { };
public:
	void SendCommandMessage(const string& aMess, uint8_t aType, uint8_t aDirection, const string& aIP) {
		fire(DebugManagerListener::DebugCommand(), aMess, aType, aDirection, aIP);
	}

	void DebugDetection(const string& aMess){ fire(DebugManagerListener::DebugDetection(),aMess);}
	
	~DebugManager() { };

	enum Type {
		TYPE_HUB, TYPE_CLIENT
	};

	enum Direction {
		INCOMING, OUTGOING
	};
};

#define COMMAND_DEBUG(a,b,c,d) DebugManager::getInstance()->SendCommandMessage(a,DebugManager::b,DebugManager::c,d);
#define DETECTION_DEBUG(a) DebugManager::getInstance()->DebugDetection(a);
} // namespace dcpp

#endif
