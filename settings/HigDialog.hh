/* 
* (C) 2025 BMDC
* This file is part of BMDC.
* BMDC is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 2 of the License, or (at your option) any later version.
*
* BMDC is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License along with Xfce-nameday-plugin. If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef _HIG_DIALOG
#define _HIG_DIALOG_

#include "../dcpp/stdinc.h"
#include "../dcpp/GetSet.h"
#include "../dcpp/Util.h"
#include "../dcpp/ColorSettings.h"
#include "../linux/entry.hh"

class HigDialog: public Entry
{
	public:
		HigDialog(dcpp::ColorSettings *_cs,bool add);
		GtkWidget* getContainer() { return dialogWin;}
		bool run();
	private:
		GtkWidget* dialogWin, *mainBox,
		*entryName, *comboType,
		*sBold,*sItalic, *sUnderline,
		*sNoti,*sTab,*colorFgButton,*colorBgButton;
		bool init;
		dcpp::ColorSettings *cs;
};
#else
class HigDialog;
#endif
