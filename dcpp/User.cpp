/*
 * Copyright (C) 2001-2011 Jacek Sieka, arnetheduck on gmail point com
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
#include "RegexUtil.h"//RSX++
#include "User.h"

#include "AdcHub.h"
#include "FavoriteUser.h"
#include "StringTokenizer.h"
#include "ClientManager.h"
#include "FavoriteManager.h"
#include "DetectionManager.h"//RSX++
#include "DebugManager.h"

namespace dcpp {

FastCriticalSection Identity::cs;

OnlineUser::OnlineUser(const UserPtr& ptr, Client& client_, uint32_t sid_) : identity(ptr, sid_), client(client_) {

}

bool Identity::isTcpActive(const Client* c) const {
    if(c != NULL && user == ClientManager::getInstance()->getMe()) {
		return c->isActive(); // userlist should display our real mode
	} else {
	return (!user->isSet(User::NMDC)) ?
		!getIp().empty() && supports(AdcHub::TCP4_FEATURE) :
		!user->isSet(User::PASSIVE);
	}
}

bool Identity::isUdpActive() const {
	if(getIp().empty() || getUdpPort().empty())
		return false;
	return (!user->isSet(User::NMDC)) ? supports(AdcHub::UDP4_FEATURE) : !user->isSet(User::PASSIVE);
}

void Identity::getParams(StringMap& sm, const string& prefix, bool compatibility) const {
	{
		FastLock l(cs);
		for(InfMap::const_iterator i = info.begin(); i != info.end(); ++i) {
			sm[prefix + string((char*)(&i->first), 2)] = i->second;
		}
	}
	if(user) {
		sm[prefix + "SID"] = getSIDString();
		sm[prefix + "CID"] = user->getCID().toBase32();
		sm[prefix + "TAG"] = getTag();
		sm[prefix + "SSshort"] = Util::formatBytes(get("SS"));

		sm[prefix + "Cheat"] = get("CS");
		sm[prefix + "RSshort"] = Util::formatBytes(get("RS"));
		sm[prefix + "LSshort"] = Util::formatBytes(get("LS"));

		if(compatibility) {
			if(prefix == "my") {
				sm["mynick"] = getNick();
				sm["mycid"] = user->getCID().toBase32();
			} else {
				sm["nick"] = getNick();
				sm["cid"] = user->getCID().toBase32();
				sm["ip"] = get("I4");
				sm["tag"] = getTag();
				sm["description"] = get("DE");
				sm["email"] = get("EM");
				sm["share"] = get("SS");
				sm["shareshort"] = Util::formatBytes(get("SS"));

				sm["ClientType"] = get("CL");
				sm["Cheat"] = get("CS");
				sm["listsize"] = Util::formatBytes(get("LS"));
				sm["host"] =	get("HT");
				//some simple names instead of Ax ;)
				sm["adlFile"] =					get("A1");
				sm["adlComment"] =				get("A2");
				sm["adlFileSize"] =				get("A3");
				sm["adlTTH"] =					get("A4");
				sm["adlForbiddenSize"] =		get("A5");
				sm["adlTotalPoints"] =			get("A6");
				sm["adlFilesCount"] =			get("A7");
				sm["adlFileSizeShort"] =		Util::formatBytes(get("A3"));
				sm["adlForbiddenSizeShort"] =	Util::formatBytes(get("A5"));
				//END
			}
		}
	}
}

bool Identity::isClientType(ClientType ct) const {
	int type = Util::toInt(get("CT"));
	return (type & ct) == ct;
}

string Identity::getTag() const {
	if(!get("TA").empty())
		return get("TA");
	if(get("VE").empty() || get("HN").empty() || get("HR").empty() ||get("HO").empty() || get("SL").empty())
		return Util::emptyString;
	return "<" + get("VE") + ",M:" + string(isTcpActive() ? "A" : "P") + ",H:" + get("HN") + "/" +
		get("HR") + "/" + get("HO") + ",S:" + get("SL") + ">";
}

string Identity::get(const char* name) const {
	FastLock l(cs);
	InfMap::const_iterator i = info.find(*(short*)name);
	return i == info.end() ? Util::emptyString : i->second;
}

bool Identity::isSet(const char* name) const {
	FastLock l(cs);
	InfMap::const_iterator i = info.find(*(short*)name);
	return i != info.end();
}

void Identity::set(const char* name, const string& val) {
	FastLock l(cs);
	if(val.empty())
		info.erase(*(short*)name);
	else
		info[*(short*)name] = val;
}

bool Identity::supports(const string& name) const {
	string su = get("SU");
	StringTokenizer<string> st(su, ',');
	for(StringIter i = st.getTokens().begin(); i != st.getTokens().end(); ++i) {
		if(*i == name)
			return true;
	}
	return false;
}

void FavoriteUser::update(const OnlineUser& info) {
	setNick(info.getIdentity().getNick());
	setUrl(info.getClient().getHubUrl());
}
//RSX
string Identity::setCheat(const Client& c, string aCheatDescription, bool aBadClient, bool aBadFilelist /*=false*/, bool aDisplayCheat /*=true*/) {
	//if(!c.isOp() || isOp()) {
	//	return Util::emptyString;
	//}

	StringMap ucParams;
	getParams(ucParams, "user", true);
	string cheat = Util::formatParams(aCheatDescription, ucParams, false);

	string newCheat = Util::emptyString;

	string currentCS = get("CS");
	StringTokenizer<string> st(currentCS, ';');
	for(StringIter i = st.getTokens().begin(); i != st.getTokens().end(); ++i) {
		if((*i).find(cheat) == string::npos) {
			newCheat += (*i) + ";";
		}
	}

	newCheat += cheat + ";";

	if(!cheat.empty())
		set("CS", newCheat);
	if(aBadClient)
		set("BC", "1");
	if(aBadFilelist)
 		set("BF", "1");

	if(BOOLSETTING(DISPLAY_CHEATS_IN_MAIN_CHAT) && aDisplayCheat) {
		string report = "*** *** " +  getNick() + " - " + newCheat;
		return report;
	}
	return Util::emptyString;
}

string Identity::getVersion() const {
	if(user->isSet(User::NMDC))
		return get("VE");
	string version = get("VE");
	string::size_type i = version.find(' ');
	if(i != string::npos)
		return version.substr(i+1);
	return version;
}

string Identity::getFilelistGeneratorVer() const {
	if(!get("FG").empty()) {
		string genVer = get("FG");
		string::size_type i = genVer.find(' ');
		if(i != string::npos ) {
			genVer = genVer.substr(i+1);
		}
		return genVer;
	} else {
		return Util::emptyString;
	}
}

string Identity::getPkVersion() const {
	string pk = get("PK");
	if(pk.find("DCPLUSPLUS") != string::npos && pk.find("ABCABC") != string::npos) {
		return pk.substr(10, pk.length() - 16);
	}
	return Util::emptyString;
}

//RSX++ //Filelist Detector
string Identity::checkFilelistGenerator(OnlineUser& ou) {
	if((get("FG") == "DC++ 0.403")) {
		if((dcpp::RegexUtil::match(getTag(), "^<StrgDC\\+\\+ V:1.00 RC([89]){1}")))  {
			string report = ou.setCheat("rmDC++ in StrongDC++ %[userVE] emulation mode" , true, false, true);
			setClientType("rmDC++");
			logDetect(true);
			ou.getClient().updated(&ou);
			ClientManager::getInstance()->sendAction(ou,SETTING(RMDC_RAW));
			return report;
		}
	}

	if(!get("VE").empty() && strncmp(getTag().c_str(), "<++ V:", 6) == 0) {
		if((Util::toFloat(get("VE")) > 0.668)) {
			if(get("FI").empty() || get("FB").empty()) {
				string report = ou.setCheat("DC++ emulation", true, false, true);
				logDetect(true);
				ou.getClient().updated(&ou);
				ClientManager::getInstance()->sendAction(ou,SETTING(DCPP_EMULATION_RAW));
				return report;
			}
		} else {
			if(!get("FI").empty() || !get("FB").empty()) {
				string report = ou.setCheat("DC++ emulation", true, false, true);
				logDetect(true);
				ou.getClient().updated(&ou);
				ClientManager::getInstance()->sendAction(ou,SETTING(DCPP_EMULATION_RAW));
				return report;
			}
		}
	}


	if((dcpp::RegexUtil::match(get("FG"), "^DC\\+\\+.*"))) {
		if(!get("VE").empty() && (get("VE") != getFilelistGeneratorVer())) {
			string report = ou.setCheat("Filelist Version mis-match", false, true, BOOLSETTING(SHOW_FILELIST_VERSION_MISMATCH));
			logDetect(true);
			ou.getClient().updated(&ou);
			ClientManager::getInstance()->sendAction(ou, SETTING(FILELIST_VERSION_MISMATCH));
			return report;
		}
	}

	return Util::emptyString;
}
//sumary detection
void Identity::logDetect(bool successful) {
	SettingsManager *sm=SettingsManager::getInstance();
	if(sm!=NULL)
	{
		if(successful) {
			int a = SETTING(DETECTT);
			sm->set(SettingsManager::DETECTT ,a + 1);
		} else {
			int b = SETTING(DETECTF);
			sm->set(SettingsManager::DETECTF ,b + 1);
		}
	}
}

void Identity::checkTagState(OnlineUser& ou) {
	string usrTag = getTag();
	if(usrTag.empty()) return;
	bool isActive = ou.getIdentity().isTcpActive();

	if(isActive && (usrTag.find(",M:P,") != string::npos)) {
		ou.getClient().cheatMessage("*** " + getNick() + " - Tag states passive mode, but user is using active mode");
	} else if(!isActive && (usrTag.find(",M:A,") != string::npos)) {
		ou.getClient().cheatMessage("*** " + getNick() + " - Tag states active mode, but user is using passive mode");
	}
}

void Identity::getDetectionParams(StringMap& p) {
	getParams(p, Util::emptyString, true/*false*/);
	p["PKVE"] = getPkVersion();
	//p["VEformat"] = getVersion();

	if(!user->isSet(User::NMDC)) {
		string version = get("VE");
		string::size_type i = version.find(" ");
		if(i != string::npos)
			p["VEformat"] = version.substr(i+1);
		else
			p["VEformat"] = version;
	} else {
		p["VEformat"] = get("VE");
	}

	// convert all special chars to make regex happy
	for(StringMap::iterator i = p.begin(); i != p.end(); ++i) {
		// looks really bad... but do the job
		Util::replace( "\\", "\\\\",i->second); // this one must be first
		Util::replace( "[", "\\[",i->second); //little edit
		Util::replace( "]", "\\]",i->second);
		Util::replace( "^", "\\^",i->second);
		Util::replace( "$", "\\$",i->second);
		Util::replace( ".", "\\.",i->second);
		Util::replace( "|", "\\|",i->second);
		Util::replace( "?", "\\?",i->second);
		Util::replace( "*", "\\*",i->second);
		Util::replace( "+", "\\+",i->second);
		Util::replace( "(", "\\(",i->second);
		Util::replace( ")", "\\)",i->second);
		Util::replace( "{", "\\{",i->second);
		Util::replace( "}", "\\}",i->second);
	}
}

string Identity::getDetectionField(const string& aName) const {
	if(aName.length() == 2) {
		if(aName == "TA")
			return getTag();
		else if(aName == "CO")
			return getConnection();
		else
			return get(aName.c_str());
	} else {
		if(aName == "PKVE") {
			return getPkVersion();
		}
		return Util::emptyString;
	}
}

string Identity::myInfoDetect(OnlineUser& ou) {
	checkTagState(ou);

	StringMap params;
	getDetectionParams(params); // get identity fields and escape them, then get the rest and leave as-is
	const DetectionManager::DetectionItems& profiles = DetectionManager::getInstance()->getProfiles(params, /*false*/true);

	for(DetectionManager::DetectionItems::const_iterator i = profiles.begin(); i != profiles.end(); ++i) {
		const DetectionEntry& entry = *i;
		if(!entry.isEnabled)
			continue;
		DetectionEntry::INFMap INFList;
		if(!entry.defaultMap.empty()) {
			// fields to check for both, adc and nmdc
			INFList = entry.defaultMap;
		}

		if(getUser()->isSet(User::NMDC) && !entry.nmdcMap.empty()) {
			INFList.insert(INFList.end(), entry.nmdcMap.begin(), entry.nmdcMap.end());
		} else if(!entry.adcMap.empty()) {
			INFList.insert(INFList.end(), entry.adcMap.begin(), entry.adcMap.end());
		}

		if(INFList.empty())
			continue;

		bool _continue = false;
		DETECTION_DEBUG("\tChecking User Info Profile: " + entry.name);


		for(DetectionEntry::INFMap::const_iterator j = INFList.begin(); j != INFList.end(); ++j) {
			string aPattern = RegexUtil::formatRegExp(j->second, params);
			string aField = getDetectionField(j->first);
			DETECTION_DEBUG("\t\tPattern: " + aPattern + " Field: " + aField);
			if(!(RegexUtil::match(aField, aPattern))) {
				_continue = true;
				break;
			}
		}
		if(_continue)
			continue;

		DETECTION_DEBUG("**** Client found: " + entry.name + " ****\r\n");

		setMyInfoType(entry.name);
		set("CM", entry.comment);

		string report = Util::emptyString;
		if(!entry.cheat.empty()) {
			report = ou.setCheat(entry.cheat, true, false, ou.getClient().isActionActive(entry.rawToSend));
		}

		ClientManager::getInstance()->sendAction(ou, entry.rawToSend);
		return report;
	}
	return Util::emptyString;
}

string Identity::updateClientType(OnlineUser& ou) {
	if(getUser()->isSet(User::DCPLUSPLUS)) {
		const float versionf = Util::toFloat(getVersion());
		if((get("LL") == "11") && (getBytesShared() > 0)) {
			setClientType("DC++ Stealth");
			const string& report = ou.setCheat("Fake FileList - ListLen = 11B", true, true, BOOLSETTING(SHOW_LISTLEN_MISMATCH));
			ClientManager::getInstance()->sendAction(ou, SETTING(LISTLEN_MISMATCH));
			logDetect(true);
			return report;
		} else if(strncmp(getTag().c_str(), "<++ V:", 6) == 0 && versionf < 1.001f && versionf >= 0.69f) {
			//suppose to be dc++  >= 0.69
			if(get("LL") != "42" && !get("LL").empty()) {
				setClientType("Faked DC++");
				set("CM", "Supports corrupted files...");
				const string& report = ou.setCheat("ListLen mis-match (V:0.69+)", true, false, BOOLSETTING(SHOW_LISTLEN_MISMATCH));
				ClientManager::getInstance()->sendAction(ou, SETTING(LISTLEN_MISMATCH));
				logDetect(true);
				return report;
			} else if(versionf > (float)0.699 && !get("TS").empty() && get("TS") != "GetListLength not supported") {
				const string& report = ou.setCheat("DC++ emulation", true, false, BOOLSETTING(SHOW_DCPP_EMULATION_RAW));
				ClientManager::getInstance()->sendAction(ou,SETTING(DCPP_EMULATION_RAW));
				logDetect(true);
				return report;
			}
		}
	}

	uint64_t tick = GET_TICK();

	StringMap params;
	getDetectionParams(params); // get identity fields and escape them, then get the rest and leave as-is
	const DetectionManager::DetectionItems& profiles = DetectionManager::getInstance()->getProfiles(params);//thinking//true

	for(DetectionManager::DetectionItems::const_iterator i = profiles.begin(); i != profiles.end(); ++i) {
		const DetectionEntry& entry = *i;
		if(!entry.isEnabled)
			continue;
		DetectionEntry::INFMap INFList;
		if(!entry.defaultMap.empty()) {
			// fields to check for both, adc and nmdc
			INFList = entry.defaultMap;
		}

		if(getUser()->isSet(User::NMDC) && !entry.nmdcMap.empty()) {
			INFList.insert(INFList.end(), entry.nmdcMap.begin(), entry.nmdcMap.end());
		} else if(!entry.adcMap.empty()) {
			INFList.insert(INFList.end(), entry.adcMap.begin(), entry.adcMap.end());
		}

		if(INFList.empty())
			continue;

		bool _continue = false;

		DETECTION_DEBUG("\tChecking profile: " + entry.name);

		for(DetectionEntry::INFMap::const_iterator j = INFList.begin(); j != INFList.end(); ++j) {
			string aPattern = RegexUtil::formatRegExp(j->second, params);
			string aField = getDetectionField(j->first);
			DETECTION_DEBUG("\t\tPattern: " + aPattern + " Field: " + aField);
			if(!RegexUtil::match(aField, aPattern)) {
				_continue = true;
				break;
			}
		}
		if(_continue)
			continue;

		DETECTION_DEBUG("**** Client found: " + entry.name + " time taken: " + Util::toString(GET_TICK()-tick) + " ms ****\r\n");

		setClientType(entry.name);
		set("CM", entry.comment);
		set("CS", entry.cheat); 
		set("BC", entry.cheat.empty() ? Util::emptyString : "1");
		logDetect(true);

		if(entry.checkMismatch && getUser()->isSet(User::NMDC) &&  (params["VE"] != params["PKVE"])) {
			setClientType(entry.name + " Version mis-match");
			return ou.setCheat(entry.cheat + " Version mis-match", true, false, ou.getClient().isActionActive(SETTING(VERSION_MISMATCH)));
		}

		string report = Util::emptyString;
		if(!entry.cheat.empty()) {
			report = ou.setCheat(entry.cheat, true, false, ou.getClient().isActionActive(entry.rawToSend));
		}

		ClientManager::getInstance()->sendAction(ou, entry.rawToSend);
		return report;
	}

	logDetect(false);
	setClientType("Unknown");
	return Util::emptyString;
}

//RSX++ //checking stuff
bool OnlineUser::getChecked(bool filelist/* = false*/, bool checkComplete/* = true*/) {
	if(!identity.isTcpActive() && !getClient().isActive()) {
		identity.setClientType("[Passive]");
		setTestSURComplete();
		setFileListComplete();
		return true;
	} else if(getUser()->isSet(User::OLD_CLIENT)) {
		setTestSURComplete();
		setFileListComplete();
		return true;
	} else if(isProtectedUser()) {
		if((BOOLSETTING(UNCHECK_CLIENT_PROTECTED_USER) && !filelist) || (BOOLSETTING(UNCHECK_LIST_PROTECTED_USER) && filelist)) {
			identity.setClientType("[Protected]");
			setTestSURComplete();
			setFileListComplete();
			return true;
		}
	}
	if(checkComplete) //prevent double checking (shouldCheckClient/Filelist)
		return filelist ? identity.isFileListChecked() : identity.isClientChecked();
	return false;
}
//END
string Identity::checkrmDC(OnlineUser& ou) {
	string report = Util::emptyString;
	if((RegexUtil::match(getVersion(), "^0.40([0123]){1}$"))) {
		report = ou.setCheat("rmDC++ in DC++ %[userVE] emulation mode" , true, false, BOOLSETTING(SHOW_RMDC_RAW));
		setClientType("rmDC++");
		ClientManager::getInstance()->sendAction(ou, SETTING(RMDC_RAW));
	}
	return report;
}

map<string, string> Identity::getReport() const {
	map<string, string> reportSet;
	string sid = getSIDString();
	{
		FastLock l(cs);
		for(InfIterC i = info.begin(); i != info.end(); ++i) {
			string name = string((char*)(&(i->first)), 2);
			string value = i->second;

    #define TAG(x,y) (x + (y << 8))

			// TODO: translate known tags and format values to something more readable
			switch(i->first) {
				case TAG('A','W'): name = "Away mode"; break;
				case TAG('B','O'): name = "Bot"; break;
				case TAG('B','C'): name = "Bad Client";break;//me
				case TAG('C','L'): name = "Client name"; break;
				case TAG('C','M'): name = "Comment"; break;
				case TAG('C','O'): name = "Connection"; break;
				case TAG('C','S'): name = "Cheat description"; break;
				case TAG('C','T'): name = "Client type"; break;
				case TAG('D','E'): name = "Description"; break;
				case TAG('D','S'): name = "Download speed"; value = Util::formatBytes(value) + "/s"; break;
				case TAG('E','M'): name = "E-mail"; break;
				case TAG('F','C'): name = "Fake Check status"; break;
				case TAG('F','D'): name = "Filelist disconnects"; break;
				case TAG('G','E'): name = "Filelist generator"; break;
				case TAG('F','B'): name = "Filelist Base";break;//me
				case TAG('F','I'): name = "Filelist CID";break;//me
				case TAG('H','N'): name = "Hubs Normal"; break;
				case TAG('H','O'): name = "Hubs OP"; break;
				case TAG('H','R'): name = "Hubs Registered"; break;
				case TAG('I','4'): name = "IPv4 Address"; value += " (" + Socket::getRemoteHost(value) + ")"; break;
				case TAG('I','6'): name = "IPv6 Address"; value += " (" + Socket::getRemoteHost(value) + ")"; break;
				case TAG('I','D'): name = "Client ID"; break;
				case TAG('K','P'): name = "KeyPrint"; break;
				case TAG('L','O'): name = "NMDC Lock"; break;
				case TAG('N','I'): name = "Nick"; break;
				case TAG('O','P'): name = "Operator"; break;
				case TAG('P','K'): name = "NMDC Pk"; break;
				case TAG('R','S'): name = "Shared bytes - real"; value = Text::fromT(Util::formatExactSize(Util::toInt64(value))); break;
				case TAG('S','F'): name = "Shared files"; break;
				case TAG('S','I'): name = "Session ID"; value = sid; break;
				case TAG('S','L'): name = "Slots"; break;
				case TAG('S','S'): name = "Shared bytes - reported"; value = Text::fromT(Util::formatExactSize(Util::toInt64(value))); break;
				//case TAG('S','T'): name = "NMDC Status"; value = Util::formatStatus(Util::toInt(value)); break;
				case TAG('S','U'): name = "Supports"; break;
				case TAG('T','A'): name = "Tag"; break;
				case TAG('T','O'): name = "Timeouts"; break;
				case TAG('U','4'): name = "IPv4 UDP port"; break;
				case TAG('U','6'): name = "IPv6 UDP port"; break;
				case TAG('U','S'): name = "Upload speed"; value = Util::formatBytes(value) + "/s"; break;
				case TAG('V','E'): name = "Client version"; break;
				case TAG('L','T'): name = "Login time"; break;
				case TAG('M','T'): name = "UserInfo"; break;
				case TAG('M','C'): name = "UserInfo count"; break;
				case TAG('T','S'): name = "TestSUR"; break;
				case TAG('A','H'): name = "All hubs count"; break;
				case TAG('A','1'): name = "ADL result File";break;
				case TAG('A','2'): name = "ADL result comment";break;
				case TAG('A','3'): name = "ADL result file size";break;
				case TAG('A','4'): name = "ADL result TTH";break; 
				case TAG('A','5'): name = "ADL result forbiden size";break;
				case TAG('A','6'): name = "ADL result total points";break; 
				case TAG('A','7'): name = "ADL result no. files";break; 
				case TAG('U','C'): name = "Commands";break;//me
				case TAG('I','C'): name = ""; break;
				case TAG('W','O'): name = ""; break;	// for GUI purposes
				default: name += " (unknown)";

			}

			if(!name.empty())
				reportSet.insert(make_pair(name, value));
		}
	}
	return reportSet;
}

//RSX++ //Protected users
bool Identity::isProtectedUser(const Client& c, bool OpBotHubCheck) const {
	if(isSet("PR") || getUser()->isSet(User::PROTECTED))
		return true;

	string RegProtect = SETTING(PROTECTED_USERS);
	/*if(!c.getUserProtected().empty()) {
		RegProtect = c.getUserProtected();
	}*/

	bool ret = false;
	if(OpBotHubCheck && (isOp() || isBot() || isHub())) {
		ret = true;
	} else if(SETTING(FAV_USER_IS_PROTECTED_USER) && FavoriteManager::getInstance()->isFavoriteUser(getUser())) {
		ret = true;
	} else if(!RegProtect.empty()) {
		/*if(RSXPP_BOOLSETTING(USE_WILDCARDS_TO_PROTECT)) {
			if(Wildcard::patternMatch(getNick(), RegProtect, '|')) {
				ret = true;
			}
		} else {*/
			if(RegexUtil::match(getNick(), RegProtect, true)) {
				ret = true;
			}
		/*}*/
		return false;
	}
	if(ret == true) {
		const_cast<Identity&>(*this).set("PR", "1");
	}
	return ret;
}

std::map<string, string> Identity::getInfo() const {
		std::map<string, string> ret;
		FastLock l(cs);
		for(InfIterC i = info.begin(); i != info.end(); ++i) {
			ret[string((char*)(&i->first), 2)] = i->second;
	}
	return ret;
  }
} // namespace dcpp
