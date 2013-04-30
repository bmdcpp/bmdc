/* 
 * Copyright (C) 2001-2013 Jacek Sieka, arnetheduck on gmail point com
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

/** @file implement UI-specific functions of the plugin API. */

#include <dcpp/stdinc.h>
#include <dcpp/DCPlusPlus.h>
#include <dcpp/PluginApiImpl.h>
#include <dcpp/Text.h>
#include "wulformanager.hh"
#include "sound.hh"

namespace dcpp {

// Functions for DCUI
void PluginApiImpl::playSound(const char* path) {
	Sound::get()->playSound(string(path));
}

void PluginApiImpl::notify(const char* title, const char* message) {
	Notify::get()->showNotify(string(title), string(message), Notify::PLUGINS);	
	
}

} // namespace dcpp
