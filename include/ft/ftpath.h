////////////////////////////////////////////////////
// (c) 2009 Brian Waters (bdwaters@sbcglobal.net) //
////////////////////////////////////////////////////
#ifndef __ftpath_h_included
#define __ftpath_h_included

#include "ftbase.h"
#include "ftstring.h"
#include "fterror.h"

DECLARE_ERROR_ADVANCED4(FTPathError_ArgumentException);

class FTPath
{
public:
	static cpStr getDirectorySeparatorString();
	static cChar getDirectorySeparatorChar();
	static cChar getAltDirectorySeparatorChar();
	static cChar getVolumeSeparatorChar();

	static cpStr getPathSeparatorChars();
	static cpStr getInvalidPathChars();
	static cpStr getInvalidFileNameChars();

	static Void changeExtension(cpStr path, cpStr extension, FTString& newPath);

	static Void combine(cpStr path1, cpStr path2, cpStr path3, cpStr path4, FTString& path);
	static Void combine(cpStr path1, cpStr path2, cpStr path3, FTString& path);
	static Void combine(cpStr path1, cpStr path2, FTString& path);

	static Void getDirectoryName(cpStr path, FTString& dirName);
	static Void getExtension(cpStr path, FTString& ext);
	static Void getFileName(cpStr path, FTString& fileName);
	static Void getFileNameWithoutExtension(cpStr path, FTString& fileName);
	static Void getPathRoot(cpStr path, FTString& root);

private:
	static Bool m_dirEqualsVolume;

	static Void cleanPath(cpStr path, FTString& cleanPath);
	static Void insecureFullPath(cpStr path, FTString& fullPath);
	static Bool isDsc(cChar c);
	static Bool isPathRooted(cpStr path);
	static Int findExtension(cpStr path);
};

inline cpStr FTPath::getDirectorySeparatorString()
{
#if defined(FT_WINDOWS)
	return "\\"; 
#elif defined(FT_GCC)
	return "/";
#elif defined(FT_SOLARIS)
#error "Unsupported platform"
#else
#error "Unrecoginzed platform"
#endif
}

inline cChar FTPath::getDirectorySeparatorChar()
{
#if defined(FT_WINDOWS)
	return '\\'; 
#elif defined(FT_GCC)
	return '/';
#elif defined(FT_SOLARIS)
#error "Unsupported platform"
#else
#error "Unrecoginzed platform"
#endif
}

inline cChar FTPath::getAltDirectorySeparatorChar()
{
#if defined(FT_WINDOWS)
	return '/'; 
#elif defined(FT_GCC)
	return '/';
#elif defined(FT_SOLARIS)
#error "Unsupported platform"
#else
#error "Unrecoginzed platform"
#endif
}

inline cChar FTPath::getVolumeSeparatorChar()
{
#if defined(FT_WINDOWS)
	return ':'; 
#elif defined(FT_GCC)
	return ':';
#elif defined(FT_SOLARIS)
#error "Unsupported platform"
#else
#error "Unrecoginzed platform"
#endif
}

inline cpStr FTPath::getPathSeparatorChars()
{
#if defined(FT_WINDOWS)
	return "\\/:";
#elif defined(FT_GCC)
	return "/:";
#elif defined(FT_SOLARIS)
#error "Unsupported platform"
#else
#error "Unrecoginzed platform"
#endif
}

inline cpStr FTPath::getInvalidPathChars()
{
#if defined(FT_WINDOWS)
	return "\x22\x3C\x3E\x7C\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0A\x0B\x0C\x0D\x0E"
		   "\x0F\x10\x11\x12\x13\x14\x15\x16\x17\x18\x19\x1A\x1B\x1C\x1D\x1E\x1F";
#elif defined(FT_GCC)
	return "";
#elif defined(FT_SOLARIS)
#error "Unsupported platform"
#else
#error "Unrecoginzed platform"
#endif
}

inline cpStr FTPath::getInvalidFileNameChars()
{
#if defined(FT_WINDOWS)
	return "\x00\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0A\x0B\x0C\x0D\x0E"
		   "\x0F\x10\x11\x12\x13\x14\x15\x16\x17\x18\x19\x1A\x1B\x1C\x1D"
		   "\x1E\x1F\x22\x3C\x3E\x7C:*?\\/"; 
#elif defined(FT_GCC)
	return "/";
#elif defined(FT_SOLARIS)
#error "Unsupported platform"
#else
#error "Unrecoginzed platform"
#endif
}


inline Bool FTPath::isDsc(cChar c)
{
	return c == getDirectorySeparatorChar() || c == getAltDirectorySeparatorChar();
}

#if defined(FT_WINDOWS)
#elif defined(FT_GCC)
#elif defined(FT_SOLARIS)
#error "Unsupported platform"
#else
#error "Unrecoginzed platform"
#endif

#endif // #define __ftpath_h_included
