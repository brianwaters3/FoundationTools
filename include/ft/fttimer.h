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

#ifndef __fttimer_h_included
#define __fttimer_h_included

#include "ftbase.h"

class FTTimerElapsed
{
public:
   FTTimerElapsed();
   FTTimerElapsed(FTTimerElapsed &a);
   FTTimerElapsed(fttime_t t);
   ~FTTimerElapsed();

   void Start();
   void Stop();
   void Set(fttime_t a);
   fttime_t MilliSeconds(Bool bRestart = False);
   fttime_t MicroSeconds(Bool bRestart = False);

   FTTimerElapsed &operator=(FTTimerElapsed &a);
   FTTimerElapsed &operator=(fttime_t t);

   operator fttime_t() { return _time; }

private:
   fttime_t _time;
   fttime_t _endtime;
};

#endif // #define __fttimer_h_included
