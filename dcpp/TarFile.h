/*
* Copyright © 2011-2025 BMDC
* This file is part of BMDC.
* BMDC is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 2 of the License, or (at your option) any later version.
*
* BMDC is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License along with Xfce-nameday-plugin. If not, see <https://www.gnu.org/licenses/>.
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
		/*
		 * CreateTarredFile: tar files
		 * @path: string for path where
		 * @files: wich files
		 * */
		static void CreateTarredFile(const string& _path, const StringPairList& files);
		static void DecompresTarredFile(const string& _file, const string& _prefix);
};

}
#endif

#endif
