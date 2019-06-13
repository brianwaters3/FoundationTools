////////////////////////////////////////////////////
// (c) 2009 Brian Waters (bdwaters@sbcglobal.net) //
////////////////////////////////////////////////////
#ifndef __ftshmem_h_included
#define __ftshmem_h_included

#include "ftsynch.h"

DECLARE_ERROR(FTSharedMemoryError_NotInitialized);
DECLARE_ERROR_ADVANCED4(FTSharedMemoryError_UnableToCreate);
DECLARE_ERROR_ADVANCED(FTSharedMemoryError_UnableToMap);

class FTSharedMemoryError_UnableToCreateKeyFile: public FTError
{
public:
    FTSharedMemoryError_UnableToCreateKeyFile(cpStr pszFile);
    virtual cpStr Name() { return "FTSharedMemoryError_UnableToCreateKeyFile"; }
};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

class FTSharedMemory
{
public:
    FTSharedMemory();
    FTSharedMemory(cpStr file, Int id, Int size);
    ~FTSharedMemory();

    Void init(cpStr file, Int id, Int size);

    pVoid getDataPtr()
    {
        return m_pData;
    }

    virtual Void onDestroy()
    {
    }

    Int getUsageCount();

private:
    typedef struct
    {
        Int s_usageCnt;
        _FTMutex s_mutex;
    } ftshmemctrl_t;

    _FTMutex &getMutex()
    {
        return m_pCtrl->s_mutex;
    }

    Char m_szShMem[FT_FILENAME_MAX + 1];
    Char m_szMutex[FT_FILENAME_MAX + 1];
    pVoid m_pShMem;
    pVoid m_pData;
    ftshmemctrl_t *m_pCtrl;

#if defined(FT_WINDOWS)
    HANDLE m_handle;
#elif defined(FT_GCC)
    Int m_shmid;
    key_t m_key;
#elif defined(FT_SOLARIS)
    Int m_shmid;
#else
#error "Unrecoginzed platform"
#endif
};

#endif // #define __ftshmem_h_included
