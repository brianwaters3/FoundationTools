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

#ifndef __estring_h_included
#define __estring_h_included

#include <string>
#include <algorithm>

#include "ebase.h"

class EString : public std::string
{
public:
   EString() {}
   EString(cpStr s) : std::string(s) {}
   EString(const std::string &s) : std::string(s) {}
   EString(const EString &s) : std::string(s) {}
   EString &format(cpChar pszFormat, ...);
   EString &tolower();
   EString &toupper();

   operator cpChar() { return c_str(); }

   EString &operator=(cpChar s)
   {
      *(std::string *)this = s;
      return *this;
   }
   EString &operator=(const std::string s)
   {
      *(std::string *)this = s;
      return *this;
   }
   Int icompare(EString &str)
   {
      return epc_strnicmp(c_str(), str.c_str(), length() > str.length() ? length() : str.length());
   }
   Int icompare(cpStr str)
   {
      size_t len = strlen(str);
      return epc_strnicmp(c_str(), str, length() > len ? length() : len);
   }

   Void ltrim()
   {
      erase(begin(), std::find_if(begin(), end(), [](Char ch) {
               return !std::isspace(ch);
            }));
   }

   Void rtrim()
   {
      erase(std::find_if(rbegin(), rend(), [](Char ch) {
               return !std::isspace(ch);
            })
                .base(),
            end());
   }

   Void trim()
   {
      ltrim();
      rtrim();
   }

   EString &replaceAll(cpStr srch, size_t srchlen, cpStr rplc, size_t rplclen);
   EString replaceAllCopy(cpStr srch, size_t srchlen, cpStr rplc, size_t rplclen);
};

#endif // #define __estring_h_included
