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

#ifndef __edir_h_included
#define __edir_h_included

/// @file
/// @brief Implements directory processing.

#include <dirent.h>

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

/// @cond DOXYGEN_EXCLUDE
DECLARE_ERROR_ADVANCED(EDirectoryError_GetFirstEntry);
DECLARE_ERROR_ADVANCED(EDirectoryError_OutOfSequence);
DECLARE_ERROR_ADVANCED(EDirectoryError_CurrentDirectory);
DECLARE_ERROR_ADVANCED2(EDirectoryError_GetNextEntry);
/// @endcond

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

/// @brief Retrieve file names in a directory.
class EDirectory
{
public:
   /// @brief Class constructor.
	EDirectory();
   /// @brief Class destructor.
	~EDirectory();

   /// @brief Gets the first entry from the directory.
   /// @param pDirectory The directory name to process.
   /// @param pFileMask The file pattern file names must match to be returned.
   /// @return a pointer to the file name
   /// @throws EDirectoryError_GetFirstEntry
   /// @details Returns the first file name from pDirectory that matches
   /// pFileMask.  pFileMask can contain any alphanumeric character as well as
   /// wildcard characters.  The supported wildcard characters are '*' which
   /// corresponds to any number of any character and '?' which corresonds to
   /// a single character.
	cpStr getFirstEntry(cpStr pDirectory, cpStr pFileMask);
   /// @brief Gets the next entry from the directory.
   /// @return a pointer to the file name
   /// @throws EDirectoryError_GetNextEntry
   /// @details
   /// Reads the next entry from the directory that matches the previously
   /// supplied file name mask.  This function will throw 
   /// EDirectoryError_GetNextEntry if it is called before calling 
   /// getFirstEntry().
	cpStr getNextEntry();

   /// @brief Returns the current working directory.
   /// @param dir A reference to an EString variable where the directory name
   /// will be stored.
   /// @throws EDirectoryError_CurrentDirectory
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
