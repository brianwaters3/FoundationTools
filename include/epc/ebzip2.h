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

#ifndef __ebzip2_h_included
#define __ebzip2_h_included

#include "estring.h"
#include "eerror.h"
#include "bzlib.h"

DECLARE_ERROR_ADVANCED4(EBZip2Error_ReadOpen);
DECLARE_ERROR_ADVANCED4(EBZip2Error_WriteOpen);
DECLARE_ERROR_ADVANCED2(EBZip2Error_Bzip2ReadInit);
DECLARE_ERROR_ADVANCED2(EBZip2Error_Bzip2WriteInit);
DECLARE_ERROR_ADVANCED2(EBZip2Error_Bzip2Read);
DECLARE_ERROR_ADVANCED2(EBZip2Error_Bzip2Write);

class EBzip2
{
public:
   enum Operation
   {
      bz2opNone,
      bz2opRead,
      bz2opWrite
   };

   EBzip2();
   ~EBzip2();

   EString &setFileName(cpStr filename)
   {
      m_filename = filename;
      return m_filename;
   }
   EString &getFileName() { return m_filename; }

   cChar setTerminator(cChar c)
   {
      m_term = c;
      return getTerminator();
   }
   cChar getTerminator() { return m_term; }

   Int getLastError() { return m_bzerror; }
   static cpStr getErrorDesc(Int e);

   ULongLong getBytesIn() { return m_bytesin; }
   ULongLong getBytesOut() { return m_bytesout; }

   Bool isOpen() { return m_fh ? True : False; }

   Void readOpen(cpStr filename);
   Void writeOpen(cpStr filename);
   Void close();
   Int read(pUChar pbuf, Int length);
   Int readLine(pStr pbuf, Int length);
   Int write(pUChar pbuf, Int length);

private:
   EString m_filename;
   FILE *m_fh;
   BZFILE *m_bfh;
   Char m_term;

   Int m_bzerror;
   Operation m_operation;
   ULongLong m_bytesin;
   ULongLong m_bytesout;
   Int m_len;
   Int m_ofs;
   UChar m_buf[65536];
};

#endif // #define __ebzip2_h_included
