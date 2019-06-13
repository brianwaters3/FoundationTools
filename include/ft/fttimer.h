////////////////////////////////////////////////////
// (c) 2009 Brian Waters (bdwaters@sbcglobal.net) //
////////////////////////////////////////////////////
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

    FTTimerElapsed &operator = (FTTimerElapsed &a);
    FTTimerElapsed &operator = (fttime_t t);

    operator fttime_t() { return _time; }

private:
    fttime_t _time;
    fttime_t _endtime;
#if defined(FT_WINDOWS)
    static fttime_t _frequency;
#elif defined(FT_GCC)
#elif defined(FT_SOLARIS)
#else
#error "Unrecoginzed platform"
#endif
};

#endif // #define __fttimer_h_included
