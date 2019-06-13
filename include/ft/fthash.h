////////////////////////////////////////////////////
// (c) 2009 Brian Waters (bdwaters@sbcglobal.net) //
////////////////////////////////////////////////////
#ifndef __fthash_h_included
#define __fthash_h_included

#include "ftbase.h"
#include "ftstring.h"

class FTHash
{
public:
	static ULong getHash(FTString& str);
	static ULong getHash(cpChar val, ULong len);
	static ULong getHash(cpUChar val, ULong len);
protected:
private:
	static ULong m_crcTable[256];
};

#endif // #define __fthash_h_included
