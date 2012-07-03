/*
 * Copyright (C) 2007-2010 Crise, crise<at>mail.berlios.de
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

#ifndef LUA_PLUGIN_STDAFX_H
#define LUA_PLUGIN_STDAFX_H

#ifdef _WIN32

#ifndef _WIN32_WINNT              
#define _WIN32_WINNT 0x0501
#endif

#define STRICT
#define WIN32_LEAN_AND_MEAN

#ifdef _MSC_VER

//disable the deprecated warnings for the CRT functions.
#define _CRT_SECURE_NO_DEPRECATE 1
#define _ATL_SECURE_NO_DEPRECATE 1
#define _CRT_NON_CONFORMING_SWPRINTFS 1

#if _MSC_VER == 1400 || _MSC_VER == 1500
#define _CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES 1
//disable the deprecated warnings for the crt functions.
#pragma warning(disable: 4996)
#endif
#endif

#include <windows.h>
#include <tchar.h>

#else
#include <unistd.h>
#endif

// This can make a #define value to string (from boost)
#define STRINGIZE(X) DO_STRINGIZE(X)
#define DO_STRINGIZE(X) #X

#include <string>

#include <cstdint>
#include <PluginDefs.h>

#ifdef UNICODE
typedef std::wstring tstring;
#else
typedef std::string tstring;
#endif

#ifdef WITH_BOOST_CS
# define BOOST_ALL_NO_LIB
# ifdef _WIN32
#  define BOOST_USE_WINDOWS_H
# endif
#endif

#ifndef __cplusplus
# include <stdio.h>
# include <stdlib.h>
# include <string.h>
#else
# include <cstdio>
# include <cstdlib>
# include <cstring>
#endif


#endif // LUA_PLUGIN_STDAFX_H
