/*
 * Copyright (C) 2001-2017 Jacek Sieka, arnetheduck on gmail point com
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

#ifndef DCPLUSPLUS_DCPP_FORMAT_H_
#define DCPLUSPLUS_DCPP_FORMAT_H_
#include <string>

#ifdef _WIN32
#include <libintl.h>
#endif
#include <glib/gi18n.h>

#ifndef PACKAGE
#define PACKAGE "libdcpp"
#endif

#define LOCALEDIR dcpp::Util::getPath(Util::PATH_LOCALE).c_str()
#define F_(String) _(string(String).c_str())
//#endif
/*
#ifdef _WIN32
#define _(String) gettext(String)
#define N_(String) gettext(String)
#endif
*/

namespace dcpp {

#include <stdio.h>
#include <stdarg.h>
std::string my_autosprintf_va(const char *format, va_list ap);
std::string my_autosprintf(const char *format, ...);
#define autosprintf my_autosprintf

}

#endif /* DCPLUSPLUS_DCPP_FORMAT_H_ */
