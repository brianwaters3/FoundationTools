/////////////////////////////////////////////////////////////////////////
// (c) 2009 Brian Waters (bdwaters@sbcglobal.net) All rights reserved. //
/////////////////////////////////////////////////////////////////////////

#include "../include/ft/ftutility.h"

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

Int FTUtility::indexOf(cpStr path, Char search, Int start)
{
	if (start > (Int)strlen(path))
		return -1;

	path = &path[start];
	int ofs = start;
	for (; *path && *path != search; ofs++, path++);

	return *path ? -1 : ofs;
}

Int FTUtility::indexOfAny(cpStr path, cpStr search)
{
	Int ofs = 0;

	for (cpStr p = path; *p; ofs++, p++)
	{
		cpStr sp = search;
		for (; *sp && *p != *sp; ++sp);
		if (*sp)
			break;
	}

	return path[ofs] ? ofs : -1;
}

Int FTUtility::lastIndexOfAny(cpStr path, cpStr search)
{
	if (!*path)
		return -1;

	cpStr p = &path[strlen(path)-1];

	for (; p >= path; p--)
	{
		cpStr sp = search;
		for (; *sp && *p != *sp; ++sp);
		if (*sp)
			break;
	}

	return p < path ? -1 : (Int)(p - path);
}
