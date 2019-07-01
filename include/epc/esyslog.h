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

#ifndef __esyslog_h_included
#define __esyslog_h_included

#include <syslog.h>

#include "etypes.h"
#include "estring.h"

class ESysLog
{
public:
   ESysLog();
   ~ESysLog();

   EString &getIdentity() { return m_ident; }
   EString &setIdentity(cpStr v)
   {
      m_ident = v;
      return getIdentity();
   }
   Int getOption() { return m_option; }
   Int setOption(Int v)
   {
      m_option = v;
      return getOption();
   }
   Int getFacility() { return m_facility; }
   Int setFacility(Int v)
   {
      m_facility = v;
      return getFacility();
   }

   void open();
   void close();

   static void syslog(Int priority, cpStr format, va_list ap);

protected:
private:
   EString m_ident;
   Int m_option;
   Int m_facility;
};

#endif // __esyslog_h_included
