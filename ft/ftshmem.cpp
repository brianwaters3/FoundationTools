/////////////////////////////////////////////////////////////////////////
// (c) 2009 Brian Waters (bdwaters@sbcglobal.net) All rights reserved. //
/////////////////////////////////////////////////////////////////////////
#include "ftshmem.h"
#include "ftinternal.h"

#if defined(FT_WINDOWS)
#elif defined(FT_GCC)
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#elif defined(FT_SOLARIS)
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#else
#error "Unrecoginzed platform"
#endif

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

FTSharedMemoryError_UnableToCreate::FTSharedMemoryError_UnableToCreate(cpStr msg)
{
    setSevere();
    setTextf("FTSharedMemoryError_UnableToCreate: Error creating shared memory for [%s] - ", msg);
    appendLastOsError();
}

FTSharedMemoryError_UnableToMap::FTSharedMemoryError_UnableToMap()
{
    setSevere();
    setText("Error mapping shared memory ");
    appendLastOsError();
}

FTSharedMemoryError_UnableToCreateKeyFile::FTSharedMemoryError_UnableToCreateKeyFile(cpStr pszFile)
{
    setSevere();
    setTextf("Error creating shared memory key file [%s] ", pszFile);
    appendLastOsError();
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

FTSharedMemory::FTSharedMemory()
    : m_pShMem(NULL), m_pData(NULL), m_pCtrl(NULL)
#if defined(FT_WINDOWS)
, m_handle(INVALID_HANDLE_VALUE)
#elif defined(FT_GCC)
, m_shmid( - 1)
#elif defined(FT_SOLARIS)
, m_shmid( - 1)
#else
#error "Unrecoginzed platform"
#endif
{
}

FTSharedMemory::FTSharedMemory(cpStr file, Int id, Int size)
    : m_pShMem(NULL), m_pData(NULL), m_pCtrl(NULL)
#if defined(FT_WINDOWS)
, m_handle(INVALID_HANDLE_VALUE)
#elif defined(FT_GCC)
, m_shmid( - 1)
#elif defined(FT_SOLARIS)
, m_shmid( - 1)
#else
#error "Unrecoginzed platform"
#endif
{
    init(file, id, size);
}

FTSharedMemory::~FTSharedMemory()
{
    Bool bDestroy = False;

    if (m_pCtrl)
    {
        _FTMutexLock l(getMutex());
        m_pCtrl->s_usageCnt--;

        if (m_pCtrl->s_usageCnt == 0)
        {
            onDestroy();
            getMutex().destroy();
            bDestroy = True;
        }
    }

#if defined(FT_WINDOWS)
    if (m_pShMem)
    {
        UnmapViewOfFile(m_pShMem);
        m_pShMem = NULL;
    }

    if (m_handle != NULL && m_handle != INVALID_HANDLE_VALUE)
    {
        CloseHandle(m_handle);
        m_handle = INVALID_HANDLE_VALUE;
    }
#elif defined(FT_GCC)
    if (m_pShMem)
    {
        shmdt((char*)m_pShMem);
        m_pShMem = NULL;
    }

    if (m_shmid != -1 && bDestroy)
    {
        shmctl(m_shmid, IPC_RMID, NULL);
        m_shmid =  - 1;
    }
#elif defined(FT_SOLARIS)
    if (m_pShMem)
    {
        shmdt((char*)m_pShMem);
        m_pShMem = NULL;
    }

    if (m_shmid != -1 && bDestroy)
    {
        shmctl(m_shmid, IPC_RMID, NULL);
        m_shmid =  - 1;
    }
#else
#error "Unrecoginzed platform"
#endif
}

Void FTSharedMemory::init(cpStr file, Int id, Int size)
{
    FTLOGFUNC("FTSharedMemory::init");

    longinteger_t liSize;

    liSize.quadPart = sizeof(ftshmemctrl_t) + size;

    // create the object names
    ft_sprintf_s(m_szShMem, sizeof(m_szShMem), "shmem_%s_%d", file, id);
    ft_sprintf_s(m_szMutex, sizeof(m_szMutex), "shmem_mutex_%s_%d", file, id);

#if defined(FT_WINDOWS)

    // create the file mapping
    m_handle = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, liSize.li.highPart, liSize.li.lowPart, m_szShMem);
    if (m_handle == NULL)
        throw new FTSharedMemoryError_UnableToCreate("");

    Bool bAlreadyExists = (GetLastError() == ERROR_ALREADY_EXISTS);

    // map the shared memory to this process space (get a pointer)
    m_pShMem = MapViewOfFile(m_handle, FILE_MAP_ALL_ACCESS, 0, 0, 0);
    if (m_pShMem == NULL)
        throw new FTSharedMemoryError_UnableToMap();

    FTLOGINFO(FTLOG_SHAREDMEMORY, "Name [%s], Size %d, Exists %s", m_szShMem, size, bAlreadyExists ? "true" : "false");

#elif defined(FT_GCC)
    Char szFile[FT_FILENAME_MAX];

    snprintf(szFile, sizeof(szFile), "%s/%s", P_tmpdir, m_szShMem);

    // create the key file
    if (access(szFile, F_OK) ==  - 1)
    {
        // the file does not exist, so create it
        int fp = open(szFile, O_CREAT | O_RDONLY, S_IRUSR);
        if (fp ==  - 1)
            throw new FTSharedMemoryError_UnableToCreateKeyFile(szFile);
        close(fp);
    }

    // create the key
    m_key = ftok(szFile, id);

    FTLOGINFO(FTLOG_SHAREDMEMORY, "File [%s], Key %0x%x, Size %d", szFile, m_key, size);

    // get the shared memory handle
    m_shmid = shmget(m_key, liSize.li.lowPart, 0666 | IPC_CREAT);
    if (m_shmid ==  - 1)
    {
        FTString s;
        s.format("%s - %ld", szFile, (long)m_key);
        throw new FTSharedMemoryError_UnableToCreate(s);
    }

    // attach
    m_pShMem = shmat(m_shmid, NULL, 0);
    if (m_pShMem == (pVoid)( - 1))
    {
        m_pShMem = NULL;
        throw new FTSharedMemoryError_UnableToMap();
    }

#elif defined(FT_SOLARIS)
    key_t key;
    Char szFile[FT_FILENAME_MAX];

    snprintf(szFile, sizeof(szFile), "%s%s", P_tmpdir, m_szShMem);

    // create the key file
    if (access(szFile, F_OK) ==  - 1)
    {
        // the file does not exist, so create it
        int fp = open(szFile, O_CREAT | O_RDONLY, S_IRUSR);
        if (fp ==  - 1)
            throw new FTSharedMemoryError_UnableToCreateKeyFile(szFile);
        close(fp);
    }

    // create the key
    key = ftok(szFile, id);

    FTLOGINFO(FTLOG_SHAREDMEMORY, "File [%s], Key %0x%x, Size %d", szFile, key, size);

    // get the shared memory handle
    m_shmid = shmget(key, liSize.li.lowPart, 0666 | IPC_CREAT);
    if (m_shmid ==  - 1)
        throw new FTSharedMemoryError_UnableToCreate();

    // attach
    m_pShMem = shmat(m_shmid, NULL, 0);
    if (m_pShMem == (pVoid)( - 1))
    {
        m_pShMem = NULL;
        throw new FTSharedMemoryError_UnableToMap();
    }

#else
#error "Unrecoginzed platform"
#endif

    // assign the control block and data pointers
    m_pCtrl = (ftshmemctrl_t*)m_pShMem;
    m_pData = (pVoid)((pChar)m_pShMem + sizeof(ftshmemctrl_t));

    // initialize the control structure
    m_pCtrl->s_mutex.init(m_szMutex);

    // lock the control mutex
    _FTMutexLock l(getMutex());

    // increment the usage counter
    m_pCtrl->s_usageCnt++;
}

Int FTSharedMemory::getUsageCount()
{
    if (m_pCtrl == NULL)
        throw new FTSharedMemoryError_NotInitialized();
    return m_pCtrl->s_usageCnt;
}
