/*
 * Copyright (C) 2001-2012 Jacek Sieka, arnetheduck on gmail point com
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

#include "version.h"

#ifndef DCPP_REVISION
//#define DCPP_REVISION 2869
#define DCPP_REVISION 2923
#define DCPP_REVISION_STRING "2923p"
#endif
#ifndef BMDC_REVISION
	#define BMDC_REVISION 120
#endif
	#define BMDC_REVISION_STRING "120"

#define xstrver(s) strver(s)
#define strver(s) #s

namespace dcpp {
const string fullVersionString(APPNAME " r" xstrver(BMDC_REVISION) " / v" VERSIONSTRING " (r" xstrver(DCPP_REVISION) ")");
}

