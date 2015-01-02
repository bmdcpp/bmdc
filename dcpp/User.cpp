/*
 * Copyright (C) 2001-2015 Jacek Sieka, arnetheduck on gmail point com
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
#include "User.h"
#include "AdcHub.h"
#include "FavoriteUser.h"
#include "format.h"
#include "GeoManager.h"
#include "StringTokenizer.h"
#include "RegEx.h"
#include "ClientManager.h"
// [BMDC++
#include "SettingsManager.h"
#include "FavoriteManager.h"
#include "DebugManager.h"
#include "DetectionManager.h"

namespace dcpp {

FastCriticalSection Identity::cs;

OnlineUser::OnlineUser(const UserPtr& ptr, Client& client_, uint32_t sid_) : identity(ptr, sid_), client(client_) {
    identity.isProtectedUser(client, true); // run init check
}

bool Identity::isTcpActive(const Client *c) const {
    if(c != NULL && user == ClientManager::getInstance()->getMe()) {
		return c->isActive(); // userlist should display our real mode
	} else {
        return isTcp4Active() || isTcp6Active();
	}
}

bool Identity::isTcp4Active() const {
	return (!user->isSet(User::NMDC)) ?
		!getIp4().empty() && supports(AdcHub::TCP4_FEATURE) :
		!user->isSet(User::PASSIVE);
}

bool Identity::isTcp6Active() const {
	return !getIp6().empty() && supports(AdcHub::TCP6_FEATURE);
}

bool Identity::isUdpActive() const {
	return isUdp4Active() || isUdp6Active();
}

bool Identity::isUdp4Active() const {
	if(getIp4().empty() || getUdp4Port().empty())
		return false;
	return user->isSet(User::NMDC) ? !user->isSet(User::PASSIVE) : supports(AdcHub::UDP4_FEATURE);
}

bool Identity::isUdp6Active() const {
	if(getIp6().empty() || getUdp6Port().empty())
		return false;
	return user->isSet(User::NMDC) ? false : supports(AdcHub::UDP6_FEATURE);
}

string Identity::getUdpPort() const {
	if(getIp6().empty() || getUdp6Port().empty()) {
		return getUdp4Port();
	}

	return getUdp6Port();
}

string Identity::getIp() const {
	return getIp6().empty() ? getIp4() : getIp6();
}

void Identity::getParams(ParamMap& params, const string& prefix, bool compatibility) const {
	{
		FastLock l(cs);
		for(InfMap::const_iterator i = info.begin(); i != info.end(); ++i) {
			params[prefix + string((char*)(&i->first), 2)] = i->second;
		}
	}
	if(user) {
		params[prefix + "SID"] =  getSIDString(); 
		params[prefix + "CID"] =  user->getCID().toBase32(); 
		params[prefix + "TAG"] =  getTag(); 
		params[prefix + "SSshort"] =  Util::formatBytes(get("SS")); 

		if(compatibility) {
			if(prefix == "my") {
				params["mynick"] =  getNick(); 
				params["mycid"] = user->getCID().toBase32();
			} else {
				params["nick"] = getNick(); 
				params["cid"] =  user->getCID().toBase32(); 
				params["ip"] =  get("I4"); 
				params["tag"] =  getTag(); 
				params["description"] =  get("DE"); 
				params["email"] =  get("EM"); 
				params["share"] =  get("SS"); 
				params["shareshort"] =  Util::formatBytes(get("SS")); 
				params["slots"] =  get("SL");
				/**/
				params["mode"] =  string(isTcpActive() ? "A" : "P");  
				params["newhubs"] = "H:" + get("HN") + "/" + get("HR") + "/" + get("HO");  
				//some simple names instead of Ax ;)
				params["adlFile"] =	 get("A1"); 
				params["adlComment"] =  get("A2");
				params["adlFileSize"] =  get("A3");
				params["adlTTH"] = 	get("A4");
				params["adlForbiddenSize"] =  get("A5");
				params["adlTotalPoints"] = get("A6");
				params["adlFilesCount"] = get("A7");
				params["adlFileSizeShort"] =  Util::formatBytes(get("A3"));
				params["adlForbiddenSizeShort"] =  Util::formatBytes(get("A5"));
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
	return "<" + getApplication() + ",M:" + string(isTcpActive() ? "A" : "P") +
		",H:" + get("HN") + "/" + get("HR") + "/" + get("HO") + ",S:" + get("SL") + ">";
}

string Identity::getApplication() const {
	string application = get("AP");
	string version = get("VE");

	if(version.empty()) {
		return application;
	}

	if(application.empty()) {
		// AP is an extension, so we can't guarantee that the other party supports it, so default to VE.
		return version;
	}

	return application + ' ' + version;
}

string Identity::getConnection() const {
	if(!get("US").empty())
		return string(Util::formatBytes(get("US")+_("/s")));
	return get("CO");
}

const string& Identity::getCountry() const {
	bool v6 = !getIp6().empty();
	return GeoManager::getInstance()->getCountry(v6 ? getIp6() : getIp4(), v6 ? GeoManager::V6 : GeoManager::V4);
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

std::map<string, string> Identity::getInfo() const {
	std::map<string, string> ret;
	FastLock l(cs);
	for(auto i = info.begin(); i != info.end(); ++i) {
		ret[string((char*)(&i->first), 2)] = i->second;
	}

	return ret;
}
//[BMDC++
void Identity::checkTagState(OnlineUser& ou) const {
	string usrTag = getTag();
	if(usrTag.empty()) return;
	bool isActive = ou.getIdentity().isTcpActive(NULL);

	if(isActive && (usrTag.find(",M:P,") != string::npos)) {
		ou.getClient().cheatMessage("*** " + getNick() + " - Tag states passive mode, but user is using active mode");
	} else if(!isActive && (usrTag.find(",M:A,") != string::npos)) {
		ou.getClient().cheatMessage("*** " + getNick() + " - Tag states active mode, but user is using passive mode");
	}
}
string Identity::setCheat(const Client& c, const string& aCheatDescription, bool aBadClient, bool aBadFilelist /*=false*/, bool aDisplayCheat /* = true*/) {
	ParamMap ucParams;
	getParams(ucParams, "user", true);
	string cheat = Util::formatParams(aCheatDescription, ucParams);
	string newCheat = Util::emptyString;

	string currentCS = get("CS");
	StringTokenizer<string> st(currentCS, ';');
	for(auto i = st.getTokens().begin(); i != st.getTokens().end(); ++i) {
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

	if(SETTING(DISPLAY_CHEATS_IN_MAIN_CHAT) && aDisplayCheat) {
		string report = "*** *** " +  getNick() + " - " + newCheat;
		return report;
	}
	return Util::emptyString;
}

string Identity::getPkVersion() const {
	string pk = get("PK");
	if(pk.find("DCPLUSPLUS") != string::npos && pk.find("ABCABC") != string::npos) {
		return pk.substr(10, pk.length() - 16);
	}
	return Util::emptyString;
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

string Identity::checkFilelistGenerator(OnlineUser& ou)
{
    if((get("FG") == "DC++ 0.403")) {
		if((RegEx::match<string>(getTag(), "^<StrgDC\\+\\+ V:1.00 RC([89]){1}")))  {
			string report = ou.setCheat("rmDC++ in StrongDC++ %[userVE] emulation mode" , true, false, true);
			setClientType("rmDC++");
			logDetection(true);
			ou.getClient().updated(ou);
			ClientManager::getInstance()->sendAction(ou, SETTING(RMDC_RAW));
			return report;
		}
	}

	if(!get("VE").empty() && strncmp(getTag().c_str(), "<++ V:", 6) == 0) {
		if((Util::toFloat(get("VE")) > 0.668)) {
			if(get("FI").empty() || get("FB").empty()) {
				string report = ou.setCheat("DC++ emulation", true, false, true);
				logDetection(true);
				ou.getClient().updated(ou);
				ClientManager::getInstance()->sendAction(ou,SETTING(DCPP_EMULATION_RAW));
				return report;
			}
		} else {
			if(!get("FI").empty() || !get("FB").empty()) {
				string report = ou.setCheat("DC++ emulation", true, false, true);
				logDetection(true);
				ou.getClient().updated(ou);
				ClientManager::getInstance()->sendAction(ou,SETTING(DCPP_EMULATION_RAW));
				return report;
			}
		}
	}


	if((RegEx::match<string>(get("FG"), "^DC\\+\\+.*"))) {
		if(!get("VE").empty() && (get("VE") != getFilelistGeneratorVer())) {
			string report = ou.setCheat("Filelist Version mis-match", false, true, SETTING(SHOW_FILELIST_VERSION_MISMATCH));
			logDetection(true);
			ou.getClient().updated(ou);
			ClientManager::getInstance()->sendAction(ou, SETTING(FILELIST_VERSION_MISMATCH));
			return report;
		}
	}

	return Util::emptyString;
}

string Identity::myInfoDetect(OnlineUser& ou) {
	checkTagState(ou);

	ParamMap params;
	getDetectionParams(params); // get identity fields and escape them, then get the rest and leave as-is
	const DetectionManager::DetectionItems& profiles = DetectionManager::getInstance()->getProfiles(params);

	for(auto i = profiles.begin(); i != profiles.end(); ++i) {
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


		for(auto j = INFList.begin(); j != INFList.end(); ++j) {
			string aPattern = Util::formatRegExp(j->second,params);
			string aField = getDetectionField(j->first);
			DETECTION_DEBUG("\t\tPattern: " + aPattern + " Field: " + aField);
			if(!RegEx::match<string>(aField, aPattern)) {
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
	uint64_t tick = GET_TICK();

	ParamMap params;
	getDetectionParams(params); // get identity fields and escape them, then get the rest and leave as-is
	const DetectionManager::DetectionItems& profiles = DetectionManager::getInstance()->getProfiles(params, true);//thinking//true

	for(auto i = profiles.begin(); i != profiles.end(); ++i) {
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

		for(auto j = INFList.begin(); j != INFList.end(); ++j) {
			string aPattern = Util::formatRegExp(j->second, params);
			string aField = getDetectionField(j->first);
			DETECTION_DEBUG("\t\tPattern: " + aPattern + " Field: " + aField);
			if(!RegEx::match<string>(aField, aPattern)) {
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
		logDetection(true);

		if(entry.checkMismatch && getUser()->isSet(User::NMDC) &&  (params["VE"]) != (params["PKVE"])) {
			setClientType(entry.name + " Version mis-match");
			return ou.setCheat(entry.cheat + " Version mis-match", true, false, ou.getClient().isActionActive(SETTING(VERSION_MISMATCH_RAW)));
		}

		string report = Util::emptyString;
		if(!entry.cheat.empty()) {
			report = ou.setCheat(entry.cheat, true, false, ou.getClient().isActionActive(entry.rawToSend));
		}

		ClientManager::getInstance()->sendAction(ou, entry.rawToSend);
		return report;
	}

	logDetection(false);
	setClientType("Unknown");
	return Util::emptyString;
}

void Identity::getDetectionParams(ParamMap& p) {
	getParams(p, Util::emptyString, true);
	p["PKVE"] = getPkVersion();

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
	for(ParamMap::iterator i = p.begin(); i != p.end(); ++i) {
		// looks really bad... but do the job
			Util::replace( "\\", "\\\\",   (i->second)); // this one must be first
			Util::replace( "[", "\\[",   (i->second));
			Util::replace( "]", "\\]",   (i->second));
			Util::replace( "^", "\\^",   (i->second));
			Util::replace( "$", "\\$",   (i->second));
			Util::replace( ".", "\\.",   (i->second));
			Util::replace( "|", "\\|",   (i->second));
			Util::replace( "?", "\\?",   (i->second));
			Util::replace( "*", "\\*",   (i->second));
			Util::replace( "+", "\\+",   (i->second));
			Util::replace( "(", "\\(",   (i->second));
			Util::replace( ")", "\\)",   (i->second));
			Util::replace( "{", "\\{",   (i->second));
			Util::replace( "}", "\\}",   (i->second));
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
	return Util::emptyString;
}

map<string, string> Identity::getReport() const
{
    map<string, string> reportSet;
	string sid = getSIDString();
	{
		FastLock l(cs);
		for(auto i = info.begin(); i != info.end(); ++i) {
			string name = string((char*)(&(i->first)), 2);
			string value = i->second;

            #define TAG(x,y) (x + (y << 8))

			// TODO: translate known tags and format values to something more readable
			switch(i->first) {
				case TAG('A','W'): name = "Away mode"; break;
				case TAG('A','P'): name = "Application"; break;
				case TAG('B','O'): name = "Bot"; break;
				case TAG('B','C'): name = "Bad Client";break;
				case TAG('B','F'): name = "Bad Filelist";break;
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
				case TAG('F','S'): name = "Free Slots"; break;
				case TAG('G','E'): name = "Filelist generator"; break;
				case TAG('F','B'): name = "Filelist Base";break;
				case TAG('F','I'): name = "Filelist CID";break;
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
				case TAG('R','F'): name = "Refferer url"; break;
				case TAG('R','S'): name = "Shared bytes - real"; value = Util::formatExactSize(Util::toInt64(value)); break;
				case TAG('S','F'): name = "Shared files"; break;
				case TAG('S','I'): name = "Session ID"; value = sid; break;
				case TAG('S','L'): name = "Slots"; break;
				case TAG('S','S'): name = "Shared bytes - reported"; value = Util::formatExactSize(Util::toInt64(value)); break;
				case TAG('S','U'): name = "Supports"; break;
				case TAG('T','A'): name = "Tag"; break;
				case TAG('T','O'): name = "Timeouts"; break;
				case TAG('U','4'): name = "IPv4 UDP port"; break;
				case TAG('U','6'): name = "IPv6 UDP port"; break;
				case TAG('U','S'): name = "Upload speed"; value = Util::formatBytes(value) + "/s"; break;
				case TAG('V','E'): name = "Client version"; break;
				case TAG('L','C'): name = "Client Locale"; break;
				case TAG('L','T'): name = "Login time"; break;
				case TAG('L','S'): name = "FileList size"; break;
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
				case TAG('U','C'): name = "Commands";break;
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

//sumary detection
void Identity::logDetection(bool successful) {
	SettingsManager *sm = SettingsManager::getInstance();
	if(sm != NULL)
	{
		if(successful) {
			int a = SETTING(DETECTIONS);
			sm->set(SettingsManager::DETECTIONS , ++a);
		} else {
			int b = SETTING(DETECTIONF);
			sm->set(SettingsManager::DETECTIONF , ++b);
		}
	}
}

void FavoriteUser::update(const OnlineUser& info) {
	setNick(info.getIdentity().getNick());
	setUrl(info.getClient().getHubUrl());
	setLastSeen(time(NULL));
	setCid(info.getUser()->getCID().toBase32());
}

bool OnlineUser::isCheckable(uint32_t delay /* = 0*/)
{
	if(identity.isBot())
		return false;
	if(identity.isHub())
		return false;
	if(identity.isHidden())
		return false;
	if(delay == 0)
		return true;
	return (GET_TICK() - identity.getLoggedIn()) > delay;
}

//RSX++ //checking stuff
bool OnlineUser::getChecked(bool filelist/* = false*/, bool checkComplete/* = true*/) {
	if(!identity.isTcpActive() && !getClient().isActive()) {
		identity.setClientType("[Passive]");
		identity.setTestSURChecked("1");
		identity.setFileListChecked("1");
		return true;
	} else if(getUser()->isSet(User::OLD_CLIENT)) {
		identity.setTestSURChecked("1");
		identity.setFileListChecked("1");
		return true;
	} else if(isProtectedUser()) {
		if((SETTING(UNCHECK_CLIENT_PROTECTED_USER) && !filelist) || (SETTING(UNCHECK_LIST_PROTECTED_USER) && filelist)) {
			identity.setClientType("[Protected]");
			identity.setTestSURChecked("1");
			identity.setFileListChecked("1");
			return true;
		}
	}
	if(checkComplete) //prevent double checking (shouldCheckClient/Filelist)
		return filelist ? identity.isFileListChecked() : identity.isClientChecked();
	return false;
}
////Protected users
bool Identity::isProtectedUser(const Client& c, bool OpBotHubCheck) const {
	if(isSet("PR") || getUser()->isSet(User::PROTECT))
		return true;

	string RegProtect = SETTING(PROTECTED_USERS);
	if(!c.getProtectUser().empty()) {
		RegProtect += c.getProtectUser();
	}

	bool ret = false;
	if(OpBotHubCheck && (isOp() || isBot() || isHub())) {
		ret = true;
	} else if(SETTING(FAV_USER_IS_PROTECTED_USER) && FavoriteManager::getInstance()->isFavoriteUser(getUser())) {
		ret = true;
	} else if(!RegProtect.empty()) {
		if(SETTING(USE_WILDCARDS_TO_PROTECT)) {
			if(Wildcard::match<string>(getNick(), RegProtect, '|')) {
				ret = true;
			}
		} else {
			if(RegEx::match<string>(getNick(), RegProtect, true)) {
				ret = true;
			}
		}
		return false;
	}
	if(ret) {
		const_cast<Identity&>(*this).set("PR", "1");
	}
	return ret;
}

UserData* OnlineUser::getPluginObject() noexcept {
	resetEntity();

	pod.nick = pluginString(getIdentity().getNick());
	pod.hubHint = pluginString(getClient().getHubUrl());
	pod.cid = pluginString(getUser()->getCID().toBase32());
	pod.sid = getIdentity().getSID();
	pod.protocol = getUser()->isNMDC() ? PROTOCOL_NMDC : PROTOCOL_ADC;
	pod.isOp = getIdentity().isOp() ? True : False;

	return &pod;
}

} // namespace dcpp
