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

#ifndef __epath_h_included
#define __epath_h_included

#include "ebase.h"
#include "estring.h"
#include "eerror.h"

DECLARE_ERROR_ADVANCED4(EPathError_ArgumentException);

class EPath
{
public:
   static cpStr getDirectorySeparatorString();
   static cChar getDirectorySeparatorChar();
   static cChar getAltDirectorySeparatorChar();
   static cChar getVolumeSeparatorChar();

   static cpStr getPathSeparatorChars();
   static cpStr getInvalidPathChars();
   static cpStr getInvalidFileNameChars();

   static Void changeExtension(cpStr path, cpStr extension, EString &newPath);

   static Void combine(cpStr path1, cpStr path2, cpStr path3, cpStr path4, EString &path);
   static Void combine(cpStr path1, cpStr path2, cpStr path3, EString &path);
   static Void combine(cpStr path1, cpStr path2, EString &path);
   static EString combine(cpStr path1, cpStr path2, cpStr path3, cpStr path4);
   static EString combine(cpStr path1, cpStr path2, cpStr path3);
   static EString combine(cpStr path1, cpStr path2);


   static Void getDirectoryName(cpStr path, EString &dirName);
   static Void getExtension(cpStr path, EString &ext);
   static Void getFileName(cpStr path, EString &fileName);
   static Void getFileNameWithoutExtension(cpStr path, EString &fileName);
   static Void getPathRoot(cpStr path, EString &root);

private:
   static Bool m_dirEqualsVolume;

   static Void cleanPath(cpStr path, EString &cleanPath);
   static Void insecureFullPath(cpStr path, EString &fullPath);
   static Bool isDsc(cChar c);
   static Bool isPathRooted(cpStr path);
   static Int findExtension(cpStr path);
};

inline cpStr EPath::getDirectorySeparatorString()
{
   return "/";
}

inline cChar EPath::getDirectorySeparatorChar()
{
   return '/';
}

inline cChar EPath::getAltDirectorySeparatorChar()
{
   return '/';
}

inline cChar EPath::getVolumeSeparatorChar()
{
   return ':';
}

inline cpStr EPath::getPathSeparatorChars()
{
   return "/:";
}

inline cpStr EPath::getInvalidPathChars()
{
   return "";
}

inline cpStr EPath::getInvalidFileNameChars()
{
   return "/";
}

inline Bool EPath::isDsc(cChar c)
{
   return c == getDirectorySeparatorChar() || c == getAltDirectorySeparatorChar();
}

#endif // #define __epath_h_included
