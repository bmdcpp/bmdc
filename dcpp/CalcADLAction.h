/*
* Copyright © 2011-2025 BMDC
* This file is part of BMDC.
* BMDC is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 2 of the License, or (at your option) any later version.
*
* BMDC is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License along with Xfce-nameday-plugin. If not, see <https://www.gnu.org/licenses/>.
*/
#ifndef _BMDC_CALC_ADL_H
#define _BMDC_CALC_ADL_H

#include "stdinc.h"
#include "DCPlusPlus.h"
#include "typedefs.h"
#include "Singleton.h"
#include "SettingsManager.h"
#include "CriticalSection.h"

namespace dcpp {
	
class SimpleXML;
class CalcADLAction: public Singleton<CalcADLAction>, public SettingsManagerListener
{
	public:
		// custom points system
		void calcADLAction(int aPoints, int& a, bool& d);

		IntMap& getADLPoints() { Lock l(cs); return points; }
		void updateADLPoints(IntMap& p) {
			Lock l(cs);
			points = p;
		}
	private:
		//clang
		using SettingsManagerListener::on;
		IntMap points;
		CriticalSection cs;
		void on(SettingsManagerListener::Load, SimpleXML& xml);
		void on(SettingsManagerListener::Save, SimpleXML& xml);

};
}//dcpp
#endif
