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

/// @file
/// @brief Encapsulates and extends a std::string object.

#include <string>
#include <algorithm>

#include "ebase.h"

/// @brief String class.
class EString : public std::string
{
public:
   /// @brief Default constructor.
   EString() {}
   /// @brief Class constructor.
   /// @param s the NULL terminated string to initilize this string object to.
   EString(cpStr s) : std::string(s) {}
   /// @brief Copy constructor.
   /// @param s the object to copy.
   EString(const std::string &s) : std::string(s) {}
   /// @brief Copy constructor.
   /// @param s the object to copy.
   EString(const EString &s) : std::string(s) {}
   /// @brief Sets the value to the string using a "printf" style format string and arguments.
   /// @return reference to this string object.
   EString &format(cpChar pszFormat, ...);
   /// @brief Converts this string to lowercase.
   /// @return reference to this string object.
   EString &tolower();
   /// @brief Converts this string to uppercase.
   /// @return reference to this string object.
   EString &toupper();

   /// @brief Retrieves a const char* to this string.
   /// @return a const char* to this string.
   operator cpChar() { return c_str(); }

   /// @brief Assigns the specified NULL terminated strint to this string object.
   /// @return reference to this string object.
   EString &operator=(cpChar s)
   {
      *(std::string *)this = s;
      return *this;
   }
   /// @brief Assignment operator.
   /// @param s the string to assignment to this object.
   EString &operator=(const std::string s)
   {
      *(std::string *)this = s;
      return *this;
   }
   /// @brief Compares this string object to the specified string object.
   /// @return see "strnicmp" for the definition of return value.
   Int icompare(EString &str)
   {
      return epc_strnicmp(c_str(), str.c_str(), length() > str.length() ? length() : str.length());
   }
   /// @brief Compares this string object to the specified NULL terminated string.
   /// @return see "strnicmp" for the definition of return value.
   Int icompare(cpStr str)
   {
      size_t len = strlen(str);
      return epc_strnicmp(c_str(), str, length() > len ? length() : len);
   }

   /// @brief Removes leading white space.
   Void ltrim()
   {
      erase(begin(), std::find_if(begin(), end(), [](Char ch) {
               return !std::isspace(ch);
            }));
   }

   /// @brief Removes trailing white space.
   Void rtrim()
   {
      erase(std::find_if(rbegin(), rend(), [](Char ch) {
               return !std::isspace(ch);
            })
                .base(),
            end());
   }

   /// @brief Removes leading and trailing white space.
   Void trim()
   {
      ltrim();
      rtrim();
   }

   /// @brief Replaces all occurances of the search string with the replacement string.
   /// @param srch the search string.
   /// @param srchlen the length of the search string.
   /// @param rplc the replacement string.
   /// @param rplclen the length of the replacement string.
   /// @return a reference to this object.
   EString &replaceAll(cpStr srch, size_t srchlen, cpStr rplc, size_t rplclen);
   /// @brief Replaces all occurances of the search string with the replacement string and returns a new string object.
   /// @param srch the search string.
   /// @param srchlen the length of the search string.
   /// @param rplc the replacement string.
   /// @param rplclen the length of the replacement string.
   /// @return the new string object.
   EString replaceAllCopy(cpStr srch, size_t srchlen, cpStr rplc, size_t rplclen);
};

#endif // #define __estring_h_included
