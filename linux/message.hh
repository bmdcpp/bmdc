/*
 * Copyright © 2009-2010 freedcpp, http://code.google.com/p/freedcpp
 * Copyright © 2011-2012 BMDC++ http://launchpad.net/bmdc++
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * In addition, as a special exception, compiling, linking, and/or
 * using OpenSSL with this program is allowed.
 */
 /*
  * Changelog BMDC++:
  * Added class Tag
  * */

#ifndef MSG_TYPE_HH
#define MSG_TYPE_HH

class Msg
{
	public:
		typedef enum
		{
			GENERAL,
			PRIVATE,
			MYOWN,
			SYSTEM,
			STATUS,
			CHEAT,
			FAVORITE,
			OPERATOR,
			UNKNOWN
		} TypeMsg;
};

class Tag
{
    public:
        typedef enum
		{
			TAG_FIRST = 0,
			TAG_GENERAL = TAG_FIRST,
			TAG_PRIVATE,
			TAG_MYOWN,
			TAG_SYSTEM,
			TAG_STATUS,
			TAG_CHEAT,
			TAG_TIMESTAMP,
			/*-*/
			TAG_MYNICK,
			TAG_NICK,
			TAG_OPERATOR,
			TAG_FAVORITE,
			TAG_URL,
			TAG_IPADR,
			TAG_LAST
		} TypeTag;
};

#endif
