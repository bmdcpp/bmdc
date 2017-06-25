
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
// MA 02110-1301, USA.
// 


#include "format.h"
namespace dcpp {
	
const size_t my_autosprintf_buffer_size = 4096;

static char my_autosprintf_buffer[my_autosprintf_buffer_size] = { 0, };

std::string my_autosprintf_va(const char *format, va_list ap)
{
	if (!format || format[0] == '\0')
		return std::string();

	int res = vsnprintf(my_autosprintf_buffer, my_autosprintf_buffer_size, format, ap);
	return res > 0 ? std::string(my_autosprintf_buffer) : std::string();
}

std::string my_autosprintf(const char *format, ...)
{
	va_list ap;
	va_start(ap, format);
	std::string res = my_autosprintf_va(format, ap);
	va_end(ap);
	return res;
}

} //dcpp
