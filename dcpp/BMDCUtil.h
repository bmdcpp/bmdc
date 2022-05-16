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

#ifndef BUTIL_H
#define BUTIL_H

namespace dcpp {

class bmUtil {
public:
	static void		init();
	static bool		checkVersion(const string& tag);
	static uint32_t			toIpNumber(const string& aIp);
	static void generateTestSURString();

	static const string&	getTestSURString();
	static bool isIpInRange(const string& aIp, const string& aRange);

private:
	static string tmpTestSur;
};

}; // namespace dcpp

#endif 
