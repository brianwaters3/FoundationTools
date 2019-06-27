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

#include <unistd.h>
#define GetCurrentDir getcwd

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

FTDirectoryError_GetNextEntry::FTDirectoryError_GetNextEntry(Int err)
{
   setSevere();
   setTextf("Error getting the next directory entry - ");
   appendLastOsError((Dword)err);
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

FTDirectory::FTDirectory()
{
   mHandle = NULL;
}

FTDirectory::~FTDirectory()
{
   closeHandle();
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

   closeHandle();
   mHandle = opendir(mDirectory.c_str());
   if (!mHandle)
      throw FTDirectoryError_GetFirstEntry();
   try
   {
      getNextEntry();
   }
   catch (FTDirectoryError_GetNextEntry &e)
   {
      throw FTDirectoryError_GetFirstEntry();
   }
   catch (...)
   {
      throw FTDirectoryError_GetFirstEntry();
   }

   return mFileName.length() == 0 ? NULL : mFileName.c_str();
}

cpStr FTDirectory::getNextEntry()
{
   struct dirent de;
   struct dirent *pde;
   cpStr pMask = mFileMask.c_str();

   while (True)
   {
      Int result = readdir_r(mHandle, &de, &pde);
      if (result)
         throw FTDirectoryError_GetNextEntry(result);
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

Void FTDirectory::getCurrentDirectory(FTString &dir)
{
   Char cwd[FILENAME_MAX];

   if (!GetCurrentDir(cwd, sizeof(cwd)))
      throw FTDirectoryError_CurrentDirectory();

   dir = cwd;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

Void FTDirectory::closeHandle()
{
   if (mHandle != NULL)
   {
      closedir(mHandle);
      mHandle = NULL;
   }
}

#define MATCH(a, b, ignoreCase) (ignoreCase ? mTable[(Int)a] == mTable[(Int)b] : a == b)

pStr FTDirectory::mTable = NULL;

Void FTDirectory::buildTable()
{
   mTable = new Char[256];
   for (Int i = 0; i < 256; i++)
      mTable[i] = (i >= 'a' && i <= 'z') ? 'A' + i - 'a' : (Char)i;
}

Bool FTDirectory::match(cpStr str, cpStr mask, Bool ignoreCase)
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
