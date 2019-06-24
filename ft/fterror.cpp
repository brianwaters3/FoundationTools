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

#include <errno.h>
#include <string.h>
#include <stdarg.h>

#include "ftbase.h"
#include "fterror.h"

cpStr FTError::m_pszSeverity[] =
    {
        "Info", "Warning", "Error"};

Void FTError::setTextf(cpStr pszFormat, ...)
{
   Char szBuff[2048];
   va_list pArgs;
   va_start(pArgs, pszFormat);
   vsnprintf(szBuff, sizeof(szBuff), pszFormat, pArgs);
   setText(szBuff);
}

Void FTError::appendTextf(cpStr pszFormat, ...)
{
   Char szBuff[2048];
   va_list pArgs;
   va_start(pArgs, pszFormat);
   vsnprintf(szBuff, sizeof(szBuff), pszFormat, pArgs);
   appendText(szBuff);
}

Void FTError::setLastOsError(Dword dwError)
{
   clear();
   appendLastOsError(dwError);
}

Void FTError::appendLastOsError(Dword dwError)
{
   m_dwError = (dwError == (Dword)-1) ? errno : dwError;
   cpStr pMsg = strerror(m_dwError);
   if (pMsg)
      appendTextf(", error [%ld], [\"%s\"]", m_dwError, pMsg);
   else
      appendTextf(", error [%ld], [Error %ld]", m_dwError, m_dwError);
}

cpStr FTError::getError(Int nError, FTErrorMapEntry *pErrors)
{
   Int i;
   for (i = 0; pErrors[i].m_pszError != NULL; i++)
   {
      if (pErrors[i].m_nError == nError)
         break;
   }

   return pErrors[i].m_pszError != NULL ? pErrors[i].m_pszError : "Undefined";
}
