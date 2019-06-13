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

#if defined(FT_WINDOWS)
fttime_t FTTimerElapsed::_frequency =  - 1;
#elif defined(FT_GCC)
#elif defined(FT_SOLARIS)
#else
#error "Unrecoginzed platform"
#endif

FTTimerElapsed &FTTimerElapsed::operator = (FTTimerElapsed &a)
{
    _time = a._time;
    _endtime = a._endtime;
    return  *this;
}

FTTimerElapsed &FTTimerElapsed::operator = (fttime_t t)
{
    _time = t;
    _endtime = -1;
    return  *this;
}

FTTimerElapsed::FTTimerElapsed()
{
    Start();

#if defined(FT_WINDOWS)
    if (_frequency ==  - 1)
        QueryPerformanceFrequency((LARGE_INTEGER*) &_frequency);
#elif defined(FT_GCC)
#elif defined(FT_SOLARIS)
#else
#error "Unrecoginzed platform"
#endif
}

FTTimerElapsed::FTTimerElapsed(FTTimerElapsed &a)
{
#if defined(FT_WINDOWS)
    _endtime =  - 1;
    _time = a._time;
#elif defined(FT_GCC)
    _endtime =  - 1;
    _time = a._time;
#elif defined(FT_SOLARIS)
    _endtime =  - 1;
    _time = a._time;
#else
#error "Unrecoginzed platform"
#endif
}

FTTimerElapsed::FTTimerElapsed(fttime_t t)
{
#if defined(FT_WINDOWS)
    _endtime =  - 1;
    _time = t;
#elif defined(FT_GCC)
    _endtime =  - 1;
    _time = t;
#elif defined(FT_SOLARIS)
    _endtime =  - 1;
    _time = t;
#else
#error "Unrecoginzed platform"
#endif
}

FTTimerElapsed::~FTTimerElapsed(){}

void FTTimerElapsed::Start()
{
#if defined(FT_WINDOWS)
    QueryPerformanceCounter((LARGE_INTEGER*) &_time);
    _endtime =  - 1;
#elif defined(FT_GCC)
    struct timespec ts;
    if (clock_gettime(CLOCK_REALTIME, &ts))
        _time = 0;
    else
        _time = (((fttime_t)ts.tv_sec) * 1000000000) + ((fttime_t)ts.tv_nsec);
    _endtime =  - 1;
#elif defined(FT_SOLARIS)
    _time = gethrtime();
    _endtime =  - 1;
#else
#error "Unrecoginzed platform"
#endif
}

void FTTimerElapsed::Stop()
{
#if defined(FT_WINDOWS)
    LARGE_INTEGER liDiff;
    QueryPerformanceCounter(&liDiff);
    _endtime = liDiff.QuadPart - _time;
#elif defined(FT_GCC)
    struct timespec ts;
    fttime_t t;
    if (clock_gettime(CLOCK_REALTIME, &ts))
        t = 0;
    else
        t = (((fttime_t)ts.tv_sec) * 1000000000) + ((fttime_t)ts.tv_nsec);

    _endtime = t - _time;
#elif defined(FT_SOLARIS)
    _endtime = gethrtime() - _time;
#else
#error "Unrecoginzed platform"
#endif
}

void FTTimerElapsed::Set(fttime_t a)
{
#if defined(FT_WINDOWS)
    _time = a;
    _endtime =  - 1;
#elif defined(FT_GCC)
    _time = a;
    _endtime =  - 1;
#elif defined(FT_SOLARIS)
    _time = a;
    _endtime =  - 1;
#else
#error "Unrecoginzed platform"
#endif
}

fttime_t FTTimerElapsed::MilliSeconds(Bool bRestart)
{
#if defined(FT_WINDOWS)
    if (_endtime ==  - 1)
    {
        LARGE_INTEGER liDiff;
        QueryPerformanceCounter(&liDiff);
        fttime_t i64Result = ((liDiff.QuadPart - _time) *1000) / _frequency;
        if (bRestart)
            _time = liDiff.QuadPart;
        return i64Result;
    }

    return (_endtime *1000) / _frequency;
#elif defined(FT_GCC)
    if (_endtime ==  - 1)
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
#elif defined(FT_SOLARIS)
    if (_endtime ==  - 1)
    {
        fttime_t t = gethrtime();
        fttime_t r = t - _time;
        if (bRestart)
            _time = t;
        return r / 1000000;
    }

    return _endtime / 1000000;
#else
#error "Unrecoginzed platform"
#endif
}

fttime_t FTTimerElapsed::MicroSeconds(Bool bRestart)
{
#if defined(FT_WINDOWS)
    if (_endtime ==  - 1)
    {
        LARGE_INTEGER liDiff;
        QueryPerformanceCounter(&liDiff);
        fttime_t i64Result = ((liDiff.QuadPart - _time) *1000000) / _frequency;
        if (bRestart)
            _time = liDiff.QuadPart;
        return i64Result;
    }

    return (_endtime *1000000) / _frequency;
#elif defined(FT_GCC)
    if (_endtime ==  - 1)
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
#elif defined(FT_SOLARIS)
    if (_endtime ==  - 1)
    {
        fttime_t t = gethrtime();
        fttime_t r = t - _time;
        if (bRestart)
            _time = t;
        return r / 1000;
    }

    return _endtime / 1000;
#else
#error "Unrecoginzed platform"
#endif
}
