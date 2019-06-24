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

#include "fttimer.h"

FTTimerElapsed &FTTimerElapsed::operator=(FTTimerElapsed &a)
{
   _time = a._time;
   _endtime = a._endtime;
   return *this;
}

FTTimerElapsed &FTTimerElapsed::operator=(fttime_t t)
{
   _time = t;
   _endtime = -1;
   return *this;
}

FTTimerElapsed::FTTimerElapsed()
{
   Start();
}

FTTimerElapsed::FTTimerElapsed(FTTimerElapsed &a)
{
   _endtime = -1;
   _time = a._time;
}

FTTimerElapsed::FTTimerElapsed(fttime_t t)
{
   _endtime = -1;
   _time = t;
}

FTTimerElapsed::~FTTimerElapsed() {}

void FTTimerElapsed::Start()
{
   struct timespec ts;
   if (clock_gettime(CLOCK_REALTIME, &ts))
      _time = 0;
   else
      _time = (((fttime_t)ts.tv_sec) * 1000000000) + ((fttime_t)ts.tv_nsec);
   _endtime = -1;
}

void FTTimerElapsed::Stop()
{
   struct timespec ts;
   fttime_t t;
   if (clock_gettime(CLOCK_REALTIME, &ts))
      t = 0;
   else
      t = (((fttime_t)ts.tv_sec) * 1000000000) + ((fttime_t)ts.tv_nsec);

   _endtime = t - _time;
}

void FTTimerElapsed::Set(fttime_t a)
{
   _time = a;
   _endtime = -1;
}

fttime_t FTTimerElapsed::MilliSeconds(Bool bRestart)
{
   if (_endtime == -1)
   {
      struct timespec ts;
      fttime_t t;
      if (clock_gettime(CLOCK_REALTIME, &ts))
         t = 0;
      else
         t = (((fttime_t)ts.tv_sec) * 1000000000) + ((fttime_t)ts.tv_nsec);

      fttime_t r = t - _time;
      if (bRestart)
         _time = t;
      return r / 1000000;
   }

   return _endtime / 1000000;
}

fttime_t FTTimerElapsed::MicroSeconds(Bool bRestart)
{
   if (_endtime == -1)
   {
      struct timespec ts;
      fttime_t t;
      if (clock_gettime(CLOCK_REALTIME, &ts))
         t = 0;
      else
         t = (((fttime_t)ts.tv_sec) * 1000000000) + ((fttime_t)ts.tv_nsec);
      fttime_t r = t - _time;
      if (bRestart)
         _time = t;
      return r / 1000;
   }

   return _endtime / 1000;
}
