/*
* Copyright (c) 2009-2019 Brian Waters
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*    http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

#ifndef __ftbase_h_included
#define __ftbase_h_included

#define True	true
#define False	false

////////////////////////////////////////////////////
// GCC                                            //
////////////////////////////////////////////////////
#if defined(__GNUG__)
#define FT_GCC

#include <sys/time.h>
#include <signal.h>
#include <string.h>

typedef long long int fttime_t;

#define FT_FILENAME_MAX	FILENAME_MAX

#define ft_gets_s(a,b) __builtin_gets(a)
#define ft_strncpy_s(a,b,c,d) __builtin_strncpy(a,c,d);
#define ft_sscanf_s __builtin_sscanf
#define ft_sprintf_s __builtin_snprintf
#define ft_vsnprintf_s __builtin_vsnprintf
#define ft_strcpy_s(strDestination, sizeInBytes, strSource)  __builtin_strncpy(strDestination, strSource, sizeInBytes)
#define ft_strdup(str) __builtin_strdup(str)
#define ft_strnicmp(str1, str2, count) __builtin_strncasecmp(str1, str2, count)

#define ft_localtime_s(a,b) localtime_r(b,a)
#define ft_gmtime_s(a,b) gmtime_r(b,a)

#define ft_fseek(a,b,c) fseek(a,b,c)
#define ft_access(a,b) access(a,b)
#define FT_ACCESS_F_OK F_OK
#define FT_ACCESS_R_OK R_OK
#define FT_ACCESS_W_OK W_OK

#define ft_atoll(a) atoll(a)
#define ft_atoull(a) strtoull(a,NULL,10)
////////////////////////////////////////////////////
// WIN64                                          //
////////////////////////////////////////////////////
#elif defined (WIN64)
#define FT_WINDOWS
#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers

#include <windows.h>
// atl
#include <atlbase.h>
#include <atlcom.h>
#include <comdef.h>
//#include <atlcomtime.h>
using namespace ATL;

#include <io.h>

typedef __int64 fttime_t;

#define FT_FILENAME_MAX	FILENAME_MAX

#define ft_gets_s(a,b) gets_s(a,b)
#define ft_strncpy_s strncpy_s
#define ft_sscanf_s sscanf_s
#define ft_sprintf_s sprintf_s
#define ft_vsnprintf_s _vsnprintf_s
#define ft_strcpy_s(strDestination, sizeInBytes, strSource)  strcpy_s(strDestination, sizeInBytes, strSource)
#define ft_strdup(str) _strdup(str)
#define ft_strnicmp(str1, str2, count) _strnicmp(str1, str2, count)

#define ft_localtime_s(a,b) _localtime64_s(a,b)
#define ft_gmtime_s(a,b) _gmtime64_s(a,b)

#define ft_fseek(a,b,c) _fseeki64(a,b,c)
#define ft_access(a,b) _access(a,b)
#define FT_ACCESS_F_OK 0
#define FT_ACCESS_R_OK 2
#define FT_ACCESS_W_OK 4

#define ft_atoll(a) _strtoi64(a,NULL,10)
#define ft_atoull(a) _strtoui64(a,NULL,10)
////////////////////////////////////////////////////
// WIN32                                          //
////////////////////////////////////////////////////
#elif defined(WIN32)
#define FT_WINDOWS
#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers

#include <windows.h>
// atl
#include <atlbase.h>
#include <atlcom.h>
#include <comdef.h>
//#include <atlcomtime.h>
using namespace ATL;

#include <io.h>

typedef __int64 fttime_t;

#define FT_FILENAME_MAX	FILENAME_MAX

#define ft_gets_s(a,b) gets_s(a,b)
#define ft_strncpy_s strncpy_s
#define ft_sscanf_s sscanf_s
#define ft_sprintf_s sprintf_s
#define ft_vsnprintf_s _vsnprintf_s
#define ft_strcpy_s(strDestination, sizeInBytes, strSource)  strcpy_s(strDestination, sizeInBytes, strSource)
#define ft_strdup(str) _strdup(str)
#define ft_strnicmp(str1, str2, count) _strnicmp(str1, str2, count)

#define ft_localtime_s(a,b) localtime_s(a,b)
#define ft_gmtime_s(a,b) gmtime_s(a,b)

#define ft_fseek(a,b,c) _fseeki64(a,b,c)
#define ft_access(a,b) _access(a,b)
#define FT_ACCESS_F_OK 0
#define FT_ACCESS_R_OK 2
#define FT_ACCESS_W_OK 4

#define ft_atoll(a) _strtoi64(a,NULL,10)
#define ft_atoull(a) _strtoui64(a,NULL,10)
////////////////////////////////////////////////////
// Solaris 32-bit                                 //
////////////////////////////////////////////////////
#elif defined(__sun) && defined(__i386)
#define FT_SOLARIS

#include <sys/time.h>
#include <signal.h>

typedef hrtime_t fttime_t;

#define FT_FILENAME_MAX	FILENAME_MAX

#define ft_gets_s(a,b) gets(a)
#define ft_strncpy_s(a,b,c,d) strncpy(a,c,d);
#define ft_sscanf_s sscanf
#define ft_sprintf_s snprintf
#define ft_vsnprintf_s vsnprintf
#define ft_strcpy_s(strDestination, sizeInBytes, strSource)  strlcpy(strDestination, strSource, sizeInBytes)
#define ft_strdup(str) strdup(str)
#define ft_strnicmp(str1, str2, count) _strncasecmp(str1, str2, count)

#define ft_localtime_s(a,b) localtime_r(b,a)
#define ft_gmtime_s(a,b) gmtime_r(b,a)
////////////////////////////////////////////////////
// Solaris 64-bit                                 //
////////////////////////////////////////////////////
#elif defined(__sun) && defined(__x86_64)
#define FT_SOLARIS

#include <sys/time.h>
#include <signal.h>

typedef hrtime_t fttime_t;

#define FT_FILENAME_MAX	FILENAME_MAX

#define ft_gets_s(a,b) gets(a)
#define ft_strncpy_s(a,b,c,d) strncpy(a,c,d);
#define ft_sscanf_s sscanf
#define ft_sprintf_s snprintf
#define ft_vsnprintf_s vsnprintf
#define ft_strcpy_s(strDestination, sizeInBytes, strSource)  strlcpy(strDestination, strSource, sizeInBytes)
#define ft_strdup(str) strdup(str)
#define ft_strnicmp(str1, str2, count) _strncasecmp(str1, str2, count)

#define ft_localtime_s(a,b) localtime_r(b,a)
#define ft_gmtime_s(a,b) gmtime_r(b,a)

#else
#error "unrecognized operating environment"
#endif

#include "fttypes.h"

#include <stdlib.h>
#include <limits.h>

// stl
#include <string>
#include <list>
#include <map>
#include <vector>
#include <set>
#include <iostream>
#include <fstream>
#include <sstream>
#ifndef FT_GCC
#include <strstream>
#endif
#include <iomanip>
#include <cmath>
using namespace std;

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////


#endif // #define __ftbase_h_included
