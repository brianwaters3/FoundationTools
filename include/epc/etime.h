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

/// @file
/// @brief Provides class for manipulating time of day values.

#include <sys/time.h>
#include <stdint.h>

#include "ebase.h"
#include "estring.h"

/// @cond DOXYGEN_EXCLUDE

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

/// @endcond

/// @brief Class for manipulating date and time of day values.
/// @details The format functions use printf like format specifiers. Here are the possible values:
///  Specifier | Output
/// :---------:|---------------------------------------------------------------------
///      A     | Full day of week name
///      a     | Day of week name abbreviation
///      0     | Milliseconds
///      1     | Microseconds
///      B     | Full month name
///    b or h  | Abbreviated month name
///      c     | Date and time, equivalent to "%a %b %e %H:%M:%S %Y"
///      D     | Month, day and year.  Equivalent to "%m/%d/%y"
///      C     | Century number (2 digit)
///      d     | 2 digit month
///      F     | Year, month and day.  Equivalent to "%Y-%m-%d"
///      e     | Day of month
///      H     | 2 digit hour of day (00-23)
///      i     | ISO 8601 date and time. Equivalent to "%Y-%m-%dT%H:%M:%S.%0"
///      I     | 2 digit hour of day (12-11)
///      l     | hour of day (0-23)
///      k     | hour of day (12-11)
///      j     | 3 digit day of year
///      M     | 2 digit minute (00-59)
///      m     | 2 digit month (01-12)
///      p     | AM/PM
///      n     | New line
///      R     | Hour and minute. Equivalent to "%H:%M"
///      r     | Time using AM/PM. Equivalent to "%I:%M:%S %p"
///      S     | Two digit seconds (00-59)
///      s     | Epoch time
///      t     | Tab character
///      T     | Time in 24-hour. Equivalent to "%H:%M:%S"
///      u     | Day of week as decimal.
///      U     | Week number as decimal.
///      V     | ISO week number.
///      G     | Four digit year.
///      g     | Two digit year.
///      v     | Equivalent to "%e-%b-%Y"
///      w     | Day of week as decimal.
///      W     | Two digit week of year.
///      x     | Date without time. Equivalent to "%m/%d/%y"
///      X     | Time without date. Equivalent to "%H:%M:%S"
///      Y     | Four digit year with century.
///      y     | Two digit year without century.
///      Z     | Timezone name.
///      z     | Hour and minute offset.
///      +     | Date/time with timezone. Equivalent to "%a, %d %b %Y %H:%M:%S %z"
///      %     | Literal percent
class ETime
{
public:
   /// @brief Default constructor.  Initializes to current time.
   ETime()
   {
      *this = Now();
   }

   /// @brief Copy constructor.
   ETime(const ETime &a)
   {
      *this = a;
   }

   /// @brief Class constructor.
   /// @param sec the number of seconds since 00:00 Coordinated Universal Time (UTC), January 1, 1970.
   /// @param usec the fractional number of micro seconds.
   ETime(Int sec, Int usec)
   {
      m_time.tv_sec = sec;
      m_time.tv_usec = usec;
   }

   /// @brief Class constructor.
   /// @param ms milliseconds.
   ETime(LongLong ms)
   {
      set(ms);
   }

   /// @brief Class constructor.
   /// @param year the year.
   /// @param mon the month.
   /// @param day the day of the month.
   /// @param hour the hour (24hr).
   /// @param min the minute of the hour.
   /// @param sec the seconds within the minute.
   /// @param isLocal indicates that this value represents a local time.
   ETime(Int year, Int mon, Int day, Int hour, Int min, Int sec, Bool isLocal);

   /// @brief Class destructor.
   ~ETime()
   {
   }

   /// @brief Assignment operator.
   /// @param a the ETime value to copy.
   /// @return a reference to this object.
   ETime &operator=(const ETime &a)
   {
      m_time.tv_sec = a.m_time.tv_sec;
      m_time.tv_usec = a.m_time.tv_usec;

      return *this;
   }
   /// @brief Assignment operator.
   /// @param a the timeval value to copy.
   /// @return a reference to this object.
   ETime &operator=(const timeval &a)
   {
      set(a);
      return *this;
   }
   /// @brief Assignment operator.
   /// @param ms the milliseconds to set this value to.
   /// @return a reference to this object.
   ETime &operator=(LongLong ms)
   {
      set(ms);
      return *this;
   }

   /// @brief Equality operator.
   /// @param a the ETime value to compare to.
   /// @return True indicates that the values are equal, otherwise False.
   Bool operator==(const ETime &a)
   {
      return ((m_time.tv_sec == a.m_time.tv_sec) && (m_time.tv_usec == a.m_time.tv_usec));
   }

   /// @brief Inequality operator.
   /// @param a the ETime value to compare to.
   /// @return True indicates that the values are not equal, otherwise False (values are equal).
   Bool operator!=(const ETime &a)
   {
      return ((m_time.tv_sec != a.m_time.tv_sec) || (m_time.tv_usec != a.m_time.tv_usec));
   }

   /// @brief Less than operator.
   /// @param a the ETime value to compare to.
   /// @return True indicates that this value is less than the provided value, otherwise False.
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

   /// @brief Greater than operator.
   /// @param a the ETime value to compare to.
   /// @return True indicates that this value is greater than the provided value, otherwise False.
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

   /// @brief Greater than or equal to operator.
   /// @param a the ETime value to compare to.
   /// @return True indicates that this value is greater than or equal to the provided value, otherwise False.
   Bool operator>=(const ETime &a)
   {
      return !(*this < a);
   }

   /// @brief Less than or equal to operator.
   /// @param a the ETime value to compare to.
   /// @return True indicates that this value is less than or equal to the provided value, otherwise False.
   Bool operator<=(const ETime &a)
   {
      return !(*this > a);
   }

   /// @brief Addition operator.
   /// @param a the ETime value to add to this value.
   /// @return a reference to this object.
   ETime operator+(const ETime &a)
   {
      return add(a.m_time);
   }

   /// @brief Addition operator.
   /// @param t the timeval value to add to this value.
   /// @return a reference to this object.
   ETime operator+(const timeval &t)
   {
      return add(t);
   }

   /// @brief Subtraction operator.
   /// @param a the ETime value to subtract from this value.
   /// @return a reference to this object.
   ETime operator-(const ETime &a)
   {
      return subtract(a.m_time);
   }

   /// @brief Subtraction operator.
   /// @param t the timeval value to subtract from this value.
   /// @return a reference to this object.
   ETime operator-(const timeval &t)
   {
      return subtract(t);
   }

   /// @brief Adds the specified days, hours minutes seconds and microseconds to the current value.
   /// @param days the number of days to add.
   /// @param hrs the number of hours to add.
   /// @param mins the number of minutes to add.
   /// @param secs the number of seconds to add.
   /// @param usecs the number of microseconds to add.
   /// @return a new ETime object.
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

   /// @brief Adds the timeval amount to the current value.
   /// @param t the timeval amount to add.
   /// @return a new ETime object.
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

   /// @brief Subtracts the specified days, hours minutes seconds and microseconds to the current value.
   /// @param days the number of days to subtract.
   /// @param hrs the number of hours to subtract.
   /// @param mins the number of minutes to subtract.
   /// @param secs the number of seconds to subtract.
   /// @param usecs the number of microseconds to subtract.
   /// @return a new ETime object.
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

   /// @brief Subtracts the timeval amount to the current value.
   /// @param t the timeval amount to subtract.
   /// @return a new ETime object.
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

   /// @brief Retrieves a reference to the timeval structure.
   /// @return a reference to the timeval structure.
   const timeval &getTimeVal()
   {
      return m_time;
   }

   /// @brief Assigns the specified timeval value to this ETime object.
   /// @param a the timeval value to assign.
   /// @return a reference to this object.
   ETime &set(const timeval &a)
   {
      m_time.tv_sec = a.tv_sec;
      m_time.tv_usec = a.tv_usec;
      return *this;
   }

   /// @brief Assigns the specified millisecond value to this ETime object.
   /// @param ms the millisecond value to assign.
   /// @return a reference to this object.
   ETime & set(LongLong ms)
   {
      m_time.tv_sec = ms / 1000;
      m_time.tv_usec = (ms % 1000) * 1000;
      return *this;
   }

   /// @brief Converts this ETime value to one compatible with a Cassandra timestamp.
   /// @return the Cassandra timestamp.
   LongLong getCassandraTimestmap()
   {
      return m_time.tv_sec * 1000 + (m_time.tv_usec / 1000);
   }

   /// @brief Retrieves the NTP time representation of this ETime object.
   /// @param ntp the ntp_time_t object that will contain the result.
   Void getNTPTime(ntp_time_t &ntp) const;
   /// @brief Assigns the time represented by the NTP time to this ETime object.
   /// @param ntp the ntp_time_t object to assign.
   Void setNTPTime(const ntp_time_t &ntp);

   /// @brief Indicates whether this ETime object is valid or not.
   /// @return True indicates this object is valid, otherwise False.
   Bool isValid() { return m_time.tv_sec != 0 || m_time.tv_usec != 0; }

   /// @brief Retrieves the year.
   /// @return the year.
   Int year();
   /// @brief Retrieves the month.
   /// @return the month.
   Int month();
   /// @brief Retrieves the day.
   /// @return the day.
   Int day();
   /// @brief Retrieves the hour.
   /// @return the hour.
   Int hour();
   /// @brief Retrieves the minute.
   /// @return the minute.
   Int minute();
   /// @brief Retrieves the second.
   /// @return the second.
   Int second();

   /// @brief Retrieves the current time.
   /// @return an ETime object.
   static ETime Now();
   /// @brief Formats the date/time value as specified by the format string.
   /// @param dest contains the resulting string.
   /// @param fmt the format string.
   /// @param local indicates if the time/date value is to be converted to the current timezone or not.
   Void Format(EString &dest, cpStr fmt, Bool local);
   /// @brief Formats the date/time value as specified by the format string.
   /// @param dest contains the resulting string.
   /// @param maxsize the maximum length of dest.
   /// @param fmt the format string.
   /// @param local indicates if the time/date value is to be converted to the current timezone or not.
   Void Format(pStr dest, Int maxsize, cpStr fmt, Bool local);
   /// @brief Formats the date/time value as specified by the format string.
   /// @param fmt the format string.
   /// @param local indicates if the time/date value is to be converted to the current timezone or not.
   /// @return a string containing the formatted output.
   EString Format(cpStr fmt, Bool local) { EString s; Format(s,fmt,local); return s; }
   /// @brief Parses the string containing the time and date.
   /// @param pszDate NULL terminated string to parse.
   /// @param isLocal indicates if the time/date string is in local time or not.
   /// @return True indicates success, otherwise False.
   Bool ParseDateTime(cpStr pszDate, Bool isLocal = True);

private:
   timeval m_time;
};

#endif // #define __etime_h_included
