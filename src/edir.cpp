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

#include "ebase.h"
#include "estring.h"
#include "eerror.h"
#include "edir.h"

#include <unistd.h>
#define GetCurrentDir getcwd

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

/// @cond DOXYGEN_EXCLUDE

EDirectoryError_GetFirstEntry::EDirectoryError_GetFirstEntry()
{
   setSevere();
   setTextf("Error getting the first directory entry - ");
   appendLastOsError();
}

EDirectoryError_OutOfSequence::EDirectoryError_OutOfSequence()
{
   setSevere();
   setTextf("Error GetNextEntry() was called before a successful call to GetFirstEntry()");
}

EDirectoryError_CurrentDirectory::EDirectoryError_CurrentDirectory()
{
   setSevere();
   setTextf("Error getting the current directory - ");
   appendLastOsError();
}

EDirectoryError_GetNextEntry::EDirectoryError_GetNextEntry(Int err)
{
   setSevere();
   setTextf("Error getting the next directory entry - ");
   appendLastOsError((Dword)err);
}

/// @endcond

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

EDirectory::EDirectory()
{
   mHandle = NULL;
}

EDirectory::~EDirectory()
{
   closeHandle();
}

cpStr EDirectory::getFirstEntry(cpStr pDirectory, cpStr pFileMask)
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

   closeHandle();
   mHandle = opendir(mDirectory.c_str());
   if (!mHandle)
      throw EDirectoryError_GetFirstEntry();
   try
   {
      getNextEntry();
   }
   catch (EDirectoryError_GetNextEntry &e)
   {
      throw EDirectoryError_GetFirstEntry();
   }
   catch (...)
   {
      throw EDirectoryError_GetFirstEntry();
   }

   return mFileName.length() == 0 ? NULL : mFileName.c_str();
}

cpStr EDirectory::getNextEntry()
{
   struct dirent de;
   struct dirent *pde;
   cpStr pMask = mFileMask.c_str();

   while (True)
   {
      Int result = readdir_r(mHandle, &de, &pde);
      if (result)
         throw EDirectoryError_GetNextEntry(result);
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
   return mFileName.length() == 0 ? NULL : mFileName.c_str();
}

Void EDirectory::getCurrentDirectory(EString &dir)
{
   Char cwd[FILENAME_MAX];

   if (!GetCurrentDir(cwd, sizeof(cwd)))
      throw EDirectoryError_CurrentDirectory();

   dir = cwd;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

Void EDirectory::closeHandle()
{
   if (mHandle != NULL)
   {
      closedir(mHandle);
      mHandle = NULL;
   }
}

#define MATCH(a, b, ignoreCase) (ignoreCase ? mTable[(Int)a] == mTable[(Int)b] : a == b)

pStr EDirectory::mTable = NULL;

Void EDirectory::buildTable()
{
   mTable = new Char[256];
   for (Int i = 0; i < 256; i++)
      mTable[i] = (i >= 'a' && i <= 'z') ? 'A' + i - 'a' : (Char)i;
}

Bool EDirectory::match(cpStr str, cpStr mask, Bool ignoreCase)
{
   if (!mTable)
      buildTable();

   cpStr mp = NULL;
   cpStr sp = NULL;

   while (*str && *mask != '*')
   {
      if (*mask != '?' && !MATCH(*str, *mask, ignoreCase))
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
      else if (*mask == '?' || MATCH(*str, *mask, ignoreCase))
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
