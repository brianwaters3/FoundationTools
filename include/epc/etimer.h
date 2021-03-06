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

#ifndef __etimer_h_included
#define __etimer_h_included

#include "ebase.h"

class ETimer
{
public:
   ETimer();
   ETimer(const ETimer &a);
   ETimer(const epctime_t t);
   ~ETimer();

   void Start();
   void Stop();
   void Set(epctime_t a);
   epctime_t MilliSeconds(Bool bRestart = False);
   epctime_t MicroSeconds(Bool bRestart = False);

   ETimer &operator=(const ETimer &a);
   ETimer &operator=(const epctime_t t);

   operator epctime_t() { return _time; }

private:
   epctime_t _time;
   epctime_t _endtime;
};

#endif // #define __etimer_h_included
