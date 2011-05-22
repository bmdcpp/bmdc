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

#include "stdinc.h"
#include "DCPlusPlus.h"

#include "HighlightManager.h"

namespace dcpp {
HighlightManager::HighlightManager(void)
{
	SettingsManager::getInstance()->addListener(this);
}

HighlightManager::~HighlightManager(void)
{
	SettingsManager::getInstance()->removeListener(this);
}

void HighlightManager::load(SimpleXML& aXml){
	aXml.resetCurrentChild();

	if(aXml.findChild("Highlights")) {
		aXml.stepIn();
		while(aXml.findChild("Highlight")) {
			ColorSettings cs;
			cs.setMatch( aXml.getChildAttrib("Match") );
			cs.setBold(	aXml.getBoolChildAttrib("Bold") );
			cs.setItalic( aXml.getBoolChildAttrib("Italic") );
			cs.setUnderline( aXml.getBoolChildAttrib("Underline") );
			cs.setStrikeout( aXml.getBoolChildAttrib("Strikeout") );
			cs.setIncludeNick( aXml.getBoolChildAttrib("IncludeNick") );
			cs.setCaseSensitive( aXml.getBoolChildAttrib("CaseSensitive") );
			cs.setWholeLine( aXml.getBoolChildAttrib("WholeLine") );
			cs.setWholeWord( aXml.getBoolChildAttrib("WholeWord") );
			cs.setPopup( aXml.getBoolChildAttrib("Popup") );
			cs.setTab( aXml.getBoolChildAttrib("Tab") );
			cs.setPlaySound( aXml.getBoolChildAttrib("PlaySound") );
			cs.setNoti( aXml.getChildAttrib("NotifyStr") );
			cs.setMatchType( aXml.getIntChildAttrib("MatchType") );
			cs.setHasFgColor( aXml.getBoolChildAttrib("HasFgColor") );
			cs.setHasBgColor( aXml.getBoolChildAttrib("HasBgColor") );
			cs.setBgColor( aXml.getChildAttrib("BgColor") );
			cs.setFgColor( aXml.getChildAttrib("FgColor") );
			cs.setSoundFile( aXml.getChildAttrib("SoundFile") );

			colorSettings.push_back(cs);
		}
		aXml.stepOut();
	} else {
		aXml.resetCurrentChild();
	}
}

void HighlightManager::save(SimpleXML& aXml){
	aXml.addTag("Highlights");
	aXml.stepIn();

	ColorIter iter = colorSettings.begin();
	for(;iter != colorSettings.end(); ++iter) {
		aXml.addTag("Highlight");

		aXml.addChildAttrib("Match", (*iter).getMatch());
		aXml.addChildAttrib("Bold", (*iter).getBold());
		aXml.addChildAttrib("Italic", (*iter).getItalic());
		aXml.addChildAttrib("Underline", (*iter).getUnderline());
		aXml.addChildAttrib("Strikeout", (*iter).getStrikeout());
		aXml.addChildAttrib("IncludeNick", (*iter).getIncludeNick());
		aXml.addChildAttrib("CaseSensitive", (*iter).getCaseSensitive());
		aXml.addChildAttrib("WholeLine", (*iter).getWholeLine());
		aXml.addChildAttrib("WholeWord", (*iter).getWholeWord());
		aXml.addChildAttrib("Popup", (*iter).getPopup());
		aXml.addChildAttrib("Tab", (*iter).getTab());
		aXml.addChildAttrib("PlaySound", (*iter).getPlaySound());
		aXml.addChildAttrib("NotifyStr", (*iter).getNoti());
		aXml.addChildAttrib("MatchType", (*iter).getMatchType());
		aXml.addChildAttrib("HasFgColor", (*iter).getHasFgColor());
		aXml.addChildAttrib("HasBgColor", (*iter).getHasBgColor());
		aXml.addChildAttrib("FgColor", (*iter).getFgColor());
		aXml.addChildAttrib("BgColor", (*iter).getBgColor());
		aXml.addChildAttrib("SoundFile", (*iter).getSoundFile());
	}//end for

	aXml.stepOut();
}

void HighlightManager::on(SettingsManagerListener::Load, SimpleXML& xml) throw(){
	load(xml);
}

void HighlightManager::on(SettingsManagerListener::Save, SimpleXML& xml) throw() {
	save(xml);
}
}
