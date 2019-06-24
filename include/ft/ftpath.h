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

   static Void changeExtension(cpStr path, cpStr extension, FTString &newPath);

   static Void combine(cpStr path1, cpStr path2, cpStr path3, cpStr path4, FTString &path);
   static Void combine(cpStr path1, cpStr path2, cpStr path3, FTString &path);
   static Void combine(cpStr path1, cpStr path2, FTString &path);

   static Void getDirectoryName(cpStr path, FTString &dirName);
   static Void getExtension(cpStr path, FTString &ext);
   static Void getFileName(cpStr path, FTString &fileName);
   static Void getFileNameWithoutExtension(cpStr path, FTString &fileName);
   static Void getPathRoot(cpStr path, FTString &root);

private:
   static Bool m_dirEqualsVolume;

   static Void cleanPath(cpStr path, FTString &cleanPath);
   static Void insecureFullPath(cpStr path, FTString &fullPath);
   static Bool isDsc(cChar c);
   static Bool isPathRooted(cpStr path);
   static Int findExtension(cpStr path);
};

inline cpStr FTPath::getDirectorySeparatorString()
{
   return "/";
}

inline cChar FTPath::getDirectorySeparatorChar()
{
   return '/';
}

inline cChar FTPath::getAltDirectorySeparatorChar()
{
   return '/';
}

inline cChar FTPath::getVolumeSeparatorChar()
{
   return ':';
}

inline cpStr FTPath::getPathSeparatorChars()
{
   return "/:";
}

inline cpStr FTPath::getInvalidPathChars()
{
   return "";
}

inline cpStr FTPath::getInvalidFileNameChars()
{
   return "/";
}

inline Bool FTPath::isDsc(cChar c)
{
   return c == getDirectorySeparatorChar() || c == getAltDirectorySeparatorChar();
}

#endif // #define __ftpath_h_included
