/*
 * Copyright (C) 2007-2009 adrian_007, adrian-007 on o2 point pl
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
#ifndef RAW_MANAGER_H
#define RAW_MANAGER_H

#include "CriticalSection.h"
#include "Singleton.h"
#include "ActionRaw.h"
#include "SettingsManager.h"

namespace dcpp {
	typedef std::map<int, int> IntMap;

class SimpleXML;
class RawManager : public Singleton<RawManager>, private SettingsManagerListener {
public:
	// remember to unlock, if locked before. use when changing sth in action/raw content
	void lock() { cs.enter(); }
	void unlock() { cs.leave(); }

	Action::ActionList& getActions() { Lock l(cs); return actions; }
	Action* findAction(int id) throw();
	Action* findAction(const std::string& name) throw();

	Action* addAction(int id, const std::string& name, bool enabled) throw(Exception);
	void editAction(Action* a, const std::string& name) throw(Exception);
	bool remAction(Action* a) throw();

	Raw* addRaw(Action* a, Raw& r) throw(Exception);
	void editRaw(const Action* a, Raw* old, Raw& _new) throw(Exception);
	bool remRaw(Action* a, Raw* r) throw();

	void loadActionRaws();
	void saveActionRaws();

	tstring getNameActionId(int actionId);
	int getValidAction(int actionId);

	// custom points system
	void calcADLAction(int aPoints, int& a, bool& d);

	IntMap& getADLPoints() { Lock l(cs); return points; }
	void updateADLPoints(IntMap& p) {
		Lock l(cs);
		points = p;
	}
private:
	friend class Singleton<RawManager>;

	RawManager();
	~RawManager();

	void loadActionRaws(SimpleXML& aXml);

	void on(SettingsManagerListener::Load, SimpleXML& xml) throw();
	void on(SettingsManagerListener::Save, SimpleXML& xml) throw();

	Action::ActionList actions;

	IntMap points;
	CriticalSection cs;
};

} // namespace dcpp
#endif //RAW_MANAGER_H

/**
 * @file
 * $Id: RawManager.h 42 2007-10-31 18:27:40Z adrian_007 $
 */
