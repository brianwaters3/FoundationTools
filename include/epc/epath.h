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

/// @file
/// @brief A path manipulation helper class.

#include "ebase.h"
#include "estring.h"
#include "eerror.h"

/// @cond DOXYGEN_EXCLUDE
DECLARE_ERROR_ADVANCED4(EPathError_ArgumentException);
/// @endcond

/// @brief A path manipulation helper class.
class EPath
{
public:
   /// @brief Retrieves the directory separator string.
   /// @return the directory separator string.
   static cpStr getDirectorySeparatorString();

   /// @brief Retrieves the directory separator character.
   /// @return the directory separator character.
   static cChar getDirectorySeparatorChar();
   /// @brief Retrieves the alternate directory separator character.
   /// @return the alternate directory separator character.
   static cChar getAltDirectorySeparatorChar();
   /// @brief Retrieves the volume separator character.
   /// @return the volume separator character.
   static cChar getVolumeSeparatorChar();

   /// @brief Retrieves the path separator characters.
   /// @return the path separator characters.
   static cpStr getPathSeparatorChars();
   /// @brief Retrieves the invalid path characters.
   /// @return the invalid path characters.
   static cpStr getInvalidPathChars();
   /// @brief Retrieves the invalid file name characters.
   /// @return the invalid file name characters.
   static cpStr getInvalidFileNameChars();

   /// @brief Changes the extension of an exist path name.
   /// @param path the original path name.
   /// @param extension the new extension.
   /// @param newPath the new path name.
   static Void changeExtension(cpStr path, cpStr extension, EString &newPath);

   /// @brief Combines individual path names into a single path name.
   /// @param path1 the first path name.
   /// @param path2 the second path name.
   /// @param path3 the third path name.
   /// @param path4 the fourth path name.
   /// @param path the combined path name.
   static Void combine(cpStr path1, cpStr path2, cpStr path3, cpStr path4, EString &path);
   /// @brief Combines individual path names into a single path name.
   /// @param path1 the first path name.
   /// @param path2 the second path name.
   /// @param path3 the third path name.
   /// @param path the combined path name.
   static Void combine(cpStr path1, cpStr path2, cpStr path3, EString &path);
   /// @brief Combines individual path names into a single path name.
   /// @param path1 the first path name.
   /// @param path2 the second path name.
   /// @param path the combined path name.
   static Void combine(cpStr path1, cpStr path2, EString &path);
   /// @brief Combines individual path names into a single path name.
   /// @param path1 the first path name.
   /// @param path2 the second path name.
   /// @param path3 the third path name.
   /// @param path4 the fourth path name.
   /// @return the combined path name.
   static EString combine(cpStr path1, cpStr path2, cpStr path3, cpStr path4);
   /// @brief Combines individual path names into a single path name.
   /// @param path1 the first path name.
   /// @param path2 the second path name.
   /// @param path3 the third path name.
   /// @return the combined path name.
   static EString combine(cpStr path1, cpStr path2, cpStr path3);
   /// @brief Combines individual path names into a single path name.
   /// @param path1 the first path name.
   /// @param path2 the second path name.
   /// @return the combined path name.
   static EString combine(cpStr path1, cpStr path2);

   /// @brief Retrieves the directory name from a file name.
   /// @param path the original file name.
   /// @param dirName the extracted directory name.
   static Void getDirectoryName(cpStr path, EString &dirName);
   /// @brief Retrieves the extension of a file name.
   /// @param path the original file name.
   /// @param ext the extracted file extension.
   static Void getExtension(cpStr path, EString &ext);
   /// @brief Retrieves just the file name from a fully qualified file name.
   /// @param path the fully qualified file name.
   /// @param fileName the extracted file name.
   static Void getFileName(cpStr path, EString &fileName);
   /// @brief Retrieves the file name without any extension.
   /// @param path the fully qualified file name.
   /// @param fileName the file name without any extension.
   static Void getFileNameWithoutExtension(cpStr path, EString &fileName);
   /// @brief Retrieves the path root.
   /// @param path the full path.
   /// @param root the path root.
   static Void getPathRoot(cpStr path, EString &root);

   /// @brief Retrieves the directory name from a file name.
   /// @param path the original file name.
   /// @return the extracted directory name.
   static EString getDirectoryName(cpStr path) { EString s; getDirectoryName(path, s); return s; }
   /// @brief Retrieves the extension of a file name.
   /// @param path the original file name.
   /// @return the extracted file extension.
   static EString getExtension(cpStr path) { EString s; getExtension(path, s); return s; }
   /// @brief Retrieves just the file name from a fully qualified file name.
   /// @param path the fully qualified file name.
   /// @return the extracted file name.
   static EString getFileName(cpStr path) { EString s; getFileName(path, s); return s; }
   /// @brief Retrieves the file name without any extension.
   /// @param path the fully qualified file name.
   /// @return the file name without any extension.
   static EString getFileNameWithoutExtension(cpStr path) { EString s; getFileNameWithoutExtension(path, s); return s; }
   /// @brief Retrieves the path root.
   /// @param path the full path.
   /// @return the path root.
   static EString getPathRoot(cpStr path) { EString s; getPathRoot(path, s); return s; }

private:
   static Bool m_dirEqualsVolume;

   static Void cleanPath(cpStr path, EString &cleanPath);
   static Void insecureFullPath(cpStr path, EString &fullPath);
   static Bool isDsc(cChar c);
   static Bool isPathRooted(cpStr path);
   static Int findExtension(cpStr path);
};

/// @cond DOXYGEN_EXCLUDE
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
/// @endcond

#endif // #define __epath_h_included
