////////////////////////////////////////////////////
// (c) 2009 Brian Waters (bdwaters@sbcglobal.net) //
////////////////////////////////////////////////////
#ifndef __fttypes_h_included
#define __fttypes_h_included

// basic data types
#if defined(FT_WINDOWS)
#include <stdint.h>
typedef void Void;
typedef bool Bool;
typedef char Char;
typedef unsigned char UChar;
typedef UChar Byte;
typedef short Short;
typedef unsigned short UShort;
typedef int Int;
typedef unsigned int UInt;
typedef long Long;
typedef unsigned long ULong;
typedef DWORD Dword;
typedef long long LongLong;
typedef unsigned long long ULongLong;
typedef float Float;
typedef double Double;
#elif defined(FT_GCC)
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
#elif defined(FT_SOLARIS)
#error "Need to define"
#else
#error "Unrecognized platform"
#endif

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

typedef union
{
    struct
    {
        Dword lowPart;
        Long highPart;
    } li;
    LongLong quadPart;
} longinteger_t;

typedef union
{
    struct
    {
        Dword lowPart;
        Dword highPart;
    } uli;
    ULongLong quadPart;
} ulonginteger_t;

#endif // #define __fttypes_h_included
