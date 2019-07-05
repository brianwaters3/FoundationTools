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

#include "etimer.h"

ETimer &ETimer::operator=(const ETimer &a)
{
   _time = a._time;
   _endtime = a._endtime;
   return *this;
}

ETimer &ETimer::operator=(const epctime_t t)
{
   _time = t;
   _endtime = -1;
   return *this;
}

ETimer::ETimer()
{
   Start();
}

ETimer::ETimer(const ETimer &a)
{
   _endtime = -1;
   _time = a._time;
}

ETimer::ETimer(const epctime_t t)
{
   _endtime = -1;
   _time = t;
}

ETimer::~ETimer() {}

void ETimer::Start()
{
   struct timespec ts;
   if (clock_gettime(CLOCK_REALTIME, &ts))
      _time = 0;
   else
      _time = (((epctime_t)ts.tv_sec) * 1000000000) + ((epctime_t)ts.tv_nsec);
   _endtime = -1;
}

void ETimer::Stop()
{
   struct timespec ts;
   epctime_t t;
   if (clock_gettime(CLOCK_REALTIME, &ts))
      t = 0;
   else
      t = (((epctime_t)ts.tv_sec) * 1000000000) + ((epctime_t)ts.tv_nsec);

   _endtime = t - _time;
}

void ETimer::Set(epctime_t a)
{
   _time = a;
   _endtime = -1;
}

epctime_t ETimer::MilliSeconds(Bool bRestart)
{
   if (_endtime == -1)
   {
      struct timespec ts;
      epctime_t t;
      if (clock_gettime(CLOCK_REALTIME, &ts))
         t = 0;
      else
         t = (((epctime_t)ts.tv_sec) * 1000000000) + ((epctime_t)ts.tv_nsec);

      epctime_t r = t - _time;
      if (bRestart)
         _time = t;
      return r / 1000000;
   }

   return _endtime / 1000000;
}

epctime_t ETimer::MicroSeconds(Bool bRestart)
{
   if (_endtime == -1)
   {
      struct timespec ts;
      epctime_t t;
      if (clock_gettime(CLOCK_REALTIME, &ts))
         t = 0;
      else
         t = (((epctime_t)ts.tv_sec) * 1000000000) + ((epctime_t)ts.tv_nsec);
      epctime_t r = t - _time;
      if (bRestart)
         _time = t;
      return r / 1000;
   }

   return _endtime / 1000;
}
