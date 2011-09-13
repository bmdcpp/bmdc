/* 
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

#ifndef RSXUTIL_H
#define RSXUTIL_H

namespace dcpp {

class RsxUtil {
public:
	static void		init();
	static void		uinit();
	static bool		checkVersion(const string& tag);
/*	static string			toIP(const uint32_t ipnum);
	static uint32_t			toIpNumber(const string& aIp);
	static uint32_t			getUpperRange(const string& aRange);
	static uint32_t			getLowerRange(const string& aRange);
*/
	static void		generateTestSURString();
	static const string&	getTestSURString();
//	static string			getUpdateFileNames(const int number);
/*	static bool				toBool(const string& aString);
	static bool				toBool(const int aNumber);
	static void				trim(string& source, const string& delims = " \t\r\n");
	static tstring			replace(const tstring& aString, const tstring& fStr, const tstring& rStr);
	static string			replace(const string& aString, const string& fStr, const string& rStr);
	static bool				compareLower(const string& firstStr, const string& secondStr);
	static bool				compareLower(const tstring& firstStr, const tstring& secondStr);
	static bool				isIpInRange(const string& aIp, const string& aRange);
*/
private:
	static string tmpTestSur;
};

}; // namespace dcpp

#endif //RSXUTIL_H
