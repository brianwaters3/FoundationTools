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

#ifndef __etimer_h_included
#define __etimer_h_included

/// @file

#include "ebase.h"

/// @brief Implements a stopwatch style timer.
class ETimer
{
public:
   /// @brief Default constructor.
   ETimer();
   /// @brief Copy constructor.
   /// @param a the ETimer object to copy.
   ETimer(const ETimer &a);
   /// @brief Class constructor.
   /// @param t an epctime_t value to initilize the timer.
   ETimer(const epctime_t t);
   /// @brief Class desctructor.
   ~ETimer();

   /// @brief Starts the timer.
   Void Start();
   /// @brief Stops the timer.
   Void Stop();
   /// @brief Assigns a value to the timer.
   /// @param a the value to assign to the timer.
   Void Set(epctime_t a);
   /// @brief Retrieves the current value of the timer in milliseconds.
   /// @param bRestart if True, the timer is restarted, otherwise it continues.
   epctime_t MilliSeconds(Bool bRestart = False);
   /// @brief Retrieves the current value of the timer in microseconds.
   /// @param bRestart if True, the timer is restarted, otherwise it continues.
   epctime_t MicroSeconds(Bool bRestart = False);

   /// @brief Assignment operator.
   /// @param a ETimer value to assign.
   /// @return a reference to this ETimer object.
   ETimer &operator=(const ETimer &a);
   /// @brief Assignment operator.
   /// @param t the epctime_t value to assign.
   /// @return a reference to this ETimer object.
   ETimer &operator=(const epctime_t t);

   /// @brief Retrieves the internal epctime_t value.
   operator epctime_t() { return _time; }

private:
   epctime_t _time;
   epctime_t _endtime;
};

#endif // #define __etimer_h_included
