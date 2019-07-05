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

#ifndef __etime_h_included
#define __etime_h_included

#include <sys/time.h>
#include <stdint.h>

#include "ebase.h"
#include "estring.h"

#define TIME_SEP 1
#define DATE_SEP 2
#define MONTH_SEP 4
#define AM_SEP 8
#define PM_SEP 16

#define MON_DAY_YEAR_ORDER 1
#define YEAR_MON_DAY_ORDER 2
#define YEAR_DAY_MON_ORDER 4
#define DAY_MON_YEAR_ORDER 8
#define MON_YEAR_DAY_ORDER 16

#define DATE_FIELDS 7
#define SWAP_POSITION(a, b) \
   {                        \
      int temp_buf;         \
      temp_buf = a;         \
      a = b;                \
      b = temp_buf;         \
   }
#define GIVE_YEAR(year)                                                 \
   do                                                                   \
   {                                                                    \
      year = year < 30 ? 2000 + year : year < 100 ? 1900 + year : year; \
   } while (0)

#define stringcmp_format(string1, string2, count) __builtin_strncasecmp(string1, string2, count)
#define format_localtime_s(a, b) localtime_r(b, a)
#define format_gmtime_s(a, b) gmtime_r(b, a)
#define format_sprintf_s __builtin_snprintf

#define BASE_YEAR 1900
#define NUM_OF_DAYS_LEAPYEAR 366
#define NUM_OF_DAYS_YEAR 365
#define NUM_OF_DAYS_WEEK 7

#define IF_LEAPYEAR(year) (!((year) % 4) && (((year) % 100) || !((year) % 400)))

#define FORMAT_TIMEZONE timezone

#define FORMAT_DAYLIGHT daylight

size_t format_time_into_string(pStr st, size_t max_limit,
                               cpStr type_format, const struct tm *time_f, const struct timeval *tval);

pStr format_time_into_specs(cpStr format, const struct tm *t, pStr pt, cpStr ptlim, const struct timeval *tv);
pStr convert_date_time_format(const Int n, cpStr format, pStr pt, cpStr ptlim);
pStr add_timeformat_to_string(cpStr str, pStr pt, cpStr ptlim);

enum strftime_format
{

   NAME_DAY_WEEK_ABB = 'a',
   FULL_DAY_NAME = 'A',
   ABBRE_MON_NAME = 'b',
   FULL_MON_NAME = 'B',
   DATE_TIME = 'c',
   CENTURY_NUMBER = 'C',
   DEC_DATE_MON = 'd',
   MON_DAY_YEAR = 'D',
   DAY_AS_DECIMAL_0 = 'e',
   ALTERNATE_ERA = 'E',
   YEAR_MON_DAY = 'F',
   TWO_DIG_YEAR = 'g',
   FOUR_DIG_YEAR = 'G',
   ABBRE_MON_NAME_2 = 'h',
   HOUR_AS_24_CLK = 'H',
   HOUR_AS_12_CLK = 'I',
   DAY_AS_DECIMAL = 'j',
   HOUR_AS_24_SINGLE = 'k',
   HOUR_AS_12_SINGLE = 'l',
   MON_AS_DECIMAL = 'm',
   MIN_AS_DECIMAL = 'M',
   NEW_LINE = 'n',
   MODIFIER = 'O',
   AM_PM = 'p',
   am_pm = 'P',
   TIME_AM_PM = 'r',
   TIME_HOUR_MIN = 'R',
   EPOCH_TIME = 's',
   SECONDS_AS_DEC = 'S',
   TAB_CHARACTER = 't',
   TIME_IN_24_HOUR = 'T',
   DAY_WEEK_AS_DEC = 'u',
   WEEK_NUM_AS_DEC = 'U',
   ISO_WEEK_NUM = 'V',
   DAY_OF_WEEK = 'w',
   WEEK_NUM_AS_DEC_MON = 'W',
   DATE_WITHOUT_TIME = 'x',
   TIME_WITHOUT_DATE = 'X',
   YEAR_WITHOUT_CEN = 'y',
   YEAR_WIT_CEN = 'Y',
   HOUR_MIN_OFFSET = 'z',
   TIMEZONE_NAME = 'Z',
   DATE_TIME_TZ = '+',
   LITERAL = '%',
   RAND_VALUE_1 = 'i',
   RAND_VALUE_2 = 'v',
   MICRO_SEC = '1',
   MILLI_SEC = '0',
   NULL_CHAR = '\0'
};

struct parse_date
{
   UInt field_flags[DATE_FIELDS];
   UInt field_parse_flags;
   UInt field_values[DATE_FIELDS];
   UInt field_count;
};

struct ntp_time_t
{
   UInt second;
   UInt fraction;
};

class ETime
{
public:
   ETime()
   {
      *this = Now();
   }

   ETime(const ETime &a)
   {
      *this = a;
   }

   ETime(Int sec, Int usec)
   {
      m_time.tv_sec = sec;
      m_time.tv_usec = usec;
   }

   ETime(LongLong ms)
   {
      set(ms);
   }

   ETime(Int year, Int mon, Int day, Int hour, Int min, Int sec, Bool isLocal);

   ~ETime()
   {
   }

   ETime &operator=(const ETime &a)
   {
      m_time.tv_sec = a.m_time.tv_sec;
      m_time.tv_usec = a.m_time.tv_usec;

      return *this;
   }

   Bool operator==(const ETime &a)
   {
      return ((m_time.tv_sec == a.m_time.tv_sec) && (m_time.tv_usec == a.m_time.tv_usec));
   }

   Bool operator!=(const ETime &a)
   {
      return ((m_time.tv_sec != a.m_time.tv_sec) || (m_time.tv_usec != a.m_time.tv_usec));
   }

   Bool operator<(const ETime &a)
   {
      if (m_time.tv_sec < a.m_time.tv_sec)
         return true;
      if (m_time.tv_sec == a.m_time.tv_sec)
      {
         if (m_time.tv_usec < a.m_time.tv_usec)
            return true;
      }

      return false;
   }

   Bool operator>(const ETime &a)
   {
      if (m_time.tv_sec > a.m_time.tv_sec)
         return true;
      if (m_time.tv_sec == a.m_time.tv_sec)
      {
         if (m_time.tv_usec > a.m_time.tv_usec)
            return true;
      }

      return false;
   }

   Bool operator>=(const ETime &a)
   {
      return !(*this < a);
   }

   Bool operator<=(const ETime &a)
   {
      return !(*this > a);
   }

   ETime operator+(const ETime &a)
   {
      return add(a.m_time);
   }

   ETime operator+(const timeval &t)
   {
      return add(t);
   }

   ETime operator-(const ETime &a)
   {
      return subtract(a.m_time);
   }

   ETime operator-(const timeval &t)
   {
      return subtract(t);
   }

   ETime add(Int days, Int hrs, Int mins, Int secs, Int usecs)
   {
      timeval t;
      t.tv_sec =
          (days * 86400) +
          (hrs * 3600) +
          (mins * 60) +
          secs;
      t.tv_usec = usecs;

      return add(t);
   }

   ETime add(const timeval &t)
   {
      ETime nt(*this);

      nt.m_time.tv_sec += t.tv_sec;
      nt.m_time.tv_usec += t.tv_usec;
      if (nt.m_time.tv_usec >= 1000000)
      {
         nt.m_time.tv_usec -= 1000000;
         nt.m_time.tv_sec++;
      }

      return nt;
   }

   ETime subtract(Int days, Int hrs, Int mins, Int secs, Int usecs)
   {
      timeval t;
      t.tv_sec =
          (days * 86400) +
          (hrs * 3600) +
          (mins * 60) +
          secs;
      t.tv_usec = usecs;

      return subtract(t);
   }

   ETime subtract(const timeval &t)
   {
      ETime nt(*this);

      nt.m_time.tv_sec -= t.tv_sec;
      nt.m_time.tv_usec -= t.tv_usec;
      if (nt.m_time.tv_usec < 0)
      {
         nt.m_time.tv_usec += 1000000;
         nt.m_time.tv_sec--;
      }

      return nt;
   }

   const timeval &getTimeVal()
   {
      return m_time;
   }

   ETime &operator=(const timeval &a)
   {
      set(a);
      return *this;
   }

   ETime &operator=(LongLong ms)
   {
      set(ms);
      return *this;
   }

   void set(const timeval &a)
   {
      m_time.tv_sec = a.tv_sec;
      m_time.tv_usec = a.tv_usec;
   }

   void set(LongLong ms)
   {
      m_time.tv_sec = ms / 1000;
      m_time.tv_usec = (ms % 1000) * 1000;
   }

   LongLong getCassandraTimestmap()
   {
      return m_time.tv_sec * 1000 + (m_time.tv_usec / 1000);
   }

   void getNTPTime(ntp_time_t &ntp) const;
   void setNTPTime(const ntp_time_t &ntp);

   Bool isValid() { return m_time.tv_sec != 0 || m_time.tv_usec != 0; }

   Int year();
   Int month();
   Int day();
   Int hour();
   Int minute();
   Int second();

   static ETime Now();
   void Format(EString &dest, cpStr fmt, Bool local);
   void Format(pStr dest, Int maxsize, cpStr fmt, Bool local);
   Bool ParseDateTime(cpStr pszDate, Bool isLocal = true);

private:
   timeval m_time;
};

#endif // #define __etime_h_included
