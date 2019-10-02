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

#ifndef __eutil_h_included
#define __eutil_h_included

#include <vector>

#include "ebase.h"
#include "estring.h"

class EUtility
{
public:
   static Int indexOf(cpStr path, Char search, Int start = 0);
   static Int indexOfAny(cpStr path, cpStr search);
   static Int lastIndexOfAny(cpStr path, cpStr search);
   static std::vector<EString> split(cpStr s, cpStr delims);
   static EString &replaceAll(EString &str, cpStr srch, size_t srchlen, cpStr rplc, size_t rplclen);
   static EString replaceAllCopy(const EString &str, cpStr srch, size_t srchlen, cpStr rplc, size_t rplclen);

   static std::string string_format( const char *format, ... );
   static void string_format( std::string &dest, const char *format, ... );

   static void copyfile( const std::string &dst, const std::string &src ) { copyfile( dst.c_str(), src.c_str() ); }
   static void copyfile( const char *dst, const std::string &src )        { copyfile( dst,         src.c_str() ); }
   static void copyfile( const std::string &dst, const char *src )        { copyfile( dst.c_str(), src ); }
   static void copyfile( const char *dst, const char *src );

   static void deletefile( const std::string &fn ) { deletefile( fn.c_str() ); }
   static void deletefile( const char *fn );

   static std::string currentTime();

private:
   static void _string_format( std::string &dest, const char *format, va_list &args );
};

#endif // #define __eutil_h_included