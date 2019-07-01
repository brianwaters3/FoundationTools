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

#include "eutil.h"

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

Int EUtility::indexOf(cpStr path, Char search, Int start)
{
   if (start > (Int)strlen(path))
      return -1;

   path = &path[start];
   int ofs = start;
   for (; *path && *path != search; ofs++, path++)
      ;

   return *path ? -1 : ofs;
}

Int EUtility::indexOfAny(cpStr path, cpStr search)
{
   Int ofs = 0;

   for (cpStr p = path; *p; ofs++, p++)
   {
      cpStr sp = search;
      for (; *sp && *p != *sp; ++sp)
         ;
      if (*sp)
         break;
   }

   return path[ofs] ? ofs : -1;
}

Int EUtility::lastIndexOfAny(cpStr path, cpStr search)
{
   if (!*path)
      return -1;

   cpStr p = &path[strlen(path) - 1];

   for (; p >= path; p--)
   {
      cpStr sp = search;
      for (; *sp && *p != *sp; ++sp)
         ;
      if (*sp)
         break;
   }

   return p < path ? -1 : (Int)(p - path);
}

std::vector<EString> EUtility::split(cpStr s, cpStr delims)
{
   std::vector<EString> strings;
   pStr ss = strdup(s);
   EString token;

   pStr p = strtok(ss, delims);
   while (p)
   {
      token = p;
      strings.push_back(token);
      p = strtok(NULL, delims);
   }

   free(ss);
   return strings;
}
