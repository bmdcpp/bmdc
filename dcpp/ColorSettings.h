/* 
* Copyright (C) 2003-2005 P�r Bj�rklund, per.bjorklund@gmail.com
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

#ifndef COLORSETTINGS_H
#define COLORSETTINGS_H

#include "stdinc.h"
#include "DCPlusPlus.h"

namespace dcpp {
class ColorSettings
{
public:
	ColorSettings(): bTimestamps(false), bUsers(false), bMyNick(false), bUsingRegexp(false), 
		strMatch(Util::emptyStringT), strSoundFile(Util::emptyStringT), bWholeWord(false), 
		bWholeLine(false), bIncludeNick(false), bCaseSensitive(false), bPopup(false), bTab(false),
		bPlaySound(false), bBold(false), bUnderline(false), bItalic(false), bStrikeout(false), 
		bNoti(Util::emptyStringT), iMatchType(1), iBgColor(Util::emptyStringT), iFgColor(Util::emptyStringT), bHasBgColor(false),
		bHasFgColor(false) {	}
	~ColorSettings(){};

	GETSET(bool, bWholeWord, WholeWord);
	GETSET(bool, bWholeLine, WholeLine);
	GETSET(bool, bIncludeNick, IncludeNick);
	GETSET(bool, bCaseSensitive, CaseSensitive);
	GETSET(bool, bPopup, Popup);
	GETSET(bool, bTab, Tab);
	GETSET(bool, bPlaySound, PlaySound);
	GETSET(bool, bBold, Bold);
	GETSET(bool, bUnderline, Underline);
	GETSET(bool, bItalic, Italic);
	GETSET(bool, bStrikeout, Strikeout);
	GETSET(string, bNoti, Noti);
	GETSET(int,  iMatchType, MatchType);
	GETSET(string,  iBgColor, BgColor);
	GETSET(string,  iFgColor, FgColor);
	GETSET(bool, bHasBgColor, HasBgColor);
	GETSET(bool, bHasFgColor, HasFgColor);
	GETSET(string, strSoundFile, SoundFile);
    	
	void setMatch(string match){
		if( match.compare(("$ts$")) == 0){
			bTimestamps = true;
		}else if(match.compare(("$users$")) == 0) {
			bUsers = true;
		}else if(match.find(("$mynick$")) != tstring::npos) {
			bMyNick = true;
		} else if(match.find(("$Re:")) == 0) {
			bUsingRegexp = true;
		}
		strMatch = match;
	}

	bool getUsers() { return bUsers; }
	bool getTimestamps() { return bTimestamps; }
	bool getMyNick() { return bMyNick; }
	bool usingRegexp() { return bUsingRegexp; }

	const string & getMatch() { return strMatch; }
	
private:
	//string to match against
	string strMatch;

	bool bTimestamps;
	bool bUsers;
	bool bMyNick;
	bool bUsingRegexp;

};
}
#endif
