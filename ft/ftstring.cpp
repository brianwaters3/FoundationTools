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

#include <stdarg.h>
#include <algorithm>

#include "ftbase.h"
#include "ftstring.h"

FTString &FTString::format(cpChar pszFormat, ...)
{
   Char szBuff[2048];
   va_list pArgs;
   va_start(pArgs, pszFormat);
   vsnprintf(szBuff, sizeof(szBuff), pszFormat, pArgs);
   va_end(pArgs);

   assign(szBuff);

   return *this;
}

FTString &FTString::tolower()
{
   std::transform(this->begin(), this->end(), this->begin(), ::tolower);
   return *this;
}

FTString &FTString::toupper()
{
   std::transform(this->begin(), this->end(), this->begin(), ::toupper);
   return *this;
}
