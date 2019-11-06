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

#include "ebase.h"
#include "ebzip2.h"
#include <string.h>

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

/// @cond DOXYGEN_EXCLUDE

EBZip2Error_ReadOpen::EBZip2Error_ReadOpen(cpChar msg)
{
   setSevere();
   setTextf("Error opening [%s] for reading", msg);
   appendLastOsError();
}

EBZip2Error_WriteOpen::EBZip2Error_WriteOpen(cpChar msg)
{
   setSevere();
   setTextf("Error opening [%s] for writing", msg);
   appendLastOsError();
}

EBZip2Error_Bzip2ReadInit::EBZip2Error_Bzip2ReadInit(Int err)
{
   setSevere();
   setTextf("Error initializing bzip2 for reading - %s (%d)", EBzip2::getErrorDesc(err), err);
}

EBZip2Error_Bzip2WriteInit::EBZip2Error_Bzip2WriteInit(Int err)
{
   setSevere();
   setTextf("Error initializing bzip2 for writing - %s (%d)", EBzip2::getErrorDesc(err), err);
}

EBZip2Error_Bzip2Read::EBZip2Error_Bzip2Read(Int err)
{
   setSevere();
   setTextf("Error reading bzip2 block - %s (%d)", EBzip2::getErrorDesc(err), err);
}

EBZip2Error_Bzip2Write::EBZip2Error_Bzip2Write(Int err)
{
   setSevere();
   setTextf("Error writing bzip2 block - %s (%d)", EBzip2::getErrorDesc(err), err);
}

/// @endcond

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

EBzip2::EBzip2()
{
   m_fh = NULL;
   m_bfh = NULL;
   m_term = '\n';
   m_bzerror = BZ_OK;
   m_operation = bz2opNone;
   m_bytesin = 0;
   m_bytesout = 0;
}

EBzip2::~EBzip2()
{
   close();
}

Void EBzip2::readOpen(cpStr filename)
{
   m_fh = fopen(filename, "rb");

   if (!m_fh)
      throw EBZip2Error_ReadOpen(filename);

   m_bfh = BZ2_bzReadOpen(&m_bzerror, m_fh, 0, 0, NULL, 0);
   if (m_bzerror != BZ_OK)
   {
      close();
      throw EBZip2Error_Bzip2ReadInit(m_bzerror);
   }

   m_len = sizeof(m_buf);
   m_ofs = sizeof(m_buf);

   m_operation = bz2opRead;
}

Void EBzip2::writeOpen(cpStr filename)
{
   throw EError(EError::Error, "EBzip2::writeOpen() is not implemented");
}

void EBzip2::close()
{
   if (m_bfh)
   {
      unsigned int inl = 0, inh = 0, outl = 0, outh = 0;

      switch (m_operation)
      {
      case bz2opRead:
         BZ2_bzReadClose(&m_bzerror, m_bfh);
         break;
      case bz2opWrite:
         BZ2_bzWriteClose64(&m_bzerror, m_bfh, 0, &inl, &inh, &outl, &outh);
         break;
      default:
         break;
      }

      ulonginteger_t v;

      v.uli.lowPart = (Dword)inl;
      v.uli.highPart = (Dword)inh;
      m_bytesin = v.quadPart;

      v.uli.lowPart = (Dword)outl;
      v.uli.highPart = (Dword)outh;
      m_bytesout = v.quadPart;

      m_operation = bz2opNone;

      m_bfh = NULL;
   }

   if (m_fh)
   {
      ::fclose(m_fh);
      m_fh = NULL;
   }
}

Int EBzip2::read(pUChar pbuf, Int length)
{
   Int amtRead = 0;

   while (amtRead < length)
   {
      if (m_ofs == m_len)
      {
         if (m_len < (Int)sizeof(m_buf))
            break;

         m_len = BZ2_bzRead(&m_bzerror, m_bfh, m_buf, sizeof(m_buf));
         if (m_bzerror != BZ_OK && m_bzerror != BZ_STREAM_END)
            throw EBZip2Error_Bzip2Read(m_bzerror);

         m_ofs = 0;
      }

      Int amt = min(m_len - m_ofs, length - amtRead);

      memcpy(&pbuf[amtRead], &m_buf[m_ofs], amt);

      m_ofs += amt;
      amtRead += amt;
   }

   return amtRead;
}

Int EBzip2::readLine(pStr pbuf, Int length)
{
   Int amtRead = 0;

   while (amtRead < length - 1)
   {
      if (m_ofs == m_len)
      {
         if (m_len < (Int)sizeof(m_buf))
            break;

         m_len = BZ2_bzRead(&m_bzerror, m_bfh, m_buf, sizeof(m_buf));
         if (m_bzerror != BZ_OK && m_bzerror != BZ_STREAM_END)
            throw EBZip2Error_Bzip2Read(m_bzerror);

         m_ofs = 0;

         if (m_len == 0)
            break;
      }

      pUChar p = (pUChar)memchr(&m_buf[m_ofs], m_term, m_len - m_ofs);
      Int amt = (p == NULL) ? m_len - m_ofs : (Int)(p - &m_buf[m_ofs] + 1);

      memcpy(&pbuf[amtRead], &m_buf[m_ofs], amt);

      m_ofs += amt;
      amtRead += amt;

      if (pbuf[amtRead - 1] == m_term)
         break;
   }

   pbuf[amtRead] = '\0';

   return amtRead;
}

Int EBzip2::write(pUChar pbuf, Int length)
{
   throw EError(EError::Error, "EBzip2::write() is not implemented");
}

cpStr EBzip2::getErrorDesc(Int e)
{
   switch (e)
   {
   case BZ_OK:
      return "BZ_OK";
   case BZ_RUN_OK:
      return "BZ_RUN_OK";
   case BZ_FLUSH_OK:
      return "BZ_FLUSH_OK";
   case BZ_FINISH_OK:
      return "BZ_FINISH_OK";
   case BZ_STREAM_END:
      return "BZ_STREAM_END";
   case BZ_SEQUENCE_ERROR:
      return "BZ_SEQUENCE_ERROR";
   case BZ_PARAM_ERROR:
      return "BZ_PARAM_ERROR";
   case BZ_MEM_ERROR:
      return "BZ_MEM_ERROR";
   case BZ_DATA_ERROR:
      return "BZ_DATA_ERROR";
   case BZ_DATA_ERROR_MAGIC:
      return "BZ_DATA_ERROR_MAGIC";
   case BZ_IO_ERROR:
      return "BZ_IO_ERROR";
   case BZ_UNEXPECTED_EOF:
      return "BZ_UNEXPECTED_EOF";
   case BZ_OUTBUFF_FULL:
      return "BZ_OUTBUFF_FULL";
   case BZ_CONFIG_ERROR:
      return "BZ_CONFIG_ERROR";
   }

   return "UNKNOWN";
}
