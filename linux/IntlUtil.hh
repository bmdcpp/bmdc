/*
 * Copyright © 2011 Jens Oknelid, paskharen@gmail.com
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

#ifndef LINUXDCPP_INTL_UTIL_HH
#define LINUXDCPP_INTL_UTIL_HH

#include <boost/format.hpp>
#include <string>
#include <exception>
#include <glib/gi18n.h>
#include <errno.h>

class IntlUtil
{
	public:
		// Initialize i18n support
		static void initialize()
		{
			if (bindtextdomain(PACKAGE, _DATADIR "/locale") == NULL)
				throw std::runtime_error(strerror(errno));

			if (textdomain(PACKAGE) == NULL)
				throw std::runtime_error(strerror(errno));

			if (bind_textdomain_codeset(PACKAGE, "UTF-8") == NULL)
				throw std::runtime_error(strerror(errno));
		}

		static inline boost::format message_format(const char *text)
		{
			boost::format fmt(text);
			fmt.exceptions(boost::io::no_error_bits);
			return fmt;
		}

		static inline boost::format message_format(const std::string &text)
		{
			return message_format(text.c_str());
		}
};

#define F_(text, params) (IntlUtil::message_format(_(text)) params).str()
#define P_(text, text_plural, params, n) (IntlUtil::message_format(g_dngettext(NULL, text, text_plural, n)) params).str()

#endif /* LINUXDCPP_INTL_UTIL_HH */

