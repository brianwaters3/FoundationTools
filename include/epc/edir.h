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

#ifndef __edir_h_included
#define __edir_h_included

#include <dirent.h>

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

DECLARE_ERROR_ADVANCED(EDirectoryError_GetFirstEntry);
DECLARE_ERROR_ADVANCED(EDirectoryError_OutOfSequence);
DECLARE_ERROR_ADVANCED(EDirectoryError_CurrentDirectory);

DECLARE_ERROR_ADVANCED2(EDirectoryError_GetNextEntry);

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

class EDirectory
{
public:
	EDirectory();
	~EDirectory();

	cpStr getFirstEntry(cpStr pDirectory, cpStr pFileMask);
	cpStr getNextEntry();

	static Void getCurrentDirectory(EString& dir);

private:
	Void closeHandle();

	EString mDirectory;
	EString mFileMask;
	EString mFileName;

#define PATH_SEPERATOR '/'
	static pStr mTable;
	static Void buildTable();
	static Bool match(cpStr str, cpStr mask, Bool ignoreCase = False);
	DIR* mHandle;
};

#endif // #define __edir_h_included
