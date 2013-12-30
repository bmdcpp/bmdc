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

#ifndef HIGHLIGHTMANAGER_H
#define HIGHLIGHTMANAGER_H

#include "SettingsManager.h"
#include "typedefs.h"
#include "Singleton.h"
#include "ColorSettings.h"
#include "SimpleXML.h"

namespace dcpp {

class HighlightManager : public Singleton<HighlightManager>, private SettingsManagerListener
{
public:
	HighlightManager();
	~HighlightManager();

	ColorList* getList() {
		return &colorSettings;
	}

	void replaceList(ColorList &settings) {
		colorSettings.clear();
		colorSettings = settings;
	}

private:
	//store all highlights
	ColorList colorSettings;

	void load(SimpleXML& aXml);
	void save(SimpleXML& aXml);

	virtual void on(SettingsManagerListener::Load, SimpleXML& xml) noexcept;
	virtual void on(SettingsManagerListener::Save, SimpleXML& xml) noexcept;
};
}
#endif
