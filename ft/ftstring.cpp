/////////////////////////////////////////////////////////////////////////
// (c) 2009 Brian Waters (bdwaters@sbcglobal.net) All rights reserved. //
/////////////////////////////////////////////////////////////////////////
#include "ftbase.h"
#include "ftstring.h"

#ifdef FT_GCC
#include <stdarg.h>
#endif

FTString& FTString::format(cpChar pszFormat, ...)
{
    Char szBuff[2048];
    va_list pArgs;
    va_start(pArgs, pszFormat);
#if defined(FT_WINDOWS)
    _vsnprintf_s(szBuff, sizeof(szBuff), pszFormat, pArgs);
#elif defined(FT_GCC)
    vsnprintf(szBuff, sizeof(szBuff), pszFormat, pArgs);
#elif defined(FT_SOLARIS)
    vsnprintf(szBuff, sizeof(szBuff), pszFormat, pArgs);
#else
#error "Unrecoginzed platform"
#endif
    va_end(pArgs);

    assign(szBuff);

    return *this;
}
