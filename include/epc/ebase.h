/*
* Copyright (c) 2009-2019 Brian Waters
* Copyright (c) 2019 Sprint
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

#ifndef __ebase_h_included
#define __ebase_h_included

/// @file
/// @brief Macros for various standard C library functions and standard includes.

/// True
#define True	true
/// False
#define False	false

#include <sys/time.h>
#include <signal.h>
#include <string.h>

/// time typedef
typedef long long int epctime_t;

/// maximum file name length
#define EPC_FILENAME_MAX	FILENAME_MAX

/// epc_gets_s - gets_s
#define epc_gets_s(a,b) __builtin_gets(a)
/// epc_strncpy_s - strncpy_s
#define epc_strncpy_s(a,b,c,d) __builtin_strncpy(a,c,d);
/// epc_sscanf_s - sscanf_s
#define epc_sscanf_s __builtin_sscanf
/// epc_sprintf_s - sprintf_s
#define epc_sprintf_s __builtin_snprintf
/// epc_vsnprintf_s - vsnprintf_s
#define epc_vsnprintf_s __builtin_vsnprintf
/// epc_strcpy_s - strncpy
#define epc_strcpy_s(strDestination, sizeInBytes, strSource)  __builtin_strncpy(strDestination, strSource, sizeInBytes)
/// epc_strdup - strdup
#define epc_strdup(str) __builtin_strdup(str)
/// strnicmp - strnicmp, strncasecmp
#define epc_strnicmp(str1, str2, count) __builtin_strncasecmp(str1, str2, count)

/// epc_localtime_s - localtime_r
#define epc_localtime_s(a,b) localtime_r(b,a)
/// epc_gmtime_s - gmtime_r
#define epc_gmtime_s(a,b) gmtime_r(b,a)

/// epc_fseek - fseek
#define epc_fseek(a,b,c) fseek(a,b,c)
/// epc_access - access
#define epc_access(a,b) access(a,b)
/// EACCESS_F_OK - F_OK
#define EACCESS_F_OK F_OK
/// EACCESS_R_OK - R_OK
#define EACCESS_R_OK R_OK
/// EACCESS_W_OK - W_OK
#define EACCESS_W_OK W_OK

#include "etypes.h"

#include <stdlib.h>
#include <limits.h>

// stl
#include <cmath>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <list>
#include <map>
#include <memory>
#include <set>
#include <sstream>
#include <string>
#include <vector>
using namespace std;

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////


#endif // #define __ebase_h_included
