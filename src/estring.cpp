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

#include <stdarg.h>
#include <algorithm>

#include "ebase.h"
#include "estring.h"
#include "eutil.h"

template<class T>
class Buffer
{
public:
   Buffer(size_t size) { msize = size; mbuf = new T[msize]; }
   ~Buffer() { if (mbuf) delete [] mbuf; }
   T *get() { return mbuf; }
private:
   Buffer();
   size_t msize;
   T *mbuf;
};

EString &EString::format(cpChar pszFormat, ...)
{
   // Char szBuff[2048];
   // va_list pArgs;
   // va_start(pArgs, pszFormat);
   // vsnprintf(szBuff, sizeof(szBuff), pszFormat, pArgs);
   // va_end(pArgs);

   // assign(szBuff);

   // return *this;
   va_list args;

   va_start( args, pszFormat );
   size_t size = vsnprintf( NULL, 0, pszFormat, args ) + 1; // Extra space for '\0'
   va_end( args );

   Buffer<char> buf( size );

   va_start( args, pszFormat );
   vsnprintf( buf.get(), size, pszFormat, args  );
   va_end( args );

   assign( buf.get() );

   return *this; // We don't want the '\0' inside
}

EString &EString::tolower()
{
   std::transform(this->begin(), this->end(), this->begin(), ::tolower);
   return *this;
}

EString &EString::toupper()
{
   std::transform(this->begin(), this->end(), this->begin(), ::toupper);
   return *this;
}

EString &EString::replaceAll(cpStr srch, size_t srchlen, cpStr rplc, size_t rplclen)
{
   return EUtility::replaceAll(*this, srch, srchlen, rplc, rplclen);
}

EString EString::replaceAllCopy(cpStr srch, size_t srchlen, cpStr rplc, size_t rplclen)
{
   return EUtility::replaceAllCopy(*this, srch, srchlen, rplc, rplclen);
}
