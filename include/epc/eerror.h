/*
* Copyright (c) 2009-2019 Brian Waters
* Copyright (c) 2019 Sprint
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

#ifndef __eerror_h_included
#define __eerror_h_included

#include <exception>

#include "estring.h"

struct EErrorMapEntry
{
   Long m_nError;
   cpStr m_pszError;
};

#define DECLARE_ERR_MAP(name) EErrorMapEntry name[];
#define BGN_ERR_MAP(name) EErrorMapEntry name[] = {
#define ERR_MAP_ENTRY(id, err) {id, err},
#define END_ERR_MAP() \
   {                  \
      0, NULL         \
   }                  \
   }                  \
   ;

#define DECLARE_ERROR(e)                  \
   class e : public EError               \
   {                                      \
      virtual const cpStr Name() const { return #e; } \
   }
#define DECLARE_ERROR_ADVANCED(__e__)         \
   class __e__ : public EError               \
   {                                      \
   public:                                \
      __e__();                                \
      virtual const cpStr Name() const { return #__e__; } \
   }
#define DECLARE_ERROR_ADVANCED2(__e__)        \
   class __e__ : public EError               \
   {                                      \
   public:                                \
      __e__(Int err);                         \
      virtual const cpStr Name() const { return #__e__; } \
   }
#define DECLARE_ERROR_ADVANCED3(__e__)        \
   class __e__ : public EError               \
   {                                      \
   public:                                \
      __e__(Int err, cpChar msg);             \
      virtual const cpStr Name() const { return #__e__; } \
   }
#define DECLARE_ERROR_ADVANCED4(__e__)        \
   class __e__ : public EError               \
   {                                      \
   public:                                \
      __e__(cpChar msg);                      \
      virtual const cpStr Name() const { return #__e__; } \
   }

class EError : public std::exception
{
public:
   enum Severity
   {
      Info,
      Warning,
      Error
   };
   Dword m_dwError;
   Severity m_eSeverity;

public:
   EError()
   {
      m_eSeverity = Info;
      m_dwError = 0;
   }
   EError(Severity eSeverity)
   {
      m_eSeverity = eSeverity;
      m_dwError = 0;
   }
   EError(Severity eSeverity, cpStr pszText)
   {
      m_eSeverity = eSeverity;
      m_dwError = 0;
      setText(pszText);
   }
   EError(const EError &val)
   {
      copy(val);
   }
   EError &operator=(const EError &val)
   {
      return copy(val);
   }
   operator cpStr() { return m_str.c_str(); }
   EError &copy(const EError &val)
   {
      m_str.assign(val.m_str);
      m_dwError = val.m_dwError;
      m_eSeverity = val.m_eSeverity;
      return *this;
   }

   virtual const cpStr Name() const
   {
      return "EError";
   }
   Void clear()
   {
      clear();
   }
   cpStr getText()
   {
      return *this;
   }
   Void setText(cpStr pszText)
   {
      m_str.assign(pszText);
   }
   Void setTextf(cpStr pszFormat, ...);
   Void appendText(cpStr pszText)
   {
      m_str.append(pszText);
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
   Void setLastOsError(Dword dwError = -1);
   Void appendLastOsError(Dword dwError = -1);

   static cpStr getError(Int nError, EErrorMapEntry *pErrors);

   virtual const char* what() const throw ()
   {
      return m_str.c_str();
   }

protected:
   static cpStr m_pszSeverity[];

private:
   EString m_str;
};

#endif // #define __eerror_h_included
