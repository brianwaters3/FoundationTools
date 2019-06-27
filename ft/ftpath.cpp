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

#include "ftpath.h"
#include "ftdir.h"
#include "ftutility.h"

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

FTPathError_ArgumentException::FTPathError_ArgumentException(cpStr arg)
{
   setSevere();
   setTextf("Invalid argument - %s", arg);
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

Void FTPath::changeExtension(cpStr path, cpStr extension, FTString &newPath)
{
   if (!path || !*path)
   {
      newPath = "";
      return;
   }

   if (FTUtility::indexOfAny(path, getInvalidPathChars()) != -1)
      throw FTPathError_ArgumentException("Illegal characters in path.");

   Int iExt = findExtension(path);

   if (!extension || !*extension)
   {
      if (iExt < 0)
         newPath = path;
      else
         newPath.assign(path, &path[iExt]);
   }
   else if (iExt < 0)
   {
      newPath.assign(path, &path[iExt]);
      if (*extension != '.')
         newPath += ".";
      newPath += extension;
   }
   else if (iExt > 0)
   {
      newPath.assign(path, &path[iExt + 1]);
      newPath += (*extension == '.') ? &extension[1] : extension;
   }
   else
   {
      newPath = extension;
   }
}

Void FTPath::combine(cpStr path1, cpStr path2, FTString &path)
{
   if (!path1)
      throw FTPathError_ArgumentException("path1");
   else if (!path2)
      throw FTPathError_ArgumentException("path2");
   if (*path1 == '\0')
      path = path2;
   else if (*path2 == '\0')
      path = path1;
   else if (FTUtility::indexOfAny(path1, getInvalidPathChars()) != -1)
      throw FTPathError_ArgumentException("Illegal characters in path1.");
   else if (FTUtility::indexOfAny(path2, getInvalidPathChars()) != -1)
      throw FTPathError_ArgumentException("Illegal characters in path2.");

   if (isPathRooted(path2))
   {
      path = path2;
      return;
   }

   Char p1end = path1[strlen(path1) - 1];
   path = path1;
   if (p1end != getDirectorySeparatorChar() && p1end != getAltDirectorySeparatorChar() && p1end != getVolumeSeparatorChar())
      path.append(getDirectorySeparatorString());
   path.append(path2);
}

Void FTPath::combine(cpStr path1, cpStr path2, cpStr path3, FTString &path)
{
   combine(path1, path2, path);
   combine(path, path3, path);
}

Void FTPath::combine(cpStr path1, cpStr path2, cpStr path3, cpStr path4, FTString &path)
{
   combine(path1, path2, path3, path);
   combine(path, path4, path);
}

Void FTPath::getDirectoryName(cpStr path, FTString &dirName)
{
   if (!path || !*path)
      throw FTPathError_ArgumentException("Invalid path");

   if (FTUtility::indexOfAny(path, getInvalidPathChars()) != -1)
      throw FTPathError_ArgumentException("Path contains invalid characters");

   Int nLast = FTUtility::lastIndexOfAny(path, getPathSeparatorChars());
   if (nLast == 0)
      nLast++;

   if (nLast > 0)
   {
      dirName.assign(path, nLast);

      cleanPath(dirName, dirName);
   }
   else
   {
      dirName = "";
   }
}

Void FTPath::getExtension(cpStr path, FTString &ext)
{
   if (!path || !*path)
   {
      ext = "";
   }
   else
   {
      if (FTUtility::indexOfAny(path, getInvalidPathChars()) != -1)
         throw FTPathError_ArgumentException("Illegal characters in path.");

      Int iExt = findExtension(path);

      if (iExt > -1)
      {
         if (iExt < ((Int)strlen(path)) - 1)
            ext = &path[iExt];
         else
            ext = "";
      }
      else
      {
         ext = "";
      }
   }
}

Void FTPath::getFileName(cpStr path, FTString &fileName)
{
   if (!path || !*path)
   {
      fileName = "";
      return;
   }

   if (FTUtility::indexOfAny(path, getInvalidPathChars()) != -1)
      throw FTPathError_ArgumentException("Illegal characters in path.");

   Int nLast = FTUtility::lastIndexOfAny(path, getPathSeparatorChars());
   if (nLast >= 0)
      fileName = &path[nLast + 1];
   else
      fileName = path;
}

Void FTPath::getFileNameWithoutExtension(cpStr path, FTString &fileName)
{
   getFileName(path, fileName);
   changeExtension(fileName, NULL, fileName);
}

Void FTPath::getPathRoot(cpStr path, FTString &root)
{
   if (!*path || !*path)
      throw FTPathError_ArgumentException("The specified path is not of a legal form.");

   if (isPathRooted(path))
   {
      root = "";
   }
   else
   {
      root = isDsc(*path) ? getDirectorySeparatorString() : "";
   }
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

Void FTPath::cleanPath(cpStr path, FTString &dirName)
{
   Int l = (Int)strlen(path);
   Int sub = 0;
   Int start = 0;

   Char p0 = path[0];
   if (l > 2 && p0 == '\\' && path[1] == '\\')
      start = 2;

   if (l == 1 && (p0 == getDirectorySeparatorChar() || p0 == getAltDirectorySeparatorChar()))
   {
      dirName = path;
   }
   else
   {
      for (int i = start; i < l; i++)
      {
         Char c = path[i];
         if (c != getDirectorySeparatorChar() && c != getAltDirectorySeparatorChar())
            continue;
         if (i + 1 == l)
            sub++;
         else
         {
            c = path[i + 1];
            if (c == getDirectorySeparatorChar() || c == getAltDirectorySeparatorChar())
               sub++;
         }
      }

      if (!sub)
      {
         dirName = path;
      }
      else
      {
         Char copy[FILENAME_MAX];
         Int len = l - sub;
         if (start != 0)
         {
            copy[0] = '\\';
            copy[1] = '\\';
         }

         for (Int i = start, j = start; i < l && j < len; i++)
         {
            Char c = path[i];

            if (c != getDirectorySeparatorChar() && c != getAltDirectorySeparatorChar())
            {
               copy[j++] = c;
               continue;
            }

            // for non-trailing cases
            if (j + 1 != len)
            {
               copy[j++] = getDirectorySeparatorChar();
               for (; i < l - i; i++)
               {
                  c = path[i + 1];
                  if (c != getDirectorySeparatorChar() && c != getAltDirectorySeparatorChar())
                     break;
               }
            }
         }

         copy[len] = '\0';
         dirName = copy;
      }
   }
}

Bool FTPath::isPathRooted(cpStr path)
{
   if (!path || !*path)
      return False;

   if (FTUtility::indexOfAny(path, getInvalidPathChars()) != -1)
      throw FTPathError_ArgumentException("Illegal characters in path.");

   Int len = (Int)strlen(path);
   Char c = *path;
   return (c == getDirectorySeparatorChar() ||
           c == getAltDirectorySeparatorChar() ||
           (getDirectorySeparatorChar() == getVolumeSeparatorChar() && len > 1 && path[1] == getVolumeSeparatorChar()));
}

Int FTPath::findExtension(cpStr path)
{
   // method should return the index of the path extension
   // start or -1 if no valid extension
   Int iLastDot = FTUtility::lastIndexOfAny(path, ".");
   Int iLastSep = FTUtility::lastIndexOfAny(path, getPathSeparatorChars());

   if (iLastDot > iLastSep)
      return iLastDot;

   return -1;
}
