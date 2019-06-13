/////////////////////////////////////////////////////////////////////////
// (c) 2009 Brian Waters (bdwaters@sbcglobal.net) All rights reserved. //
/////////////////////////////////////////////////////////////////////////
#include "../include/ft/fttime.h"

#include <time.h>

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

//#if defined(FT_WINDOWS)
//#define ft_gmtime(a,b) gmtime_s(a,b)
//#define ft_localtime(a,b) localtime_s(a,b)
//#elif defined(FT_SOLARIS)
//#define ft_gmtime(a,b) gmtime_r(b,a)
//#define ft_localtime(a,b) localtime_r(b,a)
//#else
//#error "Unrecoginzed platform"
//#endif

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

static const char *_days[] = { "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday" };
static const char *_days_abbrev[] = { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };
static const char *_months[] = { "January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December" };
static const char *_months_abbrev[] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

#define TM_YEAR_BASE   1900

#define DAYSPERLYEAR   366
#define DAYSPERNYEAR   365
#define DAYSPERWEEK    7

#define LEAPYEAR(year) (!((year) % 4) && (((year) % 100) || !((year) % 400)))

static char *_fmt(const char *format, const struct tm *t, char *pt, const char *ptlim, const struct timeval* tv);
static char *_conv(const int n, const char *format, char *pt, const char *ptlim);
static char *_add(const char *str, char *pt, const char *ptlim);

inline long fttimezone()
{
#if defined(FT_WINDOWS)
    long tz;
    _get_timezone(&tz);
    return tz;
#define _ft_timezone
#elif defined(FT_GCC)
    return timezone;
#elif defined(FT_SOLARIS)
    return timezone;
#else
#error "Unrecoginzed platform"
#endif
}

inline int ftdaylight()
{
#if defined(FT_WINDOWS)
    int dl;
    _get_daylight(&dl);
    return dl;
#define _ft_timezone
#elif defined(FT_GCC)
    return daylight;
#elif defined(FT_SOLARIS)
    return daylight;
#else
#error "Unrecoginzed platform"
#endif
}

size_t ftstrftime(char *s, size_t maxsize, const char *format, const struct tm *t, const struct timeval* tv)
{
  char *p;

  p = _fmt(((format == NULL) ? "%c" : format), t, s, s + maxsize, tv);
  if (p == s + maxsize) return 0;
  *p = '\0';
  return p - s;
}

static char *_fmt(const char *format, const struct tm *t, char *pt, const char *ptlim, const struct timeval* tv)
{
  for ( ; *format; ++format)
  {
    if (*format == '%')
    {
      if (*format == 'E')
        format++; // Alternate Era
      else if (*format == 'O')
        format++; // Alternate numeric symbols

      switch (*++format)
      {
        case '\0':
          --format;
          break;

        case '0': // milliseconds
            pt = _conv(tv->tv_usec / 1000, "%03d", pt, ptlim);
          continue;

        case '1': // microseconds
            pt = _conv(tv->tv_usec, "%06d", pt, ptlim);
            continue;

        case 'A':
          pt = _add((t->tm_wday < 0 || t->tm_wday > 6) ? "?" : _days[t->tm_wday], pt, ptlim);
          continue;

        case 'a':
          pt = _add((t->tm_wday < 0 || t->tm_wday > 6) ? "?" : _days_abbrev[t->tm_wday], pt, ptlim);
          continue;

        case 'B':
          pt = _add((t->tm_mon < 0 || t->tm_mon > 11) ? "?" : _months[t->tm_mon], pt, ptlim);
          continue;

        case 'b':
        case 'h':
          pt = _add((t->tm_mon < 0 || t->tm_mon > 11) ? "?" : _months_abbrev[t->tm_mon], pt, ptlim);
          continue;

        case 'C':
          pt = _conv((t->tm_year + TM_YEAR_BASE) / 100, "%02d", pt, ptlim);
          continue;

        case 'c':
          pt = _fmt("%a %b %e %H:%M:%S %Y", t, pt, ptlim, tv);
          continue;

        case 'D':
          pt = _fmt("%m/%d/%y", t, pt, ptlim, tv);
          continue;

        case 'd':
          pt = _conv(t->tm_mday, "%02d", pt, ptlim);
          continue;

        case 'e':
          pt = _conv(t->tm_mday, "%2d", pt, ptlim);
          continue;

        case 'F':
          pt = _fmt("%Y-%m-%d", t, pt, ptlim, tv);
          continue;

        case 'H':
          pt = _conv(t->tm_hour, "%02d", pt, ptlim);
          continue;

        case 'i':
          pt = _fmt("%Y-%m-%dT%H:%M:%S.%0", t, pt, ptlim, tv);
          continue;

        case 'I':
          pt = _conv((t->tm_hour % 12) ? (t->tm_hour % 12) : 12, "%02d", pt, ptlim);
          continue;

        case 'j':
          pt = _conv(t->tm_yday + 1, "%03d", pt, ptlim);
          continue;

        case 'k':
          pt = _conv(t->tm_hour, "%2d", pt, ptlim);
          continue;

        case 'l':
          pt = _conv((t->tm_hour % 12) ? (t->tm_hour % 12) : 12, "%2d", pt, ptlim);
          continue;

        case 'M':
          pt = _conv(t->tm_min, "%02d", pt, ptlim);
          continue;

        case 'm':
          pt = _conv(t->tm_mon + 1, "%02d", pt, ptlim);
          continue;

        case 'n':
          pt = _add("\n", pt, ptlim);
          continue;

        case 'p':
          pt = _add((t->tm_hour >= 12) ? "pm" : "am", pt, ptlim);
          continue;

        case 'R':
          pt = _fmt("%H:%M", t, pt, ptlim, tv);
          continue;

        case 'r':
          pt = _fmt("%I:%M:%S %p", t, pt, ptlim, tv);
          continue;

        case 'S':
          pt = _conv(t->tm_sec, "%02d", pt, ptlim);
          continue;

        case 's':
        {
          struct tm  tm;
          char buf[32];
          time_t mkt;

          tm = *t;
          mkt = mktime(&tm);
          ft_sprintf_s(buf, sizeof(buf), "%lu", mkt);
          pt = _add(buf, pt, ptlim);
          continue;
        }

        case 'T':
          pt = _fmt("%H:%M:%S", t, pt, ptlim, tv);
          continue;

        case 't':
          pt = _add("\t", pt, ptlim);
          continue;

        case 'U':
          pt = _conv((t->tm_yday + 7 - t->tm_wday) / 7, "%02d", pt, ptlim);
          continue;

        case 'u':
          pt = _conv((t->tm_wday == 0) ? 7 : t->tm_wday, "%d", pt, ptlim);
          continue;

        case 'V':  // ISO 8601 week number
        case 'G':  // ISO 8601 year (four digits)
        case 'g':  // ISO 8601 year (two digits)
        {
          int  year;
          int  yday;
          int  wday;
          int  w;

          year = t->tm_year + TM_YEAR_BASE;
          yday = t->tm_yday;
          wday = t->tm_wday;
          while (1)
          {
            int  len;
            int  bot;
            int  top;

            len = LEAPYEAR(year) ? DAYSPERLYEAR : DAYSPERNYEAR;
            bot = ((yday + 11 - wday) % DAYSPERWEEK) - 3;
            top = bot - (len % DAYSPERWEEK);
            if (top < -3) top += DAYSPERWEEK;
            top += len;
            if (yday >= top)
            {
              ++year;
              w = 1;
              break;
            }
            if (yday >= bot)
            {
              w = 1 + ((yday - bot) / DAYSPERWEEK);
              break;
            }
            --year;
            yday += LEAPYEAR(year) ? DAYSPERLYEAR : DAYSPERNYEAR;
          }
          if (*format == 'V')
            pt = _conv(w, "%02d", pt, ptlim);
          else if (*format == 'g')
            pt = _conv(year % 100, "%02d", pt, ptlim);
          else
            pt = _conv(year, "%04d", pt, ptlim);

          continue;
        }

        case 'v':
          pt = _fmt("%e-%b-%Y", t, pt, ptlim, tv);
          continue;

        case 'W':
          pt = _conv((t->tm_yday + 7 - (t->tm_wday ? (t->tm_wday - 1) : 6)) / 7, "%02d", pt, ptlim);
          continue;

        case 'w':
          pt = _conv(t->tm_wday, "%d", pt, ptlim);
          continue;

        case 'X':
          pt = _fmt("%H:%M:%S", t, pt, ptlim, tv);
          continue;

        case 'x':
          pt = _fmt("%m/%d/%y", t, pt, ptlim, tv);
          continue;

        case 'y':
          pt = _conv((t->tm_year + TM_YEAR_BASE) % 100, "%02d", pt, ptlim);
          continue;

        case 'Y':
          pt = _conv(t->tm_year + TM_YEAR_BASE, "%04d", pt, ptlim);
          continue;

        case 'Z':
          pt = _add("?", pt, ptlim);
          continue;

        case 'z':
        {
          long absoff;
          if (fttimezone() >= 0)
          {
            absoff = fttimezone();
            pt = _add("+", pt, ptlim);
          }
          else
          {
            absoff = fttimezone();
            pt = _add("-", pt, ptlim);
          }
          pt = _conv(absoff / 3600, "%02d", pt, ptlim);
          pt = _conv((absoff % 3600) / 60, "%02d", pt, ptlim);

          continue;
        }

        case '+':
          pt = _fmt("%a, %d %b %Y %H:%M:%S %z", t, pt, ptlim, tv);
          continue;

        case '%':
        default:
          break;
      }
    }

    if (pt == ptlim) break;
    *pt++ = *format;
  }

  return pt;
}

static char *_conv(const int n, const char *format, char *pt, const char *ptlim)
{
  char  buf[32];

  ft_sprintf_s(buf, sizeof(buf), format, n);
  return _add(buf, pt, ptlim);
}

static char *_add(const char *str, char *pt, const char *ptlim)
{
  while (pt < ptlim && (*pt = *str++) != '\0') ++pt;
  return pt;
}


#if defined(FT_WINDOWS)

#if defined(_MSC_VER) || defined(_MSC_EXTENSIONS)
  #define DELTA_EPOCH_IN_MICROSECS  11644473600000000Ui64
#else
  #define DELTA_EPOCH_IN_MICROSECS  11644473600000000ULL
#endif

struct timezone
{
  int  tz_minuteswest; /* minutes W of Greenwich */
  int  tz_dsttime;     /* type of dst correction */
};

int gettimeofday(struct timeval *tv, struct timezone *tz)
{
  FILETIME ft;
  unsigned __int64 tmpres = 0;
  static int tzflag;

  if (NULL != tv)
  {
    GetSystemTimeAsFileTime(&ft);

    tmpres |= ft.dwHighDateTime;
    tmpres <<= 32;
    tmpres |= ft.dwLowDateTime;

    /*converting file time to unix epoch*/
    tmpres /= 10;  /*convert into microseconds*/
    tmpres -= DELTA_EPOCH_IN_MICROSECS;
    tv->tv_sec = (long)(tmpres / 1000000UL);
    tv->tv_usec = (long)(tmpres % 1000000UL);
  }

  if (NULL != tz)
  {
    if (!tzflag)
    {
      _tzset();
      tzflag++;
    }
    tz->tz_minuteswest = fttimezone() / 60;
    tz->tz_dsttime = ftdaylight();
  }

  return 0;
}

#elif defined(FT_GCC)
#elif defined(FT_SOLARIS)
#else
#error "Unrecoginzed platform"
#endif


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

#if defined(FT_WINDOWS)
#elif defined(FT_GCC)
time_t timegm(struct tm *t)
{
    time_t tl, tb;
    struct tm *tg;

    tl = mktime (t);
    if (tl == -1)
    {
        t->tm_hour--;
        tl = mktime (t);
        if (tl == -1)
            return -1; /* can't deal with output from strptime */
        tl += 3600;
    }
    tg = gmtime (&tl);
    tg->tm_isdst = 0;
    tb = mktime (tg);
    if (tb == -1)
    {
        tg->tm_hour--;
        tb = mktime (tg);
        if (tb == -1)
            return -1; /* can't deal with output from gmtime */
        tb += 3600;
    }
    return (tl - (tb - tl));
}
#elif defined(FT_SOLARIS)
time_t timegm(struct tm *t)
{
    time_t tl, tb;
    struct tm *tg;

    tl = mktime (t);
    if (tl == -1)
    {
        t->tm_hour--;
        tl = mktime (t);
        if (tl == -1)
            return -1; /* can't deal with output from strptime */
        tl += 3600;
    }
    tg = gmtime (&tl);
    tg->tm_isdst = 0;
    tb = mktime (tg);
    if (tb == -1)
    {
        tg->tm_hour--;
        tb = mktime (tg);
        if (tb == -1)
            return -1; /* can't deal with output from gmtime */
        tb += 3600;
    }
    return (tl - (tb - tl));
}
#else
#error "Unrecoginzed platform"
#endif

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

/* Date string parsing */
#define DP_TIMESEP 0x01 /* Time separator ( _must_ remain 0x1, used as a bitmask) */
#define DP_DATESEP 0x02 /* Date separator */
#define DP_MONTH   0x04 /* Month name */
#define DP_AM      0x08 /* AM */
#define DP_PM      0x10 /* PM */

typedef struct tagDATEPARSE
{
    Dword dwCount;      /* Number of fields found so far (maximum 6) */
    Dword dwParseFlags; /* Global parse flags (DP_ Flags above) */
    Dword dwFlags[7];   /* Flags for each field */
    Dword dwValues[7];  /* Value of each field */
} DATEPARSE;

#define TIMEFLAG(i) ((dp.dwFlags[i] & DP_TIMESEP) << i)

#define IsLeapYear(y) (((y % 4) == 0) && (((y % 100) != 0) || ((y % 400) == 0)))

/* Determine if a day is valid in a given month of a given year */
static Bool VARIANT_IsValidMonthDay(Dword day, Dword month, Dword year)
{
  static const UChar days[] = { 0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

  if (day && month && month < 13)
  {
    if (day <= days[month] || (month == 2 && day == 29 && IsLeapYear(year)))
      return True;
  }
  return False;
}

/* Possible orders for 3 numbers making up a date */
#define ORDER_MDY 0x01
#define ORDER_YMD 0x02
#define ORDER_YDM 0x04
#define ORDER_DMY 0x08
#define ORDER_MYD 0x10 /* Synthetic order, used only for funky 2 digit dates */

/* Determine a date for a particular locale, from 3 numbers */
static Bool MakeDate(DATEPARSE *dp, Dword iDate, Dword offset, struct tm *ptm)
{
    Dword dwAllOrders, dwTry, dwCount = 0, v1, v2, v3;

    if (!dp->dwCount)
    {
        v1 = 30; /* Default to (Variant) 0 date part */
        v2 = 12;
        v3 = 1899;
        goto VARIANT_MakeDate_OK;
    }

    v1 = dp->dwValues[offset + 0];
    v2 = dp->dwValues[offset + 1];
    if (dp->dwCount == 2)
    {
        struct tm current;
        time_t t = time(NULL);
        ft_gmtime_s(&current, &t);
        v3 = current.tm_year + 1900;
    }
    else
        v3 = dp->dwValues[offset + 2];

    //TRACE("(%d,%d,%d,%d,%d)\n", v1, v2, v3, iDate, offset);

    /* If one number must be a month (Because a month name was given), then only
    * consider orders with the month in that position.
    * If we took the current year as 'v3', then only allow a year in that position.
    */
    if (dp->dwFlags[offset + 0] & DP_MONTH)
    {
        dwAllOrders = ORDER_MDY;
    }
    else if (dp->dwFlags[offset + 1] & DP_MONTH)
    {
        dwAllOrders = ORDER_DMY;
        if (dp->dwCount > 2)
            dwAllOrders |= ORDER_YMD;
    }
    else if (dp->dwCount > 2 && dp->dwFlags[offset + 2] & DP_MONTH)
    {
        dwAllOrders = ORDER_YDM;
    }
    else
    {
        dwAllOrders = ORDER_MDY|ORDER_DMY;
        if (dp->dwCount > 2)
              dwAllOrders |= (ORDER_YMD|ORDER_YDM);
    }

VARIANT_MakeDate_Start:
    //TRACE("dwAllOrders is 0x%08x\n", dwAllOrders);

    while (dwAllOrders)
    {
        Dword dwTemp;

        if (dwCount == 0)
        {
            /* First: Try the order given by iDate */
            switch (iDate)
            {
                case 0:  dwTry = dwAllOrders & ORDER_MDY; break;
                case 1:  dwTry = dwAllOrders & ORDER_DMY; break;
                default: dwTry = dwAllOrders & ORDER_YMD; break;
            }
        }
        else if (dwCount == 1)
        {
            /* Second: Try all the orders compatible with iDate */
            switch (iDate)
            {
                case 0:  dwTry = dwAllOrders & ~(ORDER_DMY|ORDER_YDM); break;
                case 1:  dwTry = dwAllOrders & ~(ORDER_MDY|ORDER_YMD|ORDER_MYD); break;
                default: dwTry = dwAllOrders & ~(ORDER_DMY|ORDER_YDM); break;
            }
        }
        else
        {
            /* Finally: Try any remaining orders */
            dwTry = dwAllOrders;
        }

        //TRACE("Attempt %d, dwTry is 0x%08x\n", dwCount, dwTry);

        dwCount++;
        if (!dwTry)
            continue;

#define DATE_SWAP(x,y) do { dwTemp = x; x = y; y = dwTemp; } while (0)

        if (dwTry & ORDER_MDY)
        {
            if (VARIANT_IsValidMonthDay(v2,v1,v3))
            {
                DATE_SWAP(v1,v2);
                goto VARIANT_MakeDate_OK;
            }
            dwAllOrders &= ~ORDER_MDY;
        }
        if (dwTry & ORDER_YMD)
        {
            if (VARIANT_IsValidMonthDay(v3,v2,v1))
            {
                DATE_SWAP(v1,v3);
                goto VARIANT_MakeDate_OK;
            }
            dwAllOrders &= ~ORDER_YMD;
        }
        if (dwTry & ORDER_YDM)
        {
            if (VARIANT_IsValidMonthDay(v2,v3,v1))
            {
                DATE_SWAP(v1,v2);
                DATE_SWAP(v2,v3);
                goto VARIANT_MakeDate_OK;
            }
            dwAllOrders &= ~ORDER_YDM;
        }
        if (dwTry & ORDER_DMY)
        {
            if (VARIANT_IsValidMonthDay(v1,v2,v3))
                goto VARIANT_MakeDate_OK;
            dwAllOrders &= ~ORDER_DMY;
        }
        if (dwTry & ORDER_MYD)
        {
            /* Only occurs if we are trying a 2 year date as M/Y not D/M */
            if (VARIANT_IsValidMonthDay(v3,v1,v2))
            {
                DATE_SWAP(v1,v3);
                DATE_SWAP(v2,v3);
                goto VARIANT_MakeDate_OK;
            }
            dwAllOrders &= ~ORDER_MYD;
        }
    }

    if (dp->dwCount == 2)
    {
        /* We couldn't make a date as D/M or M/D, so try M/Y or Y/M */
        v3 = 1; /* 1st of the month */
        dwAllOrders = ORDER_YMD|ORDER_MYD;
        dp->dwCount = 0; /* Don't return to this code path again */
        dwCount = 0;
        goto VARIANT_MakeDate_Start;
    }

    /* No valid dates were able to be constructed */
    return False;

VARIANT_MakeDate_OK:

    /* Check that the time part is ok */
    if (ptm->tm_hour > 23 || ptm->tm_min > 59 || ptm->tm_sec > 59)
        return False; //return DISP_E_TYPEMISMATCH;

    //TRACE("Time %d %d %d\n", st->wHour, st->wMinute, st->wSecond);
    if (ptm->tm_hour < 12 && (dp->dwParseFlags & DP_PM))
        ptm->tm_hour += 12;
    else if (ptm->tm_hour == 12 && (dp->dwParseFlags & DP_AM))
        ptm->tm_hour = 0;
    //TRACE("Time %d %d %d\n", st->wHour, st->wMinute, st->wSecond);

    ptm->tm_mday = v1;
    ptm->tm_mon = v2;
    ptm->tm_year = v3 < 30 ? 2000 + v3 : v3 < 100 ? 1900 + v3 : v3;
    //TRACE("Returning date %d/%d/%d\n", v1, v2, st->wYear);
    return True; //return S_OK;
}

Bool DateFromStr(cpStr strIn, struct timeval* ptvout, Bool isLocal)
{
    static cpStr tokens[] =
    {
        "January",      // LOCALE_SMONTHNAME1
        "February",     // LOCALE_SMONTHNAME2
        "March",        // LOCALE_SMONTHNAME3
        "April",        // LOCALE_SMONTHNAME4
        "May",          // LOCALE_SMONTHNAME5
        "June",         // LOCALE_SMONTHNAME6
        "July",         // LOCALE_SMONTHNAME7
        "August",       // LOCALE_SMONTHNAME8
        "September",    // LOCALE_SMONTHNAME9
        "October",      // LOCALE_SMONTHNAME10
        "November",     // LOCALE_SMONTHNAME11
        "December",     // LOCALE_SMONTHNAME12
        "",             // LOCALE_SMONTHNAME13
        "Jan",          // LOCALE_SABBREVMONTHNAME1
        "Feb",          // LOCALE_SABBREVMONTHNAME2
        "Mar",          // LOCALE_SABBREVMONTHNAME3
        "Apr",          // LOCALE_SABBREVMONTHNAME4
        "May",          // LOCALE_SABBREVMONTHNAME5
        "Jun",          // LOCALE_SABBREVMONTHNAME6
        "Jul",          // LOCALE_SABBREVMONTHNAME7
        "Aug",          // LOCALE_SABBREVMONTHNAME8
        "Sep",          // LOCALE_SABBREVMONTHNAME9
        "Oct",          // LOCALE_SABBREVMONTHNAME10
        "Nov",          // LOCALE_SABBREVMONTHNAME11
        "Dec",          // LOCALE_SABBREVMONTHNAME12
        "",             // LOCALE_SABBREVMONTHNAME13
        "Monday",       // LOCALE_SDAYNAME1
        "Tuesday",      // LOCALE_SDAYNAME2
        "Wednesday",    // LOCALE_SDAYNAME3
        "Thursday",     // LOCALE_SDAYNAME4
        "Friday",       // LOCALE_SDAYNAME5
        "Saturday",     // LOCALE_SDAYNAME6
        "Sunday",       // LOCALE_SDAYNAME7
        "Mon",          // LOCALE_SABBREVDAYNAME1
        "Tue",          // LOCALE_SABBREVDAYNAME2
        "Wed",          // LOCALE_SABBREVDAYNAME3
        "Thu",          // LOCALE_SABBREVDAYNAME4
        "Fri",          // LOCALE_SABBREVDAYNAME5
        "Sat",          // LOCALE_SABBREVDAYNAME6
        "Sun",          // LOCALE_SABBREVDAYNAME7
        "AM",           // LOCALE_S1159
        "PM"            // LOCALE_S2359
    };

    static const Byte ParseDateMonths[] =
    {
        1,2,3,4,5,6,7,8,9,10,11,12,13,
        1,2,3,4,5,6,7,8,9,10,11,12,13
    };

    unsigned int i;

    DATEPARSE dp;
    Dword dwDateSeps = 0, iDate = 0;

    if (!strIn)
        return False;

    ptvout->tv_sec = 0;
    ptvout->tv_usec = 0;

    memset(&dp, 0, sizeof(dp));

    /* Parse the string into our structure */
    while (*strIn)
    {
        if (dp.dwCount >= 7)
            break;

        if (isdigit(*strIn))
        {
            dp.dwValues[dp.dwCount] = strtoul(strIn, (pStr*)&strIn, 10);
            dp.dwCount++;
            strIn--;
        }
        else if (isalpha(*strIn))
        {
            Bool bFound = False;

            for (i = 0; i < sizeof(tokens)/sizeof(tokens[0]); i++)
            {
                size_t dwLen = strlen(tokens[i]);
                if (dwLen && !ft_strnicmp(strIn, tokens[i], dwLen))
                {
                    if (i <= 25)
                    {
                        dp.dwValues[dp.dwCount] = ParseDateMonths[i];
                        dp.dwFlags[dp.dwCount] |= (DP_MONTH|DP_DATESEP);
                        dp.dwCount++;
                    }
                    else if (i > 39)
                    {
                        if (!dp.dwCount || dp.dwParseFlags & (DP_AM|DP_PM))
                            return False; //hRet = DISP_E_TYPEMISMATCH;
                        else
                        {
                            dp.dwFlags[dp.dwCount - 1] |= (i == 40 ? DP_AM : DP_PM);
                            dp.dwParseFlags |= (i == 40 ? DP_AM : DP_PM);
                        }
                    }
                    strIn += (dwLen - 1);
                    bFound = True;
                    break;
                }
            }

            if (!bFound)
            {
                if ((*strIn == 'a' || *strIn == 'A' || *strIn == 'p' || *strIn == 'P') &&
                    (dp.dwCount && !(dp.dwParseFlags & (DP_AM|DP_PM))))
                {
                    /* Special case - 'a' and 'p' are recognised as short for am/pm */
                    if (*strIn == 'a' || *strIn == 'A')
                    {
                        dp.dwFlags[dp.dwCount - 1] |= DP_AM;
                        dp.dwParseFlags |=  DP_AM;
                    }
                    else
                    {
                        dp.dwFlags[dp.dwCount - 1] |= DP_PM;
                        dp.dwParseFlags |=  DP_PM;
                    }
                    strIn++;
                }
                else
                {
                    //TRACE("No matching token for %s\n", debugstr_w(strIn));
                    return False; //hRet = DISP_E_TYPEMISMATCH;
                    //break;
                }
            }
        }
        else if (*strIn == ':' ||  *strIn == '.')
        {
            if (!dp.dwCount || !strIn[1])
                return False; //hRet = DISP_E_TYPEMISMATCH;
            else
                dp.dwFlags[dp.dwCount - 1] |= DP_TIMESEP;
        }
        else if (*strIn == '-' || *strIn == '/')
        {
            dwDateSeps++;
            if (dwDateSeps > 2 || !dp.dwCount || !strIn[1])
                return False; //hRet = DISP_E_TYPEMISMATCH;
            else
                dp.dwFlags[dp.dwCount - 1] |= DP_DATESEP;
        }
        else if (*strIn == ',' || isspace(*strIn))
        {
            if (*strIn == ',' && !strIn[1])
                return False; //hRet = DISP_E_TYPEMISMATCH;
        }
        else
        {
            return False; //hRet = DISP_E_TYPEMISMATCH;
        }
        strIn++;
    }

    if (!dp.dwCount || dp.dwCount > 7 ||
        (dp.dwCount == 1 && !(dp.dwParseFlags & (DP_AM|DP_PM))))
        return False; //hRet = DISP_E_TYPEMISMATCH;

    struct tm t;
    Int milliseconds;
    Dword dwOffset = 0; /* Start of date fields in dp.dwValues */

    t.tm_wday = t.tm_hour = t.tm_min = t.tm_sec = milliseconds = 0;
    //st.wDayOfWeek = t.tm_hour = t.tm_min = t.tm_sec = st.wMilliseconds = 0;

    /* Figure out which numbers correspond to which fields.
     *
     * This switch statement works based on the fact that native interprets any
     * fields that are not joined with a time separator ('.' or ':') as date
     * fields. Thus we construct a value from 0-32 where each set bit indicates
     * a time field. This encapsulates the hundreds of permutations of 2-6 fields.
     * For valid permutations, we set dwOffset to point to the first date field
     * and shorten dp.dwCount by the number of time fields found. The real
     * magic here occurs in MakeDate() above, where we determine what
     * each date number must represent in the context of iDate.
     */
    //TRACE("0x%08x\n", TIMEFLAG(0)|TIMEFLAG(1)|TIMEFLAG(2)|TIMEFLAG(3)|TIMEFLAG(4));

    switch (TIMEFLAG(0)|TIMEFLAG(1)|TIMEFLAG(2)|TIMEFLAG(3)|TIMEFLAG(4))
    {
        case 0x1: /* TT TTDD TTDDD */
            if (dp.dwCount > 3 &&
                ((dp.dwFlags[2] & (DP_AM|DP_PM)) || (dp.dwFlags[3] & (DP_AM|DP_PM)) ||
                (dp.dwFlags[4] & (DP_AM|DP_PM))))
                return False; //hRet = DISP_E_TYPEMISMATCH;
            else if (dp.dwCount != 2 && dp.dwCount != 4 && dp.dwCount != 5)
                return False; //hRet = DISP_E_TYPEMISMATCH;
            t.tm_hour = dp.dwValues[0];
            t.tm_min  = dp.dwValues[1];
            dp.dwCount -= 2;
            dwOffset = 2;
            break;

        case 0x3: /* TTT TTTDD TTTDDD */
            if (dp.dwCount > 4 &&
                ((dp.dwFlags[3] & (DP_AM|DP_PM)) || (dp.dwFlags[4] & (DP_AM|DP_PM)) ||
                (dp.dwFlags[5] & (DP_AM|DP_PM))))
                return False; //hRet = DISP_E_TYPEMISMATCH;
            else if (dp.dwCount != 3 && dp.dwCount != 5 && dp.dwCount != 6)
                return False; //hRet = DISP_E_TYPEMISMATCH;
            t.tm_hour   = dp.dwValues[0];
            t.tm_min = dp.dwValues[1];
            t.tm_sec = dp.dwValues[2];
            milliseconds = dp.dwValues[3];
            dwOffset = 3;
            dp.dwCount -= 3;
            break;

        case 0x4: /* DDTT */
            if (dp.dwCount != 4 ||
                (dp.dwFlags[0] & (DP_AM|DP_PM)) || (dp.dwFlags[1] & (DP_AM|DP_PM)))
                return False; //hRet = DISP_E_TYPEMISMATCH;
            t.tm_hour = dp.dwValues[2];
            t.tm_min  = dp.dwValues[3];
            dp.dwCount -= 2;
            break;

        case 0x0: /* T DD DDD TDDD TDDD */
            if (dp.dwCount == 1 && (dp.dwParseFlags & (DP_AM|DP_PM)))
            {
                t.tm_hour = dp.dwValues[0]; /* T */
                dp.dwCount = 0;
                break;
            }
            else if (dp.dwCount > 4 || (dp.dwCount < 3 && dp.dwParseFlags & (DP_AM|DP_PM)))
            {
                return False; //hRet = DISP_E_TYPEMISMATCH;
            }
            else if (dp.dwCount == 3)
            {
                if (dp.dwFlags[0] & (DP_AM|DP_PM)) /* TDD */
                {
                    dp.dwCount = 2;
                    t.tm_hour = dp.dwValues[0];
                    dwOffset = 1;
                    break;
                }
                if (dp.dwFlags[2] & (DP_AM|DP_PM)) /* DDT */
                {
                    dp.dwCount = 2;
                    t.tm_hour = dp.dwValues[2];
                    break;
                }
                else if (dp.dwParseFlags & (DP_AM|DP_PM))
                    return False; //hRet = DISP_E_TYPEMISMATCH;
            }
            else if (dp.dwCount == 4)
            {
                dp.dwCount = 3;
                if (dp.dwFlags[0] & (DP_AM|DP_PM)) /* TDDD */
                {
                    t.tm_hour = dp.dwValues[0];
                    dwOffset = 1;
                }
                else if (dp.dwFlags[3] & (DP_AM|DP_PM)) /* DDDT */
                {
                    t.tm_hour = dp.dwValues[3];
                }
                else
                    return False; //hRet = DISP_E_TYPEMISMATCH;
                break;
            }
            /* .. fall through .. */

        case 0x8: /* DDDTT */
            if ((dp.dwCount == 2 && (dp.dwParseFlags & (DP_AM|DP_PM))) ||
                (dp.dwCount == 5 && ((dp.dwFlags[0] & (DP_AM|DP_PM)) ||
                (dp.dwFlags[1] & (DP_AM|DP_PM)) || (dp.dwFlags[2] & (DP_AM|DP_PM)))) ||
                dp.dwCount == 4 || dp.dwCount == 6)
                return False; //hRet = DISP_E_TYPEMISMATCH;
            t.tm_hour   = dp.dwValues[3];
            t.tm_min = dp.dwValues[4];
            if (dp.dwCount == 5)
                dp.dwCount -= 2;
            break;

        case 0xC: /* DDTTT */
            if (dp.dwCount != 5 ||
                (dp.dwFlags[0] & (DP_AM|DP_PM)) || (dp.dwFlags[1] & (DP_AM|DP_PM)))
                return False; //hRet = DISP_E_TYPEMISMATCH;
            t.tm_hour   = dp.dwValues[2];
            t.tm_min = dp.dwValues[3];
            t.tm_sec = dp.dwValues[4];
            milliseconds = dp.dwValues[5];
            dp.dwCount -= 3;
            break;

        case 0x18: /* DDDTTT */
            if ((dp.dwFlags[0] & (DP_AM|DP_PM)) || (dp.dwFlags[1] & (DP_AM|DP_PM)) ||
                (dp.dwFlags[2] & (DP_AM|DP_PM)))
                return False; //hRet = DISP_E_TYPEMISMATCH;
            t.tm_hour   = dp.dwValues[3];
            t.tm_min = dp.dwValues[4];
            t.tm_sec = dp.dwValues[5];
            milliseconds = dp.dwValues[6];
            dp.dwCount -= 3;
            break;

        default:
            return False; //hRet = DISP_E_TYPEMISMATCH;
            break;
    }

    if (MakeDate(&dp, iDate, dwOffset, &t))
    {
        if (t.tm_year > 2037)
        {
            t.tm_year = 2037;
            t.tm_mon = 12;
            t.tm_hour = 23;
            t.tm_min = 59;
            t.tm_sec = 59;
            milliseconds = 0;
        }
        t.tm_year -= 1900;
        t.tm_mon--;
        t.tm_wday = 0;
        t.tm_yday = 0;
        if (isLocal)
        {
            t.tm_isdst = -1;
            ptvout->tv_sec = (long)mktime(&t);
        }
        else
        {
            t.tm_isdst = 0;
#if defined(FT_WINDOWS)
            ptvout->tv_sec = (long)_mkgmtime(&t);
#elif defined(FT_GCC)
            ptvout->tv_sec = (long)timegm(&t);
#elif defined(FT_SOLARIS)
            ptvout->tv_sec = (long)timegm(&t);
#else
#error "Unrecognized platform"
#endif
        }
        ptvout->tv_usec = (long)(milliseconds * 1000);
    }

    return False;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

FTTime::FTTime(Int year, Int mon, Int day, Int hour, Int min, Int sec, Bool isLocal)
{
    timeval tv;
    struct tm t;

    memset(&t, 0, sizeof(t));

    t.tm_year = year - 1900;
    t.tm_mon = mon - 1;
    t.tm_mday = day;
    t.tm_hour = hour;
    t.tm_min = min;
    t.tm_sec = sec;

    if (isLocal)
    {
        t.tm_isdst = -1;
        tv.tv_sec = (long)mktime(&t);
    }
    else
    {
        t.tm_isdst = 0;
#if defined(FT_WINDOWS)
        tv.tv_sec = (long)_mkgmtime(&t);
#elif defined(FT_GCC) || defined(FT_SOLARIS)
        tv.tv_sec = (long)timegm(&t);
#else
#error "Unrecognized platform"
#endif
    }

    tv.tv_usec = 0;

    set(tv);
}

Int FTTime::year()
{
	struct tm tms;
	time_t tv_sec  = (time_t)m_time.tv_sec;
    ft_localtime_s(&tms, &tv_sec);
    return tms.tm_year + 1900;
}

Int FTTime::month()
{
	struct tm tms;
	time_t tv_sec  = (time_t)m_time.tv_sec;
    ft_localtime_s(&tms, &tv_sec);
    return tms.tm_mon + 1;
}

Int FTTime::day()
{
	struct tm tms;
	time_t tv_sec  = (time_t)m_time.tv_sec;
    ft_localtime_s(&tms, &tv_sec);
    return tms.tm_mday;
}

Int FTTime::hour()
{
	struct tm tms;
	time_t tv_sec  = (time_t)m_time.tv_sec;
    ft_localtime_s(&tms, &tv_sec);
    return tms.tm_hour;
}

Int FTTime::minute()
{
	struct tm tms;
	time_t tv_sec  = (time_t)m_time.tv_sec;
    ft_localtime_s(&tms, &tv_sec);
    return tms.tm_min;
}

Int FTTime::second()
{
	struct tm tms;
	time_t tv_sec  = (time_t)m_time.tv_sec;
    ft_localtime_s(&tms, &tv_sec);
    return tms.tm_sec;
}

FTTime FTTime::Now()
{
    struct timeval tv;
    struct timezone tz;

    gettimeofday(&tv, &tz);

    return FTTime(tv.tv_sec, tv.tv_usec);
}

Void FTTime::Format(FTString& dest, cpChar fmt, Bool local)
{
    Char buf[2048];
    Format(buf, sizeof(buf), fmt, local);
    dest.assign(buf);
}

Void FTTime::Format(pChar dest, Int maxsize, cpChar fmt, Bool local)
{
    struct tm ts;
    time_t t = m_time.tv_sec;

    if (local)
        ft_localtime_s(&ts, &t);
    else
        ft_gmtime_s(&ts, &t);

    ftstrftime(dest, maxsize, fmt, &ts, &m_time);
}

Bool FTTime::ParseDateTime(cpStr pszDate, Bool isLocal)
{
    return DateFromStr(pszDate, &m_time, isLocal);
}

Void FTTime::getNTPTime(ntp_time_t &ntp)
{
    ntp.second = m_time.tv_sec + 0x83AA7E80;
    ntp.fraction = (ULong)((double)(m_time.tv_usec+1) * (double)(1LL<<32) * 1.0e-6);
}

Void FTTime::setNTPTime(ntp_time_t &ntp)
{
    m_time.tv_sec = ntp.second - 0x83AA7E80;
    m_time.tv_usec = (uint32_t)((double)ntp.fraction * 1.0e6 / (double)(1LL<<32));
}
