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

#ifndef VERSION_H
#define VERSION_H

#ifdef WITH_LUAJIT
# define REVISION_GUID "{139e5bad-9998-44ae-8564-479c7886edff}"
# define LUAJIT " (LuaJIT)"
# define LUAJIT_LONG " Using " LUAJIT_VERSION " from " LUAJIT_URL "."
# define LUA_DIST LUAJIT_VERSION
#else
# define LUAJIT
# define LUAJIT_LONG
# define REVISION_GUID "{6a8a51d8-50a9-4155-aa4c-8569c7aaf85b}"
# define LUA_DIST LUA_RELEASE
#endif

#define PLUGIN_GUID REVISION_GUID											// UUID/GUID for this plugin project

#define PLUGIN_NAME "Clientside Lua Scripting" LUAJIT						// Name of the plugin
#define PLUGIN_AUTHOR "Crise (Lua API: BCDC++ Team)"						// Author of the plugin
#define PLUGIN_DESC "Adds BCDC++ like clientside lua support." LUAJIT_LONG	// Short description about the plugin
#define PLUGIN_VERSION 2.1													// Version of the plugin (note: not api version)
#define PLUGIN_WEB "N/A"													// Plugin website, set to "N/A" if none

#endif // VERSION_H