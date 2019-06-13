/*
* Copyright (c) 2009-2019 Brian Waters
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

#include "ftbase.h"
#include "ftstring.h"
#include "fterror.h"
#include "ftdir.h"

#if defined(FT_WINDOWS)
#include <direct.h>
#define GetCurrentDir _getcwd
#elif defined(FT_GCC)
#include <unistd.h>
#define GetCurrentDir getcwd
#elif defined(FT_SOLARIS)
#error "Unsupported platform"
#else
#error "Unrecoginzed platform"
#endif

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

FTDirectoryError_GetFirstEntry::FTDirectoryError_GetFirstEntry()
{
    setSevere();
    setTextf("Error getting the first directory entry - ");
	appendLastOsError();
}

FTDirectoryError_OutOfSequence::FTDirectoryError_OutOfSequence()
{
    setSevere();
    setTextf("Error GetNextEntry() was called before a successful call to GetFirstEntry()");
}

FTDirectoryError_CurrentDirectory::FTDirectoryError_CurrentDirectory()
{
    setSevere();
    setTextf("Error getting the current directory - ");
	appendLastOsError();
}

#if defined(FT_WINDOWS)
FTDirectoryError_GetNextEntry::FTDirectoryError_GetNextEntry()
{
    setSevere();
    setTextf("Error getting the next directory entry - ");
	appendLastOsError();
}
#elif defined(FT_GCC)
FTDirectoryError_GetNextEntry::FTDirectoryError_GetNextEntry(Int err)
{
    setSevere();
    setTextf("Error getting the next directory entry - ");
	appendLastOsError((Dword)err);
}
#elif defined(FT_SOLARIS)
#error "Unsupported platform"
#else
#error "Unrecoginzed platform"
#endif

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

FTDirectory::FTDirectory()
{
#if defined(FT_WINDOWS)
	mHandle = NULL;
#elif defined(FT_GCC)
	mHandle = NULL;
#elif defined(FT_SOLARIS)
#error "Unsupported platform"
#else
#error "Unrecoginzed platform"
#endif
}

FTDirectory::~FTDirectory()
{
#if defined(FT_WINDOWS)
	closeHandle();
#elif defined(FT_GCC)
	closeHandle();
#elif defined(FT_SOLARIS)
#error "Unsupported platform"
#else
#error "Unrecoginzed platform"
#endif
}

cpStr FTDirectory::getFirstEntry(cpStr pDirectory, cpStr pFileMask)
{
	mDirectory = pDirectory;

	// remove any consecutive asterisks from the file mask
	cpStr pMask = pFileMask;
	cpStr pLastAsterisk = NULL;
	mFileMask = "";
	while (*pMask)
	{
		if (*pMask == '*')
		{
			if (!pLastAsterisk || (pLastAsterisk && pLastAsterisk != pMask - 1))
				mFileMask += *pMask;
			pLastAsterisk = pMask;
		}
		else
		{
			mFileMask += *pMask;
		}
		pMask++;
	}

#if defined(FT_WINDOWS)
	WIN32_FIND_DATA ffd;

	FTString s;
	s = (pDirectory == NULL || !*pDirectory) ? "." : pDirectory;
	if (s[s.length()-1] != PATH_SEPERATOR)
		s += PATH_SEPERATOR;
	s += mFileMask;

	closeHandle();

	mHandle = FindFirstFile(s.c_str(), &ffd);
	if (mHandle == INVALID_HANDLE_VALUE)
		throw new FTDirectoryError_GetFirstEntry();

	mFileName = ffd.cFileName;
#elif defined(FT_GCC)
	closeHandle();
	mHandle = opendir(mDirectory.c_str());
	if (!mHandle)
		throw new FTDirectoryError_GetFirstEntry();
	try
	{
		getNextEntry();
	}
	catch (FTDirectoryError_GetNextEntry* e)
	{
		FTDirectoryError_GetFirstEntry *ee = new FTDirectoryError_GetFirstEntry();
		delete e;
		throw ee;
	}
	catch (...)
	{
		throw new FTDirectoryError_GetFirstEntry();
	}
#elif defined(FT_SOLARIS)
#error "Unsupported platform"
#else
#error "Unrecoginzed platform"
#endif

	return mFileName.length() == 0 ? NULL : mFileName.c_str();
}

cpStr FTDirectory::getNextEntry()
{
#if defined(FT_WINDOWS)
	WIN32_FIND_DATA ffd;

	if (mHandle == NULL)
		throw new FTDirectoryError_OutOfSequence();

	if (FindNextFile(mHandle, &ffd))
	{
		mFileName = ffd.cFileName;
	}
	else
	{
		if (GetLastError() == ERROR_NO_MORE_FILES)
			mFileName = "";
		else
			throw new FTDirectoryError_GetNextEntry();
	}
#elif defined(FT_GCC)
	struct dirent de;
	struct dirent *pde;
	cpStr pMask = mFileMask.c_str();

	while (True)
	{
		Int result = readdir_r(mHandle, &de, &pde);
		if (result)
			throw new FTDirectoryError_GetNextEntry(result);
		if (!pde)
		{
			mFileName = "";
			break;
		}
		if (match(de.d_name, pMask))
		{
			mFileName = de.d_name;
			break;
		}
	}
#elif defined(FT_SOLARIS)
#error "Unsupported platform"
#else
#error "Unrecoginzed platform"
#endif
	return mFileName.length() == 0 ? NULL : mFileName.c_str();
}

Void FTDirectory::getCurrentDirectory(FTString& dir)
{
	Char cwd[FILENAME_MAX];

	if (!GetCurrentDir(cwd, sizeof(cwd)))
		throw new FTDirectoryError_CurrentDirectory();

	dir = cwd;
}


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

Void FTDirectory::closeHandle()
{
	if (mHandle != NULL)
	{
#if defined(FT_WINDOWS)
		FindClose(mHandle);
#elif defined(FT_GCC)
		closedir(mHandle);
#elif defined(FT_SOLARIS)
#error "Unsupported platform"
#else
#error "Unrecoginzed platform"
#endif
		mHandle = NULL;
	}
}

#if defined(FT_WINDOWS)
#elif defined(FT_GCC)
#define MATCH(a,b,ignoreCase) (ignoreCase?mTable[(Int)a]==mTable[(Int)b]:a==b)

pStr FTDirectory::mTable = NULL;

Void FTDirectory::buildTable()
{
	mTable = new Char[256];
	for (Int i=0; i<256; i++)
		mTable[i] = (i >= 'a' && i <= 'z') ? 'A' + i - 'a' : (Char)i;
}

Bool FTDirectory::match(cpStr str, cpStr mask, Bool ignoreCase)
{
	if (!mTable)
		buildTable();

	cpStr mp = NULL;
	cpStr sp = NULL;

	while(*str && *mask != '*')
	{
		if (*mask != '?' && !MATCH(*str,*mask,ignoreCase))
			return False;
		str++;
		mask++;
	}

	while (*str)
	{
		if (*mask == '*')
		{
			if (!*++mask)
				return True;
			mp = mask;
			sp = str + 1;
		}
		else if (*mask == '?' || MATCH(*str,*mask,ignoreCase))
		{
			str++;
			mask++;
		}
		else
		{
			mask = mp;
			str = sp++;
		}
	}

	while (*mask == '*')
		mask++;

	return !*mask;
}
#elif defined(FT_SOLARIS)
#error "Unsupported platform"
#else
#error "Unrecoginzed platform"
#endif
