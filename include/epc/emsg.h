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

#ifndef __emsg_h_included
#define __emsg_h_included

/// @file
/// @brief Classes used to encode and decode messages sent to and recieved from a message queue.

#include "ebase.h"
#include "etime.h"
#include "etimer.h"

/// @brief The message queue base message class.
class EMessage
{
   friend class EQueueBase;

public:
   /// @brief Default constructor.
   EMessage()
   {
   }

   /// @brief Class destructor.
   ~EMessage()
   {
   }

   /// @brief Calculates the packed length of a message.
   /// @param length a reference to the length value to update.
   virtual Void getLength(ULong &length);
   /// @brief Serializes (packs) a message.
   /// @param pBuffer a pointer to the destination buffer.
   /// @param nOffset a reference to the next location to write to in the buffer.
   virtual Void serialize(pVoid pBuffer, ULong &nOffset);
   /// @brief Unserializes (unpacks) a message.
   /// @param pBuffer a pointer to the raw buffer to unpack.
   /// @param nOffset a reference to the next location in the buffer to unpack.
   virtual Void unserialize(pVoid pBuffer, ULong &nOffset);

   /// @brief Adds the length of a Bool value to the message length.
   /// @param val the Bool value to calculate the length for.
   /// @param length the length value to update.
   Void elementLength(Bool val, ULong &length)
   {
      length += sizeof(Bool);
   }
   /// @brief Adds the length of a Bool value to the message length.
   /// @param val the Char value to calculate the length for.
   /// @param length the length value to update.
   Void elementLength(Char val, ULong &length)
   {
      length += sizeof(Char);
   }
   /// @brief Adds the length of a Bool value to the message length.
   /// @param val the UChar value to calculate the length for.
   /// @param length the length value to update.
   Void elementLength(UChar val, ULong &length)
   {
      length += sizeof(UChar);
   }
   /// @brief Adds the length of a Bool value to the message length.
   /// @param val the Short value to calculate the length for.
   /// @param length the length value to update.
   Void elementLength(Short val, ULong &length)
   {
      length += sizeof(Short);
   }
   /// @brief Adds the length of a Bool value to the message length.
   /// @param val the UShort value to calculate the length for.
   /// @param length the length value to update.
   Void elementLength(UShort val, ULong &length)
   {
      length += sizeof(UShort);
   }
   /// @brief Adds the length of a Bool value to the message length.
   /// @param val the Long value to calculate the length for.
   /// @param length the length value to update.
   Void elementLength(Long val, ULong &length)
   {
      length += sizeof(Long);
   }
   /// @brief Adds the length of a Bool value to the message length.
   /// @param val the ULong value to calculate the length for.
   /// @param length the length value to update.
   Void elementLength(ULong val, ULong &length)
   {
      length += sizeof(ULong);
   }
   /// @brief Adds the length of a Bool value to the message length.
   /// @param val the LongLong value to calculate the length for.
   /// @param length the length value to update.
   Void elementLength(LongLong val, ULong &length)
   {
      length += sizeof(LongLong);
   }
   /// @brief Adds the length of a Bool value to the message length.
   /// @param val the ULongLong value to calculate the length for.
   /// @param length the length value to update.
   Void elementLength(ULongLong val, ULong &length)
   {
      length += sizeof(ULongLong);
   }
   /// @brief Adds the length of a Bool value to the message length.
   /// @param val the Float value to calculate the length for.
   /// @param length the length value to update.
   Void elementLength(Float val, ULong &length)
   {
      length += sizeof(Float);
   }
   /// @brief Adds the length of a Bool value to the message length.
   /// @param val the Double value to calculate the length for.
   /// @param length the length value to update.
   Void elementLength(Double val, ULong &length)
   {
      length += sizeof(Double);
   }
   /// @brief Adds the length of a Bool value to the message length.
   /// @param val the NULL terminated string value to calculate the length for.
   /// @param length the length value to update.
   Void elementLength(cpStr val, ULong &length)
   {
      length += (sizeof(UShort) + (UShort)strlen(val));
   }
   /// @brief Adds the length of a Bool value to the message length.
   /// @param val the Timer value to calculate the length for.
   /// @param length the length value to update.
   Void elementLength(ETimer &val, ULong &length)
   {
      length += sizeof(epctime_t);
   }
   /// @brief Adds the length of a Bool value to the message length.
   /// @param val the Time value to calculate the length for.
   /// @param length the length value to update.
   Void elementLength(ETime &val, ULong &length)
   {
      length += sizeof(LongLong); //sizeof(val.getTimeVal().tv_sec);
      length += sizeof(LongLong); //sizeof(val.getTimeVal().tv_usec);
   }

   /// @brief Packs the Bool value.
   /// @param val the value to pack.
   /// @param pBuffer a pointer to the destination buffer.
   /// @param nOffset the offset in the destination buffer to write to.
   Void pack(Bool val, pVoid pBuffer, ULong &nOffset);
   /// @brief Packs the Char value.
   /// @param val the value to pack.
   /// @param pBuffer a pointer to the destination buffer.
   /// @param nOffset the offset in the destination buffer to write to.
   Void pack(Char val, pVoid pBuffer, ULong &nOffset);
   /// @brief Packs the UChar value.
   /// @param val the value to pack.
   /// @param pBuffer a pointer to the destination buffer.
   /// @param nOffset the offset in the destination buffer to write to.
   Void pack(UChar val, pVoid pBuffer, ULong &nOffset);
   /// @brief Packs the Short value.
   /// @param val the value to pack.
   /// @param pBuffer a pointer to the destination buffer.
   /// @param nOffset the offset in the destination buffer to write to.
   Void pack(Short val, pVoid pBuffer, ULong &nOffset);
   /// @brief Packs the UShort value.
   /// @param val the value to pack.
   /// @param pBuffer a pointer to the destination buffer.
   /// @param nOffset the offset in the destination buffer to write to.
   Void pack(UShort val, pVoid pBuffer, ULong &nOffset);
   /// @brief Packs the Long value.
   /// @param val the value to pack.
   /// @param pBuffer a pointer to the destination buffer.
   /// @param nOffset the offset in the destination buffer to write to.
   Void pack(Long val, pVoid pBuffer, ULong &nOffset);
   /// @brief Packs the ULong value.
   /// @param val the value to pack.
   /// @param pBuffer a pointer to the destination buffer.
   /// @param nOffset the offset in the destination buffer to write to.
   Void pack(ULong val, pVoid pBuffer, ULong &nOffset);
   /// @brief Packs the LongLong value.
   /// @param val the value to pack.
   /// @param pBuffer a pointer to the destination buffer.
   /// @param nOffset the offset in the destination buffer to write to.
   Void pack(LongLong val, pVoid pBuffer, ULong &nOffset);
   /// @brief Packs the ULongLong value.
   /// @param val the value to pack.
   /// @param pBuffer a pointer to the destination buffer.
   /// @param nOffset the offset in the destination buffer to write to.
   Void pack(ULongLong val, pVoid pBuffer, ULong &nOffset);
   /// @brief Packs the Float value.
   /// @param val the value to pack.
   /// @param pBuffer a pointer to the destination buffer.
   /// @param nOffset the offset in the destination buffer to write to.
   Void pack(Float val, pVoid pBuffer, ULong &nOffset);
   /// @brief Packs the Double value.
   /// @param val the value to pack.
   /// @param pBuffer a pointer to the destination buffer.
   /// @param nOffset the offset in the destination buffer to write to.
   Void pack(Double val, pVoid pBuffer, ULong &nOffset);
   /// @brief Packs the NULL terminated string value.
   /// @param val the value to pack.
   /// @param pBuffer a pointer to the destination buffer.
   /// @param nOffset the offset in the destination buffer to write to.
   Void pack(cpStr val, pVoid pBuffer, ULong &nOffset);
   /// @brief Packs the Timer value.
   /// @param val the value to pack.
   /// @param pBuffer a pointer to the destination buffer.
   /// @param nOffset the offset in the destination buffer to write to.
   Void pack(ETimer &val, pVoid pBuffer, ULong &nOffset);
   /// @brief Packs the Time value.
   /// @param val the value to pack.
   /// @param pBuffer a pointer to the destination buffer.
   /// @param nOffset the offset in the destination buffer to write to.
   Void pack(ETime &val, pVoid pBuffer, ULong &nOffset);
   /// @brief Packs the string value.
   /// @param val the value to pack.
   /// @param pBuffer a pointer to the destination buffer.
   /// @param nOffset the offset in the destination buffer to write to.
   Void pack(EString &val, pVoid pBuffer, ULong &nOffset);

   /// @brief Unpacks a Bool value.
   /// @param val the destination for the value.
   /// @param pBuffer a pointer to the source buffer.
   /// @param nOffset the offset in the source buffer to read from.
   Void unpack(Bool &val, pVoid pBuffer, ULong &nOffset);
   /// @brief Unpacks a Char value.
   /// @param val the destination for the value.
   /// @param pBuffer a pointer to the source buffer.
   /// @param nOffset the offset in the source buffer to read from.
   Void unpack(Char &val, pVoid pBuffer, ULong &nOffset);
   /// @brief Unpacks a UChar value.
   /// @param val the destination for the value.
   /// @param pBuffer a pointer to the source buffer.
   /// @param nOffset the offset in the source buffer to read from.
   Void unpack(UChar &val, pVoid pBuffer, ULong &nOffset);
   /// @brief Unpacks a Short value.
   /// @param val the destination for the value.
   /// @param pBuffer a pointer to the source buffer.
   /// @param nOffset the offset in the source buffer to read from.
   Void unpack(Short &val, pVoid pBuffer, ULong &nOffset);
   /// @brief Unpacks a UShort value.
   /// @param val the destination for the value.
   /// @param pBuffer a pointer to the source buffer.
   /// @param nOffset the offset in the source buffer to read from.
   Void unpack(UShort &val, pVoid pBuffer, ULong &nOffset);
   /// @brief Unpacks a Long value.
   /// @param val the destination for the value.
   /// @param pBuffer a pointer to the source buffer.
   /// @param nOffset the offset in the source buffer to read from.
   Void unpack(Long &val, pVoid pBuffer, ULong &nOffset);
   /// @brief Unpacks a ULong value.
   /// @param val the destination for the value.
   /// @param pBuffer a pointer to the source buffer.
   /// @param nOffset the offset in the source buffer to read from.
   Void unpack(ULong &val, pVoid pBuffer, ULong &nOffset);
   /// @brief Unpacks a LongLong value.
   /// @param val the destination for the value.
   /// @param pBuffer a pointer to the source buffer.
   /// @param nOffset the offset in the source buffer to read from.
   Void unpack(LongLong &val, pVoid pBuffer, ULong &nOffset);
   /// @brief Unpacks a ULongLong value.
   /// @param val the destination for the value.
   /// @param pBuffer a pointer to the source buffer.
   /// @param nOffset the offset in the source buffer to read from.
   Void unpack(ULongLong &val, pVoid pBuffer, ULong &nOffset);
   /// @brief Unpacks a Float value.
   /// @param val the destination for the value.
   /// @param pBuffer a pointer to the source buffer.
   /// @param nOffset the offset in the source buffer to read from.
   Void unpack(Float &val, pVoid pBuffer, ULong &nOffset);
   /// @brief Unpacks a Double value.
   /// @param val the destination for the value.
   /// @param pBuffer a pointer to the source buffer.
   /// @param nOffset the offset in the source buffer to read from.
   Void unpack(Double &val, pVoid pBuffer, ULong &nOffset);
   /// @brief Unpacks a NULL terminated string value.
   /// @param val the destination for the value.
   /// @param pBuffer a pointer to the source buffer.
   /// @param nOffset the offset in the source buffer to read from.
   Void unpack(pStr val, pVoid pBuffer, ULong &nOffset);
   /// @brief Unpacks a Timer value.
   /// @param val the destination for the value.
   /// @param pBuffer a pointer to the source buffer.
   /// @param nOffset the offset in the source buffer to read from.
   Void unpack(ETimer &val, pVoid pBuffer, ULong &nOffset);
   /// @brief Unpacks a Time value.
   /// @param val the destination for the value.
   /// @param pBuffer a pointer to the source buffer.
   /// @param nOffset the offset in the source buffer to read from.
   Void unpack(ETime &val, pVoid pBuffer, ULong &nOffset);
   /// @brief Unpacks a string value.
   /// @param val the destination for the value.
   /// @param pBuffer a pointer to the source buffer.
   /// @param nOffset the offset in the source buffer to read from.
   Void unpack(EString &val, pVoid pBuffer, ULong &nOffset);
};

/// @brief A vector of fields.
template <class T>
class EMessageVector : public EMessage
{
public:
   /// @brief Default constructor.
   EMessageVector()
   {
   }

   EMessageVector(EMessageVector const &val)
   /// @brief Copy constructor.
   /// @param val the EMessageVector to copy.
   {
      m_list = val.m_list;
   }

   /// @brief Class destructor.
   ~EMessageVector()
   {
   }

   /// @brief Retrieves the message vector.
   /// @return the message vector.
   vector<T> &getList()
   {
      return m_list;
   }

   /// @brief Calcuates the packed length of the vector.
   /// @param length the length value to update.
   Void vectorLength(ULong &length)
   {
      ULong len = 0;
      ULong vectorLength = m_list.size();

      elementLength(vectorLength, len);
      for (ULong i = 0; i < vectorLength; i++)
         m_list[i].getLength(len);

      length += len;
   }

   /// @brief Packs the vector.
   /// @param pBuffer a pointer to the destination buffer.
   /// @param nOffset the offset into the destination buffer to write the data to.
   Void packVector(pVoid pBuffer, ULong &nOffset)
   {
      ULong vectorLength = m_list.size();

      pack(vectorLength, pBuffer, nOffset);
      for (ULong i = 0; i < vectorLength; i++)
         m_list[i].serialize(pBuffer, nOffset);
   }

   /// @brief Unpacks a vector.
   /// @param Buffer a pointer to the source buffer to unpack.
   /// @param nOffset the offset into the source buffer to read the data from.
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
