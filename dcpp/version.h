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

#define APPNAME "BMDC++"

#define VERSIONSTRING "0.800"
#define VERSIONFLOAT 0.800

#ifndef DCPP_REVISION
	#define DCPP_REVISION 2972
	#define DCPP_REVISION_STRING "2972p"
#endif
#ifndef BMDC_REVISION
	#define BMDC_REVISION 191
#endif
#ifndef BMDC_REVISION_STRING
	#define BMDC_REVISION_STRING "191"
#endif
#define xstrver(s) strver(s)
#define strver(s) #s

namespace dcpp {
extern const string fullVersionString;
}
