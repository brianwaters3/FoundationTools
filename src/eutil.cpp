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

#include <cstdarg>

#include "eutil.h"

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

Int EUtility::indexOf(cpStr path, Char search, Int start)
{
   if (start > (Int)strlen(path))
      return -1;

   path = &path[start];
   int ofs = start;
   for (; *path && *path != search; ofs++, path++)
      ;

   return *path ? -1 : ofs;
}

Int EUtility::indexOfAny(cpStr path, cpStr search)
{
   Int ofs = 0;

   for (cpStr p = path; *p; ofs++, p++)
   {
      cpStr sp = search;
      for (; *sp && *p != *sp; ++sp)
         ;
      if (*sp)
         break;
   }

   return path[ofs] ? ofs : -1;
}

Int EUtility::lastIndexOfAny(cpStr path, cpStr search)
{
   if (!*path)
      return -1;

   cpStr p = &path[strlen(path) - 1];

   for (; p >= path; p--)
   {
      cpStr sp = search;
      for (; *sp && *p != *sp; ++sp)
         ;
      if (*sp)
         break;
   }

   return p < path ? -1 : (Int)(p - path);
}

std::vector<EString> EUtility::split(cpStr s, cpStr delims)
{
   std::vector<EString> strings;
   pStr ss = strdup(s);
   EString token;

   pStr p = strtok(ss, delims);
   while (p)
   {
      token = p;
      strings.push_back(token);
      p = strtok(NULL, delims);
   }

   free(ss);
   return strings;
}

EString &EUtility::replaceAll(EString &str, cpStr srch, size_t srchlen, cpStr rplc, size_t rplclen)
{
   size_t pos = 0;
   while ((pos=str.find(srch,0,srchlen)) != std::string::npos)
      str.replace(pos, srchlen, rplc, rplclen);
   return str;
}

EString EUtility::replaceAllCopy(const EString &str, cpStr srch, size_t srchlen, cpStr rplc, size_t rplclen)
{
   EString newstr = str;
   return replaceAll(newstr, srch, srchlen, rplc, rplclen);
}

void EUtility::copy_file( const char *dst, const char *src )
{
   std::ofstream fdst( dst, std::ios::binary);
   std::ifstream fsrc( src, std::ios::binary);

   fdst << fsrc.rdbuf();
}

void EUtility::delete_file( const char *fn )
{
   remove( fn );
}

void EUtility::_string_format( std::string &dest, const char *format, va_list &args )
{
   char buf[2048];
   int len = vsnprintf( buf, sizeof(buf), format, args  );
   dest.assign( buf, len < sizeof(buf) ? len : sizeof(buf) );
}

std::string EUtility::string_format( const char *format, ... )
{
   va_list args;
   std::string str;

   va_start( args, format );
   _string_format( str, format, args );
   va_end( args );

   return str;
}

void EUtility::string_format( std::string &dest, const char *format, ... )
{
   va_list args;

   va_start( args, format );
   _string_format( dest, format, args );
   va_end( args );
}

std::string EUtility::currentTime()
{
   time_t t = time( NULL );
   struct tm *now = localtime( &t );
   std::stringstream ss;

   ss << (now->tm_year + 1900) << '-'
      << (now->tm_mon + 1) << '-'
      << (now->tm_mday) << ' '
      <<  now->tm_hour << ':'
      <<  now->tm_min << ':'
      <<  now->tm_sec;

   return ss.str();
}
