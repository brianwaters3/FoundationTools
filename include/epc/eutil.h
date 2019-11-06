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

/// @file
/// @brief Defines various utility functions.

#include <unistd.h>
#include <vector>

#include "ebase.h"
#include "estring.h"

/// @brief Class that contains various utility functions.
class EUtility
{
public:
   /// @brief Retrieves the byte index of the search value within the supplied path.
   /// @param path the string to search.
   /// @param search the character to search for.
   /// @param start the byte offset in path where to start the search.
   /// @return the index where the value was located.
   static Int indexOf(cpStr path, Char search, Int start = 0);
   /// @brief Retrieves the byte index of any of the search values within the supplied path.
   /// @param path the string to search.
   /// @param search the characters to search for.
   /// @return the index where the value was located.
   static Int indexOfAny(cpStr path, cpStr search);
   /// @brief Retrieves the last byte index of any of the search values within the supplied path.
   /// @param path the string to search.
   /// @param search the characters to search for.
   /// @return the index where the value was located.
   static Int lastIndexOfAny(cpStr path, cpStr search);
   /// @brief Splits a string into substrings that are based on the characters in the delimiter array.
   /// @param s the string to split.
   /// @param delims the list of delimiting characters.
   /// @return a vector (array) of the resulting strings.
   static std::vector<EString> split(cpStr s, cpStr delims);
   /// @brief replaces all occurances of the search string in the provided string with the replacement
   ///   string.
   /// @param str the string to make the replacements in.
   /// @param srch the search string.
   /// @param srchlen the length of the search string.
   /// @param rplc the replacement string.
   /// @param rplclen the length of the replacement string.
   /// @return a reference to the updated string.
   static EString &replaceAll(EString &str, cpStr srch, size_t srchlen, cpStr rplc, size_t rplclen);
   /// @brief replaces all occurances of the search string in the provided string with the replacement
   ///   string returning a new string.
   /// @param str the string to make the replacements in.
   /// @param srch the search string.
   /// @param srchlen the length of the search string.
   /// @param rplc the replacement string.
   /// @param rplclen the length of the replacement string.
   /// @return the updated string.
   static EString replaceAllCopy(const EString &str, cpStr srch, size_t srchlen, cpStr rplc, size_t rplclen);

   /// @brief Formats a string using the supplied format string.  Uses printf format specifiers.
   /// @param format the format specification string.
   /// @return the new formatted string.
   static std::string string_format( const char *format, ... );
   /// @brief Formats a string using the supplied format string.  Uses printf format specifiers.
   /// @param dest a string reference where the formatted string will be output.
   /// @param format the format specification string.
   static void string_format( std::string &dest, const char *format, ... );

   /// @brief Copies the source file to the destination location.
   /// @param dst the destination file name.
   /// @param src the source file name.
   static Void copy_file( const std::string &dst, const std::string &src ) { copy_file( dst.c_str(), src.c_str() ); }
   /// @copydoc copy_file(const std::string &, const std::string &)
   static Void copy_file( const char *dst, const std::string &src )        { copy_file( dst,         src.c_str() ); }
   /// @copydoc copy_file(const std::string &, const std::string &)
   static Void copy_file( const std::string &dst, const char *src )        { copy_file( dst.c_str(), src ); }
   /// @copydoc copy_file(const std::string &, const std::string &)
   static Void copy_file( const char *dst, const char *src );

   /// @brief Deletes the supplied file.
   /// @param fn the file name to delete.
   static Void delete_file( const std::string &fn ) { delete_file( fn.c_str() ); }
   /// @copydoc delete_file(const std::string &)
   static Void delete_file( const char *fn );

   /// @brief Determines if the supplied file exists.
   /// @return True if the file exists, otherwise False.
   static Bool file_exists( cpStr fn ) { return (access(fn,F_OK)!=-1); }

   /// @brief Returns a string representation of the current system time.
   /// @return a string representation of the current system time.
   static std::string currentTime();

private:
   static void _string_format( std::string &dest, const char *format, va_list &args );
};

#endif // #define __eutil_h_included