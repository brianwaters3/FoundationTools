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

#ifndef __etypes_h_included
#define __etypes_h_included

/// @file
/// @brief Contains type definitions used by this library.
/// @cond DOXYGEN_EXCLUDE

// basic data types
#include <stdint.h>
typedef void Void;
typedef bool Bool;
typedef char Char;
typedef unsigned char UChar;
typedef UChar Byte;
typedef int16_t Short;
typedef uint16_t UShort;
typedef int32_t Int;
typedef uint32_t UInt;
typedef int32_t Long;
typedef uint32_t ULong;
typedef ULong Dword;
typedef int64_t LongLong;
typedef uint64_t ULongLong;
typedef float Float;
typedef double Double;

// constants
typedef const Bool cBool;
typedef const Char cChar;
typedef const UChar cUChar;
typedef const Short cShort;
typedef const UShort cUShort;
typedef const Int cInt;
typedef const UInt cUInt;
typedef const Long cLong;
typedef const ULong cULong;
typedef const Dword cDword;
typedef const LongLong cLongLong;
typedef const ULongLong cULongLong;
typedef const Float cFloat;
typedef const Double cDouble;

// basic pointers
typedef Void *pVoid;
typedef Bool *pBool;
typedef Char *pChar;
typedef pChar pStr;
typedef UChar *pUChar;
typedef Short *pShort;
typedef UShort *pUShort;
typedef Int *pInt;
typedef UInt *pUInt;
typedef Long *pLong;
typedef ULong *pULong;
typedef Dword *pDword;
typedef LongLong *pLongLong;
typedef ULongLong *pULongLong;
typedef Float *pFloat;
typedef Double *pDouble;

// const pointers
typedef const pVoid cpVoid;
typedef const pBool cpBool;
typedef const Char *cpChar;
typedef const Char *cpStr;
typedef const UChar *cpUChar;
typedef const pShort cpShort;
typedef const pUShort cpUShort;
typedef const pInt cpInt;
typedef const pUInt cpUInt;
typedef const pLong cpLong;
typedef const pULong cpULong;
typedef const pDword cpDword;
typedef const pLongLong cpLongLong;
typedef const pULongLong cpUlongLong;
typedef const pFloat cpFloat;
typedef const pDouble cpDouble;

typedef union {
   struct
   {
      Dword lowPart;
      Long highPart;
   } li;
   LongLong quadPart;
} longinteger_t;

typedef union {
   struct
   {
      Dword lowPart;
      Dword highPart;
   } uli;
   ULongLong quadPart;
} ulonginteger_t;

/// @endcond

#endif // #define __etypes_h_included
