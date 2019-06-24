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

#ifndef __ftautohandle_h_included
#define __ftautohandle_h_included

#include <exception>

struct bad_handle : public std::exception
{
   bad_handle() : std::exception("bad handle") {}
};

class FTAutoHandle
{
   HANDLE _handle;

public:
   FTAutoHandle() : _handle(INVALID_HANDLE_VALUE) {}
   explicit FTAutoHandle(HANDLE handle) : _handle(handle)
   {
      if (!isvalid())
         throw bad_handle();
   }
   ~FTAutoHandle()
   {
      release();
   }
   Bool isvalid() const
   {
      return (INVALID_HANDLE_VALUE != _handle) && (NULL != _handle);
   }
   Void release()
   {
      if (isvalid())
      {
         CloseHandle(_handle);
         _handle = INVALID_HANDLE_VALUE;
      }
   }
   operator HANDLE() const
   {
      if (!isvalid())
         throw bad_handle();
      return _handle;
   }
   const FTAutoHandle &operator=(HANDLE handle)
   {
      release();
      _handle = handle;
      return *this;
   }
   const FTAutoHandle &operator=(FTAutoHandle &other)
   {
      release();
      _handle = other._handle;
      other._handle = INVALID_HANDLE_VALUE;
      return *this;
   }
};

#endif // #define __ftautohandle_h_included
