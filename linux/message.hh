/*
 * Copyright © 2009-2010 freedcpp, http://code.google.com/p/freedcpp
 * Copyright © 2011-2017 BMDC++ http://launchpad.net/bmdc++
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
  * Added class Tag: Used for Chat "Text" Tags
  * Added class CActions: Used for "right click action
  * */

#ifndef _BMDC_MSG_TYPE_HH
#define _BMDC_MSG_TYPE_HH

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
/*
	 * 0 - Browse FL
	 * 1 - Nick to Chat
	 * 2 - PM to nick
	 * 3 - match quene
	 * 4 - grant slot
	 * 5 - add to fav users
	 * 6 - Partial FL	
*/
class CActions
{
	public:
	typedef enum
	{
		BROWSE = 0,
		NICK_TO_CHAT,
		PM_TO_NICK,
		MATCH_Q,
		GRANT_SLOT,
		ADD_AS_FAV,
		GET_PARTIAL_FILELIST
	} User;
};

#endif
