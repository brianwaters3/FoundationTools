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

#ifndef __ftdir_h_included
#define __ftdir_h_included

#include <dirent.h>

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

DECLARE_ERROR_ADVANCED(FTDirectoryError_GetFirstEntry);
DECLARE_ERROR_ADVANCED(FTDirectoryError_OutOfSequence);
DECLARE_ERROR_ADVANCED(FTDirectoryError_CurrentDirectory);

DECLARE_ERROR_ADVANCED2(FTDirectoryError_GetNextEntry);

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

#define PATH_SEPERATOR '/'
	static pStr mTable;
	static Void buildTable();
	static Bool match(cpStr str, cpStr mask, Bool ignoreCase = False);
	DIR* mHandle;
};

#endif // #define __ftdir_h_included
