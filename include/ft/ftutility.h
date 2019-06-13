////////////////////////////////////////////////////
// (c) 2009 Brian Waters (bdwaters@sbcglobal.net) //
////////////////////////////////////////////////////
#ifndef __ftutility_h_included
#define __ftutility_h_included

#include "ftbase.h"

class FTUtility
{
public:
	static Int indexOf(cpStr path, Char search, Int start = 0);
	static Int indexOfAny(cpStr path, cpStr search);
	static Int lastIndexOfAny(cpStr path, cpStr search);
};

#endif // #define __ftutility_h_included