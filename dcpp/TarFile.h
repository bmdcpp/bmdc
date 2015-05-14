/*
 * Copyright (C) 2011-2015 Mank, freedcpp at seznam dot cz
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
#ifdef HAVE_LIBTAR

#if !defined(GZIP_H)
#define GZIP_H
#include "stdinc.h"
#include "typedefs.h"

namespace dcpp {

class TarFile
{
	public:
		static void CreateTarredFile(const string& _path, const StringPairList& files);
		static void DecompresTarredFile(const string& _file, const string& _prefix);
};

}
#endif

#endif
