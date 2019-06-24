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

#ifndef __ftstring_h_included
#define __ftstring_h_included

#include <algorithm>

class FTString : public std::string
{
public:
   FTString() {}
   FTString(cpStr s) : std::string(s) {}
   FTString &format(cpChar pszFormat, ...);
   FTString &tolower();
   FTString &toupper();

   operator cpChar() { return c_str(); }
   FTString &operator=(cpChar s)
   {
      *(std::string *)this = s;
      return *this;
   }
   FTString &operator=(const std::string s)
   {
      *(std::string *)this = s;
      return *this;
   }
   Int icompare(FTString &str)
   {
      return ft_strnicmp(c_str(), str.c_str(), length() > str.length() ? length() : str.length());
   }
   Int icompare(cpStr str)
   {
      size_t len = strlen(str);
      return ft_strnicmp(c_str(), str, length() > len ? length() : len);
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
};

#endif // #define __ftstring_h_included
