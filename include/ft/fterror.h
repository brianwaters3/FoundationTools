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

#ifndef __fterror_h_included
#define __fterror_h_included

struct FTErrorMapEntry
{
    Long m_nError;
    cpStr m_pszError;
};

#define DECLARE_ERR_MAP(name) FTErrorMapEntry name [];
#define BGN_ERR_MAP(name) FTErrorMapEntry name [] = {
#define ERR_MAP_ENTRY(id, err) {id, err},
#define END_ERR_MAP() {0, NULL}};

#define DECLARE_ERROR(e) class e : public FTError { virtual cpStr Name() { return #e; } }
#define DECLARE_ERROR_ADVANCED(e) class e : public FTError { public: e(); virtual cpStr Name() { return #e; } }
#define DECLARE_ERROR_ADVANCED2(e) class e : public FTError { public: e(Int err); virtual cpStr Name() { return #e; } }
#define DECLARE_ERROR_ADVANCED3(e) class e : public FTError { public: e(Int err, cpChar msg); virtual cpStr Name() { return #e; } }
#define DECLARE_ERROR_ADVANCED4(e) class e : public FTError { public: e(cpChar msg); virtual cpStr Name() { return #e; } }

class FTError : public stringstream
{
protected:
    static cpStr m_pszSeverity[];

public:
    enum Severity
    {
        Info, Warning, Error
    };
    Dword m_dwError;
    Severity m_eSeverity;

public:
    FTError()
    {
        m_eSeverity = Info;
        m_dwError = 0;
    }
    FTError(Severity eSeverity)
    {
        m_eSeverity = eSeverity;
        m_dwError = 0;
    }
    FTError(Severity eSeverity, cpStr pszText)
    {
        m_eSeverity = eSeverity;
        m_dwError = 0;
        setText(pszText);
    }
    FTError(const FTError &val)
    {
        copy(val);
    }
    FTError &operator = (const FTError &val)
    {
        return copy(val);
    }
    operator cpStr() { return str().c_str(); }
    FTError &copy(const FTError &val)
    {
        str(val.str());
        m_dwError = val.m_dwError;
        m_eSeverity = val.m_eSeverity;
        return  *this;
    }

    virtual cpStr Name()
    {
        return "FTError";
    }
    Void clear()
    {
        str("");
    }
    cpStr getText()
    {
        return *this;
    }
    Void setText(cpStr pszText)
    {
        clear();
        *this << pszText;
    }
    Void setTextf(cpStr pszFormat, ...);
    Void appendText(cpStr pszText)
    {
        *this << pszText;
    }
    Void appendTextf(cpStr pszText, ...);

    Void setSeverity(Severity eSeverity)
    {
        m_eSeverity = eSeverity;
    }
    Void setSevere()
    {
        m_eSeverity = Error;
    }
    Void setWarning()
    {
        m_eSeverity = Warning;
    }
    Void setInfo()
    {
        m_eSeverity = Info;
    }
    Bool isSevere()
    {
        return m_eSeverity == Error;
    }
    Bool isWarning()
    {
        return m_eSeverity == Warning;
    }
    Bool isInfo()
    {
        return m_eSeverity == Info;
    }
    Bool isError()
    {
        return m_eSeverity == Error;
    }
    Bool isErrorOrWarning()
    {
        return m_eSeverity != Info;
    }
    Severity getSeverity()
    {
        return m_eSeverity;
    }
    cpStr getSeverityText()
    {
        return m_pszSeverity[m_eSeverity];
    }
    static cpStr getSeverityText(Severity eSeverity)
    {
        return m_pszSeverity[eSeverity];
    }

    Dword getLastOsError()
    {
        return m_dwError;
    }
    Void setLastOsError(Dword dwError =  - 1);
    Void appendLastOsError(Dword dwError =  - 1);

    static cpStr getError(Int nError, FTErrorMapEntry *pErrors);
#if defined(FT_WINDOWS)
    Void setText(_com_error &e);
    Void appendText(_com_error &e);
#elif defined(FT_GCC)
#elif defined(FT_SOLARIS)
#else
#error "Unrecoginzed platform"
#endif
};

#endif // #define __fterror_h_included
