////////////////////////////////////////////////////
// (c) 2009 Brian Waters (bdwaters@sbcglobal.net) //
////////////////////////////////////////////////////
#ifndef __fttime_h_included
#define __fttime_h_included

#include "ftbase.h"
#include "ftstring.h"

size_t ftstrftime(char *s, size_t maxsize, const char *format, const struct tm *t, const struct timeval* tv);

struct ntp_time_t
{
    ULong second;
    ULong fraction;
};

class FTTime
{
public:
    FTTime()
    {
        *this = Now();
    }
    FTTime(const FTTime& a)
    {
        *this = a;
    }
    FTTime(Long sec, Long usec)
    {
        m_time.tv_sec = sec;
        m_time.tv_usec = usec;
    }
    FTTime(Int year, Int mon, Int day, Int hour, Int min, Int sec, Bool isLocal);
    ~FTTime()
    {
    }

    FTTime& operator=(const FTTime& a)
    {
        m_time.tv_sec = a.m_time.tv_sec;
        m_time.tv_usec = a.m_time.tv_usec;

        return *this;
    }

    Bool operator==(const FTTime& a)
    {
        return ((m_time.tv_sec == a.m_time.tv_sec) && (m_time.tv_usec == a.m_time.tv_usec));
    }

    Bool operator!=(const FTTime& a)
    {
        return ((m_time.tv_sec != a.m_time.tv_sec) || (m_time.tv_usec != a.m_time.tv_usec));
    }

    Bool operator<(const FTTime& a)
    {
        if (m_time.tv_sec < a.m_time.tv_sec)
            return True;
        if (m_time.tv_sec == a.m_time.tv_sec)
        {
            if (m_time.tv_usec < a.m_time.tv_usec)
                return True;
        }

        return False;
    }

    Bool operator>(const FTTime& a)
    {
        if (m_time.tv_sec > a.m_time.tv_sec)
            return True;
        if (m_time.tv_sec == a.m_time.tv_sec)
        {
            if (m_time.tv_usec > a.m_time.tv_usec)
                return True;
        }

        return False;
    }

    Bool operator>=(const FTTime& a)
    {
        return !(*this < a);
    }

    Bool operator<=(const FTTime& a)
    {
        return !(*this > a);
    }

    FTTime operator+(const FTTime& a)
    {
        return add(a.m_time);
    }

    FTTime operator+(const timeval& t)
    {
        return add(t);
    }

    FTTime operator-(const FTTime& a)
    {
        return subtract(a.m_time);
    }

    FTTime operator-(const timeval& t)
    {
        return subtract(t);
    }

    FTTime add(Long days, Long hrs, Long mins, Long secs, Long usecs)
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

    FTTime add(const timeval& t)
    {
        FTTime nt(*this);

        nt.m_time.tv_sec += t.tv_sec;
        nt.m_time.tv_usec += t.tv_usec;
        if (nt.m_time.tv_usec >= 1000000)
        {
            nt.m_time.tv_usec -= 1000000;
            nt.m_time.tv_sec++;
        }

        return nt;
    }

    FTTime subtract(Long days, Long hrs, Long mins, Long secs, Long usecs)
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

    FTTime subtract(const timeval& t)
    {
        FTTime nt(*this);

        nt.m_time.tv_sec -= t.tv_sec;
        nt.m_time.tv_usec -= t.tv_usec;
        if (nt.m_time.tv_usec < 0)
        {
            nt.m_time.tv_usec += 1000000;
            nt.m_time.tv_sec--;
        }

        return nt;
    }

    const timeval& getTimeVal()
    {
        return m_time;
    }

    Void set(const timeval& a)
    {
        m_time.tv_sec = a.tv_sec;
        m_time.tv_usec = a.tv_usec;
    }

    Void getNTPTime(ntp_time_t &ntp);
    Void setNTPTime(ntp_time_t &ntp);

//    LongLong operator - (const FTTime& a)
//    {
//        return ((m_time.tv_sec - a.m_time.tv_sec) * 1000000) + (m_time.tv_usec - a.m_time.tv_usec);
//    }

    Int year();
    Int month();
    Int day();
    Int hour();
    Int minute();
    Int second();

    static FTTime Now();
    Void Format(FTString& dest, cpChar fmt, Bool local);
    Void Format(pChar dest, Int maxsize, cpChar fmt, Bool local);
    Bool ParseDateTime(cpStr pszDate, Bool isLocal = True);

private:
    timeval m_time;
};

#endif // #define __fttime_h_included
