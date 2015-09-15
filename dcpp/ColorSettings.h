/*
* Copyright (C) 2003-2014 Pär Björklund, per.bjorklund@gmail.com
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

#ifndef _COLORSETTINGS_H
#define _COLORSETTINGS_H

#include "stdinc.h"
#include "DCPlusPlus.h"
#include "Flags.h"

namespace dcpp {
class ColorSettings: public Flags
{
  public:
	enum ColorFlags {
		CONTEXT_CHAT = 1,
		CONTEXT_NICKLIST = 2,
		CONTEXT_FILELIST = 3
	};
  
	ColorSettings(): Flags(CONTEXT_CHAT), bIncludeNick(false), bCaseSensitive(false), bPopup(false), bTab(false),
		bPlaySound(false), bBold(false), bUnderline(false), bItalic(false),
		bNoti(Util::emptyString), iMatchType(1), iBgColor(Util::emptyString), iFgColor(Util::emptyString), bHasBgColor(false),
		bHasFgColor(false) , strSoundFile(Util::emptyString), strMatch(Util::emptyString), bUsingRegexp(false)  {	}
	~ColorSettings(){ };

private:
	//string to match against
	string strMatch;	
public:	
	GETSET(string, bNoti, Noti);
	GETSET(string,  iBgColor, BgColor);
	GETSET(string,  iFgColor, FgColor);
	GETSET(string, strSoundFile, SoundFile);
	GETSET(int,  iMatchType, MatchType);
	GETSET(bool, bIncludeNick, IncludeNick);
	GETSET(bool, bCaseSensitive, CaseSensitive);
	GETSET(bool, bPopup, Popup);
	GETSET(bool, bTab, Tab);
	GETSET(bool, bPlaySound, PlaySound);
	GETSET(bool, bBold, Bold);
	GETSET(bool, bUnderline, Underline);
	GETSET(bool, bItalic, Italic);
	GETSET(bool, bHasBgColor, HasBgColor);
	GETSET(bool, bHasFgColor, HasFgColor);

	void setMatch(string match){
		if(match.find(("$Re:")) == 0) {
			bUsingRegexp = true;
		}
		strMatch = match;
	}
	bool usingRegexp() const { return bUsingRegexp; }

	const string & getMatch() const { return strMatch; }
	int getFlag() const
	{
		if(isSet(CONTEXT_CHAT))
			return 1;
		if(isSet(CONTEXT_NICKLIST))
			return 2;
		if(isSet(CONTEXT_FILELIST))
			return 3;
		return 0;	
	}
private:
	bool bUsingRegexp;

   };
}
#endif
