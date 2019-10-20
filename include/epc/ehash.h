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

#ifndef __ehash_h_included
#define __ehash_h_included

#include "ebase.h"
#include "estring.h"

/// @file
/// @brief Hash calculation functions for strings and byte arrays.

/// @brief Calcuates a 32-bit hash value for the specified string or array of characters.
class EHash
{
public:
   /// @brief Returns the 32-bit has value for the specified string.
   /// @param str The string to calculate the hash for.
   /// @return 32-bit hash value
   static ULong getHash(EString &str);
   /// @brief Returns the 32-bit has value for the array of characters.
   /// @param val The array of characters to calculate the hash for.
   /// @param len The length of the array of characters.
   /// @return 32-bit hash value
   static ULong getHash(cpChar val, ULong len);
   /// @brief Returns the 32-bit has value for the array of unsigned characters.
   /// @param val The array of unsigned characters to calculate the hash for.
   /// @param len The length of the array of unsigned characters.
   /// @return 32-bit hash value
   static ULong getHash(cpUChar val, ULong len);

private:
   static ULong m_crcTable[256];
};

#endif // #define __ehash_h_included