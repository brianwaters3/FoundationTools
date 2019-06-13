////////////////////////////////////////////////////
// (c) 2009 Brian Waters (bdwaters@sbcglobal.net) //
////////////////////////////////////////////////////
#ifndef __ftstatic_h_included
#define __ftstatic_h_included

#include "ftbase.h"
#include "ftgetopt.h"

enum {
    STATIC_INIT_TYPE_SHARED_OBJECT_MANAGER,
    STATIC_INIT_TYPE_PRIORITY,
    STATIC_INIT_TYPE_THREADS,            // initialized thread info

    STATIC_INIT_TYPE_STRING_MANAGER,
    STATIC_INIT_TYPE_FILE_MANAGER,
    STATIC_INIT_TYPE_DLLS,
    STATIC_INIT_TYPE_BEFORE_OTHER,
    STATIC_INIT_TYPE_OTHER,
    STATIC_INIT_TYPE_AFTER_ALL			//For anything that relies upon others
};

class FTStatic
{
public:
    static Void Initialize(FTGetOpt& opt);
    static Void UnInitialize();

    virtual Int getInitType() { return STATIC_INIT_TYPE_OTHER; }

    virtual Void init(FTGetOpt& opt)   {}
    virtual Void uninit() {}

    FTStatic();
    virtual ~FTStatic();

protected:
    static FTStatic *m_pFirst; 
    FTStatic *m_pNext;
};

#endif // #define __ftstatic_h_included
