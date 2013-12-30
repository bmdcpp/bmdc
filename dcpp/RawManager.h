/*
 * Copyright (C) 2007-2014 adrian_007, adrian-007 on o2 point pl
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
#ifndef _RAW_MANAGER_H
#define _RAW_MANAGER_H

#include "CriticalSection.h"
#include "Singleton.h"
#include "ActionRaw.h"
#include "SettingsManager.h"
#include "typedefs.h"
#include "CalcADLAction.h"

namespace dcpp {

class SimpleXML;
class RawManager : public Singleton<RawManager> {
public:

	Action::ActionList& getActions() { Lock l(cs); return actions; }
	Action* findAction(int id) noexcept;
	Action* findAction(const std::string& name) noexcept;

	Action* addAction(int id, const std::string& name, bool enabled) throw(Exception);
	void editAction(Action* a, const std::string& name) throw(Exception);
	bool remAction(Action* a) throw();

	void addRaw(Action* a, Raw& r) throw(Exception);
	void editRaw(const Action* a, Raw* old, Raw _new) throw(Exception);
	bool remRaw(Action* a, Raw* r) noexcept;

	void loadActionRaws();
	void saveActionRaws();

	tstring getNameActionId(int actionId);
	int getValidAction(int actionId);

private:
	friend class Singleton<RawManager>;

	RawManager();
	virtual ~RawManager();

	void loadActionRaws(SimpleXML& aXml);

	Action::ActionList actions;

	CriticalSection cs;
};

} // namespace dcpp
#endif //RAW_MANAGER_H

/**
 * @file
 * $Id: RawManager.h 42 2011-10-31 18:27:40Z adrian_007 $
 */
