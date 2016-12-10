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
