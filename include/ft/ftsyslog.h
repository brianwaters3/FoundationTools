////////////////////////////////////////////////////
// (c) 2009 Brian Waters (bdwaters@sbcglobal.net) //
////////////////////////////////////////////////////
#ifndef FTSYSLOG_H_INCLUDED
#define FTSYSLOG_H_INCLUDED

#if defined(FT_WINDOWS)
#elif defined(FT_GCC)
#include <syslog.h>

#elif defined(FT_SOLARIS)
#error "Need to define"
#else
#error "Unrecognized platform"
#endif

class FTSysLog
{
public:
    FTSysLog();
    ~FTSysLog();

    FTString& getIdentity()         { return m_ident; }
    FTString& setIdentity(cpStr v)  { m_ident = v; return getIdentity(); }
    Int getOption()                 { return m_option; }
    Int setOption(Int v)            { m_option = v; return getOption(); }
    Int getFacility()               { return m_facility; }
    Int setFacility(Int v)          { m_facility = v; return getFacility(); }

    void open();
    void close();

    static void syslog(Int priority, cpStr format, va_list ap);

protected:
private:
    FTString    m_ident;
    Int         m_option;
    Int         m_facility;
};

#endif // FTSYSLOG_H_INCLUDED
