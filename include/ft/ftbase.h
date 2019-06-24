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
#include <iomanip>
#include <cmath>
using namespace std;

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////


#endif // #define __ftbase_h_included
