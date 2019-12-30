#include "Log.h"
#include <stdio.h>
#include <stdarg.h>

namespace vmc
{
	void logd(const char* format, ...)
	{
		va_list args;
		va_start(args, format);
		vprintf(format, args);
		va_end(args);
		printf("\n");
	}

	void loge(const char* format, ...)
	{
		va_list args;
		va_start(args, format);
		vprintf(format, args);
		va_end(args);
		printf("\n");
	}
}