/////////////////////////////////////////////////////////////////////////
// (c) 2009 Brian Waters (bdwaters@sbcglobal.net) All rights reserved. //
/////////////////////////////////////////////////////////////////////////
#ifndef __ftautohandle_h_included
#define __ftautohandle_h_included

#include <exception>

struct bad_handle: public std::exception
{
    bad_handle(): std::exception("bad handle"){}
};


class FTAutoHandle
{
    HANDLE _handle;
public:
    FTAutoHandle(): _handle(INVALID_HANDLE_VALUE){}
    explicit FTAutoHandle(HANDLE handle): _handle(handle)
    {
        if (!isvalid())
            throw bad_handle();
    }
    ~FTAutoHandle()
    {
        release();
    }
    Bool isvalid()const
    {
        return (INVALID_HANDLE_VALUE != _handle) && (NULL != _handle);
    }
    Void release()
    {
        if (isvalid())
        {
            CloseHandle(_handle);
            _handle = INVALID_HANDLE_VALUE;
        }
    }
    operator HANDLE()const
    {
        if (!isvalid())
            throw bad_handle();
        return _handle;
    }
    const FTAutoHandle &operator = (HANDLE handle)
    {
        release();
        _handle = handle;
        return  *this;
    }
    const FTAutoHandle &operator = (FTAutoHandle &other)
    {
        release();
        _handle = other._handle;
        other._handle = INVALID_HANDLE_VALUE;
        return  *this;
    }
};

#endif // #define __ftautohandle_h_included
