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

/// @cond DOXYGEN_EXCLUDE
DECLARE_ERROR_ADVANCED4(EBZip2Error_ReadOpen);
DECLARE_ERROR_ADVANCED4(EBZip2Error_WriteOpen);
DECLARE_ERROR_ADVANCED2(EBZip2Error_Bzip2ReadInit);
DECLARE_ERROR_ADVANCED2(EBZip2Error_Bzip2WriteInit);
DECLARE_ERROR_ADVANCED2(EBZip2Error_Bzip2Read);
DECLARE_ERROR_ADVANCED2(EBZip2Error_Bzip2Write);
/// @endcond

/// @file
/// @brief Wrapper around bzip2 library.
/// @details
/// EBzip2 provides the ability to compress or decompress a file utilizing
/// the Burrows-Wheeler compression algorithm.  The library is compatible
/// with the standard bzip2 program.  This library compresses/decompresses
/// between memory (decompressed) and a file (compressed).  The decompressed
/// data can be read on a buffer by buffer and does not require for the entire
/// file to be decompressed. See https://www.sourceware.org/bzip2/ for more
/// information.
/// 
class EBzip2
{
public:
   /// @cond DOXYGEN_EXCLUDE
   enum Operation
   {
      bz2opNone,
      bz2opRead,
      bz2opWrite
   };
   /// @endcond

   /// @brief Class constructor.
   EBzip2();
   /// @brief Class destructor.
   ~EBzip2();

   /// @brief Sets the file name that to be operated on.
   EString &setFileName(cpStr filename)
   {
      m_filename = filename;
      return m_filename;
   }
   /// @brief Gets the file name that to be operated on.
   EString &getFileName() { return m_filename; }

   /// @brief Sets the line terminator used by readLine().
   cChar setTerminator(cChar c)
   {
      m_term = c;
      return getTerminator();
   }
   /// @brief Gets the line terminator used by readLine().
   cChar getTerminator() { return m_term; }

   /// @brief Gets the last error value that occurred.
   Int getLastError() { return m_bzerror; }
   /// @brief Gets the description of the specified error value.
   static cpStr getErrorDesc(Int e);

   /// @brief The number of uncompressed bytes associated with the file written.
   ULongLong getBytesIn() { return m_bytesin; }
   /// @brief The number of compressed bytes associated with the file written.
   ULongLong getBytesOut() { return m_bytesout; }

   /// @brief True - the file is open, False - the file is closed
   Bool isOpen() { return m_fh ? True : False; }

   /// @brief Open the file for reading.
   Void readOpen(cpStr filename);
   /// @brief Open the file for writing.
   Void writeOpen(cpStr filename);
   /// @brief Close the file.
   Void close();
   /// @brief Read a specified number of decompressed bytes.
   Int read(pUChar pbuf, Int length);
   /// @brief Read a decompressed line terminated by the supplied terminator.
   Int readLine(pStr pbuf, Int length);
   /// @brief Write the specified number of decompressed bytes.
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
