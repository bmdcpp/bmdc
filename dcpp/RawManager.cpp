/*
 * Copyright (C) 2007-2017 adrian_007, adrian-007 on o2 point pl
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
#include "DCPlusPlus.h"
#include "RawManager.h"
#include "SimpleXML.h"
#include "ClientManager.h"
#include "File.h"

namespace dcpp {

RawManager::RawManager() {
	loadActionRaws();
}

RawManager::~RawManager() {
	saveActionRaws();
	for(auto i = actions.begin(); i != actions.end(); ++i) {
		delete *i;
	}
}

void RawManager::saveActionRaws() {
	try {
		SimpleXML xml;
		xml.addTag("ActionRaws");
		xml.stepIn();

		for(auto i = actions.begin(); i != actions.end(); ++i) {
			xml.addTag("Action");
			xml.addChildAttrib("ID", Util::toString((*i)->getId()));
			xml.addChildAttrib("Name", (*i)->getName());
			xml.addChildAttrib("Enabled", Util::toString((*i)->getEnabled()));
			xml.stepIn();
			for(auto j = (*i)->raw.begin(); j != (*i)->raw.end(); ++j) {
				xml.addTag("Raw");
				xml.addChildAttrib("ID", Util::toString(j->getId()));
				xml.addChildAttrib("Name", j->getName());
				xml.addChildAttrib("Raw", j->getRaw());
				xml.addChildAttrib("Time", Util::toString(j->getTime()));
				xml.addChildAttrib("Enabled", Util::toString(j->getEnabled()));
			}
			xml.stepOut();
		}
		xml.stepOut();

		string fname = Util::getPath(Util::PATH_USER_CONFIG) + "Raws.xml";

		File f(fname + ".tmp", File::WRITE, File::CREATE | File::TRUNCATE);
		f.write(SimpleXML::utf8Header);
		f.write(xml.toXML());
		f.close();
		File::deleteFile(fname);
		File::renameFile(fname + ".tmp", fname);

	} catch(const Exception& e) {
		dcdebug("RawManager::saveActionRaws: %s\n", e.getError().c_str());
	}
}

void RawManager::loadActionRaws(SimpleXML& aXml) {
	aXml.resetCurrentChild();
	while(aXml.findChild("Action")) {
		Action* a = NULL;
		try {
			a = addAction(aXml.getIntChildAttrib("ID"), aXml.getChildAttrib("Name"), aXml.getBoolChildAttrib("Enabled"));
		} catch(const Exception&) {
			continue;
		}

		aXml.stepIn();
		while(aXml.findChild("Raw")) {
			try {
				Raw r;
				r.setEnabled(aXml.getBoolChildAttrib("Enabled"));
				r.setId(aXml.getIntChildAttrib("ID"));
				r.setName(aXml.getChildAttrib("Name"));
				r.setRaw(aXml.getChildAttrib("Raw"));
				r.setTime(aXml.getIntChildAttrib("Time"));
				addRaw(a, r);
			} catch(const Exception&) {
				// ...
			}
		}
		aXml.stepOut();
	}
}

void RawManager::loadActionRaws() {
	try {
		SimpleXML xml;
		xml.fromXML(File(Util::getPath(Util::PATH_USER_CONFIG) + "Raws.xml", File::READ, File::OPEN).read());

		if(xml.findChild("ActionRaws")) {
			xml.stepIn();
			loadActionRaws(xml);
			xml.stepOut();
		}
	} catch(const Exception& e) {
		dcdebug("RawManager::loadActionRaws: %s\n", e.getError().c_str());
	}
}

int RawManager::getValidAction(int actionId) {
	if(actionId <= 0) return 0;
	Lock l(cs);
	for(auto i = actions.begin(); i != actions.end(); ++i) {
		if((*i)->getId() == actionId)
			return (*i)->getId();
	}
	return 0;
}

string RawManager::getNameActionId(int actionId) {
	Lock l(cs);
	for(auto i = actions.begin(); i != actions.end(); ++i) {
		if((*i)->getId() == actionId)
			return Text::toT((*i)->getName());
	}
	return _("Undefined Action");
}

Action* RawManager::addAction(int id, const std::string& name, bool enabled) noexcept {
	if(name.empty()) {
		printf("NO NAME SPECIFIED");
		return nullptr;
	}	

	Lock l(cs);
	for(auto i = actions.begin(); i != actions.end(); ++i) {
		if(Util::stricmp(name, (*i)->getName()) == 0)
			throw Exception("ACTION EXISTS");
	}

	while(id == 0) {
		id = Util::rand(1, 2147483647);
		for(auto i = actions.begin(); i != actions.end(); ++i) {
			if((*i)->getId() == id) {
				id = 0;
				break;
			}
		}
	}
	actions.push_back(new Action(id, name, enabled));
	return actions.back();
}

int RawManager::editAction(Action* a, const std::string& name) noexcept {
	if(name.empty()) return -1;

	{
		Lock l(cs);
		for(auto i = actions.begin(); i != actions.end(); ++i) {
			if(Util::stricmp(name, (*i)->getName()) == 0)
				return -2;
		}
	}
	a->setName(name);
	return 0;
}

bool RawManager::remAction(Action* a) noexcept {
	Lock l(cs);
	auto i = std::find(actions.begin(), actions.end(), a);
	if(i != actions.end()) {
		actions.erase(i);
		delete a;
		a = NULL;
		return true;
	}
	return false;
}

Action* RawManager::findAction(int id) noexcept {
	Lock l(cs);
	for(auto i = actions.begin(); i != actions.end(); ++i) {
		if(id == (*i)->getId())
			return *i;
	}
	return NULL;
}

Action* RawManager::findAction(const std::string& name) noexcept {
	Lock l(cs);
	for(auto i = actions.begin(); i != actions.end(); ++i) {
		if(Util::stricmp(name, (*i)->getName()) == 0)
			return *i;
	}
	return nullptr;
}

int RawManager::addRaw(Action* a, Raw& r) noexcept {
	if(!a) return -1; // nothing to do

	if(r.getName().empty())
		return -1;

	Lock l(cs);
	for(auto j = a->raw.begin(); j != a->raw.end(); ++j) {
		if(Util::stricmp(j->getName(), r.getName()) == 0)
			return -2;
	}

	while(r.getId() == 0) {
		r.setId(Util::rand(1, 2147483647));
		for(auto j = a->raw.begin(); j != a->raw.end(); ++j) {
			if(j->getId() == r.getId()) {
				r.setId(0);
				break;
			}
		}
	}

	a->raw.push_back(r);
	return 0;
}

int RawManager::editRaw(const Action* a, Raw* old, Raw _new) noexcept {
	if(_new.getName().empty())
		return -1;
	if(Util::stricmp(old->getName(), _new.getName()) != 0) {
		Lock l(cs);
		for(auto j = a->raw.begin(); j != a->raw.end(); ++j) {
			if(Util::stricmp(j->getName(), _new.getName()) == 0)
				continue;
		}
	}
	*old = _new;
	return 0;
}

bool RawManager::remRaw(Action* a, Raw* r) noexcept {
	Lock l(cs);
	for(auto i = a->raw.begin(); i != a->raw.end(); ++i) {
		if(&(*i) == r) { a->raw.erase(i); return true; }
	}
	return false;
}

void CalcADLAction::on(SettingsManagerListener::Load, SimpleXML& xml)  {
	if(xml.findChild("ADLSPoints")) {
		xml.stepIn();
		while(xml.findChild("PointsSetting")) {
			int _points = xml.getIntChildAttrib("Points");
			if(_points <= 0)
				continue;

			Lock l(cs);
			IntMap::iterator i = points.find(_points);
			if(i != points.end())
				continue;
			int _action = xml.getIntChildAttrib("Action");
			points.insert(make_pair(_points, _action));
		}
		xml.stepOut();
	}
}

void CalcADLAction::on(SettingsManagerListener::Save, SimpleXML& xml) {
	xml.addTag("ADLSPoints");
	xml.stepIn();
	for(auto i = points.begin(); i != points.end(); ++i) {
		xml.addTag("PointsSetting");
		xml.addChildAttrib("Points", i->first);
		xml.addChildAttrib("Action", i->second);
	}
	xml.stepOut();
}

void CalcADLAction::calcADLAction(int aPoints, int& a, bool& d) {
	Lock l(cs);
	for(auto i = points.begin(); i != points.end(); ++i) {
		if(aPoints >= i->first) {
			a = i->second;
			continue;

		}
	}

	if(a == 0) {
		a = SETTING(ADLSEARCH_DEFAULT_ACTION);
	}

	int min_p = SETTING(MIN_POINTS_TO_DISPLAY_CHEAT);
	if(min_p > 0) {
		d = (aPoints >= min_p) && SETTING(SHOW_ADLSEARCH_DEFAULT_ACTION);
	} else {
		d = SETTING(SHOW_ADLSEARCH_DEFAULT_ACTION);
	}
}

} // namespace dcpp
/**
 * @file
 * $Id: RawManager.cpp 42 2011-10-31 18:27:40Z adrian_007 $
 */
