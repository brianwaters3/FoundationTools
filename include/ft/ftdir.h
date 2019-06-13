////////////////////////////////////////////////////
// (c) 2009 Brian Waters (bdwaters@sbcglobal.net) //
////////////////////////////////////////////////////
#ifndef __ftdir_h_included
#define __ftdir_h_included

#if defined(FT_WINDOWS)
#elif defined(FT_GCC)
#include <dirent.h>
#elif defined(FT_SOLARIS)
#error "Unsupported platform"
#else
#error "Unrecoginzed platform"
#endif

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

DECLARE_ERROR_ADVANCED(FTDirectoryError_GetFirstEntry);
DECLARE_ERROR_ADVANCED(FTDirectoryError_OutOfSequence);
DECLARE_ERROR_ADVANCED(FTDirectoryError_CurrentDirectory);

#if defined(FT_WINDOWS)
DECLARE_ERROR_ADVANCED(FTDirectoryError_GetNextEntry);
#elif defined(FT_GCC)
DECLARE_ERROR_ADVANCED2(FTDirectoryError_GetNextEntry);
#elif defined(FT_SOLARIS)
#error "Unsupported platform"
#else
#error "Unrecoginzed platform"
#endif

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

class FTDirectory
{
public:
	FTDirectory();
	~FTDirectory();

	cpStr getFirstEntry(cpStr pDirectory, cpStr pFileMask);
	cpStr getNextEntry();

	static Void getCurrentDirectory(FTString& dir);

private:
	Void closeHandle();

	FTString mDirectory;
	FTString mFileMask;
	FTString mFileName;

#if defined(FT_WINDOWS)
#define PATH_SEPERATOR '\\'
	HANDLE mHandle;
#elif defined(FT_GCC)
#define PATH_SEPERATOR '/'
	static pStr mTable;
	static Void buildTable();
	static Bool match(cpStr str, cpStr mask, Bool ignoreCase = False);
	DIR* mHandle;
#elif defined(FT_SOLARIS)
#error "Unsupported platform"
#else
#error "Unrecoginzed platform"
#endif
};

#endif // #define __ftdir_h_included
