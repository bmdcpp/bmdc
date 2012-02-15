/*
 * Copyright (C) 2012 Jacek Sieka, arnetheduck on gmail point com
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

#ifndef PLUGIN_H
#define PLUGIN_H

#include "version.h"

#ifdef _WIN32
extern HINSTANCE hInst;
#endif

/* Settings helpers */
const char* DCAPI get_cfg(const char* name);
int32_t DCAPI get_cfg_int32(const char* name);
int64_t DCAPI get_cfg_int64(const char* name);

void DCAPI set_cfg(const char* name, const char* value);
void DCAPI set_cfg_int32(const char* name, int32_t value);
void DCAPI set_cfg_int64(const char* name, int64_t value);

/* Plugin event processing (listeners could have own functions) */
Bool DCAPI pluginProc(uint32_t eventId, dcptr_t pData);

#endif /* PLUGIN_H */
