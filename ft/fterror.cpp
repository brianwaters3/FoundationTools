/////////////////////////////////////////////////////////////////////////
// (c) 2009 Brian Waters (bdwaters@sbcglobal.net) All rights reserved. //
/////////////////////////////////////////////////////////////////////////
#include "ftbase.h"
#include "fterror.h"

#if defined(FT_WINDOWS)
#elif defined(FT_GCC)
#include <errno.h>
#include <string.h>
#include <stdarg.h>
#elif defined(FT_SOLARIS)
#include <errno.h>
#include <string.h>
#else
#error "Unrecoginzed platform"
#endif

cpStr FTError::m_pszSeverity[] =
{
    "Info", "Warning", "Error"
};

Void FTError::setTextf(cpStr pszFormat, ...)
{
    Char szBuff[2048];
    va_list pArgs;
    va_start(pArgs, pszFormat);
#if defined(FT_WINDOWS)
    _vsnprintf_s(szBuff, sizeof(szBuff), pszFormat, pArgs);
#elif defined(FT_GCC)
    vsnprintf(szBuff, sizeof(szBuff), pszFormat, pArgs);
#elif defined(FT_SOLARIS)
    vsnprintf(szBuff, sizeof(szBuff), pszFormat, pArgs);
#else
#error "Unrecoginzed platform"
#endif
    setText(szBuff);
}

Void FTError::appendTextf(cpStr pszFormat, ...)
{
    Char szBuff[2048];
    va_list pArgs;
    va_start(pArgs, pszFormat);
#if defined(FT_WINDOWS)
    _vsnprintf_s(szBuff, sizeof(szBuff), pszFormat, pArgs);
#elif defined(FT_GCC)
    vsnprintf(szBuff, sizeof(szBuff), pszFormat, pArgs);
#elif defined(FT_SOLARIS)
    vsnprintf(szBuff, sizeof(szBuff), pszFormat, pArgs);
#else
#error "Unrecoginzed platform"
#endif
    appendText(szBuff);
}

Void FTError::setLastOsError(Dword dwError)
{
    clear();
    appendLastOsError(dwError);
}

Void FTError::appendLastOsError(Dword dwError)
{
#if defined(FT_WINDOWS)
    m_dwError = (dwError ==  - 1) ? GetLastError(): dwError;

    LPVOID lpMsgBuf;
    FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, m_dwError, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),  // Default language
    (LPTSTR) &lpMsgBuf, 0, NULL);

    LPSTR pszError = (LPSTR)lpMsgBuf;
    Int nLen = (Int)strlen(pszError);

    // get rid of line feed
    if (nLen > 1)
        pszError[nLen - 2] = 0x00;

    *this << ", error [" << m_dwError << "], [\"" << pszError << "\"]";

    LocalFree((HLOCAL)lpMsgBuf);
#elif defined(FT_GCC)
    m_dwError = (dwError ==  (Dword)-1) ? errno : dwError;
    cpStr pMsg = strerror(m_dwError);
    if (pMsg)
        *this << ", error [" << m_dwError << "], [\"" << pMsg << "\"]";
    else
        *this << ", error [" << m_dwError << "], [" << "Error " << m_dwError << "]";
#elif defined(FT_SOLARIS)
    m_dwError = (dwError ==  - 1) ? errno : dwError;
    cpStr pMsg = strerror(errno);
    if (pMsg)
        *this << ", error [" << m_dwError << "], [\"" << pMsg << "\"]";
    else
        *this << ", error [" << m_dwError << "], [" << "Error " << m_dwError << "]";
#else
#error "Unrecoginzed platform"
#endif
}

#if defined(FT_WINDOWS)
Void FTError::setText(_com_error &e)
{
    clear();
    appendText(e);
}

Void FTError::appendText(_com_error &e)
{
    pStr pszErr = "";
    _bstr_t bstrError = e.Description();
    if (bstrError.length())
        pszErr = (pStr)bstrError;
    *this << "COM ERROR = [" << pszErr << "]. hr=[0x" << setw(8) << setfill('0') << std::hex << e.Error() << "].";
}

#elif defined(FT_GCC)
#elif defined(FT_SOLARIS)
#else
#error "Unrecoginzed platform"
#endif

cpStr FTError::getError(Int nError, FTErrorMapEntry *pErrors)
{
    Int i;
    for (i = 0; pErrors[i].m_pszError != NULL; i++)
    {
        if (pErrors[i].m_nError == nError)
            break;
        //			pszError = pErrors[i].m_pszError;
    }

    return pErrors[i].m_pszError != NULL ? pErrors[i].m_pszError: "Undefined";
}
