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

#ifndef __emsg_h_included
#define __emsg_h_included

#include "ebase.h"
#include "etime.h"
#include "etimer.h"

class EMessage
{
   friend class EQueueBase;

public:
   EMessage()
   {
   }

   ~EMessage()
   {
   }

   EMessage &operator=(const EMessage &a)
   {
      return *this;
   }

   virtual Void getLength(ULong &length);
   virtual Void serialize(pVoid pBuffer, ULong &nOffset);
   virtual Void unserialize(pVoid pBuffer, ULong &nOffset);

   Void elementLength(Bool val, ULong &length)
   {
      length += sizeof(Bool);
   }
   Void elementLength(Char val, ULong &length)
   {
      length += sizeof(Char);
   }
   Void elementLength(UChar val, ULong &length)
   {
      length += sizeof(UChar);
   }
   Void elementLength(Short val, ULong &length)
   {
      length += sizeof(Short);
   }
   Void elementLength(UShort val, ULong &length)
   {
      length += sizeof(UShort);
   }
   Void elementLength(Long val, ULong &length)
   {
      length += sizeof(Long);
   }
   Void elementLength(ULong val, ULong &length)
   {
      length += sizeof(ULong);
   }
   Void elementLength(LongLong val, ULong &length)
   {
      length += sizeof(LongLong);
   }
   Void elementLength(ULongLong val, ULong &length)
   {
      length += sizeof(ULongLong);
   }
   Void elementLength(Float val, ULong &length)
   {
      length += sizeof(Float);
   }
   Void elementLength(Double val, ULong &length)
   {
      length += sizeof(Double);
   }
   Void elementLength(cpStr val, ULong &length)
   {
      length += (sizeof(UShort) + (UShort)strlen(val));
   }
   Void elementLength(ETimer &val, ULong &length)
   {
      length += sizeof(epctime_t);
   }
   Void elementLength(ETime &val, ULong &length)
   {
      length += sizeof(LongLong); //sizeof(val.getTimeVal().tv_sec);
      length += sizeof(LongLong); //sizeof(val.getTimeVal().tv_usec);
   }

   Void pack(Bool val, pVoid pBuffer, ULong &nOffset);
   Void pack(Char val, pVoid pBuffer, ULong &nOffset);
   Void pack(UChar val, pVoid pBuffer, ULong &nOffset);
   Void pack(Short val, pVoid pBuffer, ULong &nOffset);
   Void pack(UShort val, pVoid pBuffer, ULong &nOffset);
   //    Void pack(Int val, pVoid pBuffer, ULong &nOffset);
   //    Void pack(UInt val, pVoid pBuffer, ULong &nOffset);
   Void pack(Long val, pVoid pBuffer, ULong &nOffset);
   Void pack(ULong val, pVoid pBuffer, ULong &nOffset);
   Void pack(LongLong val, pVoid pBuffer, ULong &nOffset);
   Void pack(ULongLong val, pVoid pBuffer, ULong &nOffset);
   Void pack(Float val, pVoid pBuffer, ULong &nOffset);
   Void pack(Double val, pVoid pBuffer, ULong &nOffset);
   Void pack(cpStr val, pVoid pBuffer, ULong &nOffset);
   Void pack(ETimer &val, pVoid pBuffer, ULong &nOffset);
   Void pack(ETime &val, pVoid pBuffer, ULong &nOffset);
   Void pack(EString &val, pVoid pBuffer, ULong &nOffset);

   Void unpack(Bool &val, pVoid pBuffer, ULong &nOffset);
   Void unpack(Char &val, pVoid pBuffer, ULong &nOffset);
   Void unpack(UChar &val, pVoid pBuffer, ULong &nOffset);
   Void unpack(Short &val, pVoid pBuffer, ULong &nOffset);
   Void unpack(UShort &val, pVoid pBuffer, ULong &nOffset);
   //    Void unpack(Int &val, pVoid pBuffer, ULong &nOffset);
   //    Void unpack(UInt &val, pVoid pBuffer, ULong &nOffset);
   Void unpack(Long &val, pVoid pBuffer, ULong &nOffset);
   Void unpack(ULong &val, pVoid pBuffer, ULong &nOffset);
   Void unpack(LongLong &val, pVoid pBuffer, ULong &nOffset);
   Void unpack(ULongLong &val, pVoid pBuffer, ULong &nOffset);
   Void unpack(Float &val, pVoid pBuffer, ULong &nOffset);
   Void unpack(Double &val, pVoid pBuffer, ULong &nOffset);
   Void unpack(pStr val, pVoid pBuffer, ULong &nOffset);
   Void unpack(ETimer &val, pVoid pBuffer, ULong &nOffset);
   Void unpack(ETime &val, pVoid pBuffer, ULong &nOffset);
   Void unpack(EString &val, pVoid Buffer, ULong &nOffset);

private:
   ETime m_timer;
   Long m_msgType;
};

template <class T>
class EMessageVector : public EMessage
{
public:
   EMessageVector()
   {
   }

   EMessageVector(EMessageVector const &val)
   {
      m_list = val.m_list;
   }

   ~EMessageVector()
   {
   }

   vector<T> &getList()
   {
      return m_list;
   }

   Void vectorLength(ULong &length)
   {
      ULong len = 0;
      ULong vectorLength = m_list.size();

      elementLength(vectorLength, len);
      for (ULong i = 0; i < vectorLength; i++)
         m_list[i].getLength(len);

      length += len;
   }

   Void packVector(pVoid pBuffer, ULong &nOffset)
   {
      ULong vectorLength = m_list.size();

      pack(vectorLength, pBuffer, nOffset);
      for (ULong i = 0; i < vectorLength; i++)
         m_list[i].serialize(pBuffer, nOffset);
   }

   Void unpackVector(pVoid Buffer, ULong &nOffset)
   {
      ULong vectorLength;

      unpack(vectorLength, Buffer, nOffset);

      for (ULong i = 0; i < vectorLength; i++)
      {
         T val;
         val.unserialize(Buffer, nOffset);
         m_list.push_back(val);
      }
   }

private:
   vector<T> m_list;
};
#endif // #ifndef __emsg_h_included
