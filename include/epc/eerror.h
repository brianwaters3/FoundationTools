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

/// @file
/// @brief Defines base class for exceptions and declaration helper macros.

/// @cond DOXYGEN_EXCLUDE
struct EErrorMapEntry
{
   Long m_nError;
   cpStr m_pszError;
};
/// @endcond

/// Declares the error map in the EError derived object.  The error map is
/// used in conjunction with the static cpStr EError::getError() method to
/// return a description of an application specific error code.
#define DECLARE_ERR_MAP(__name__) EErrorMapEntry __name__[];
/// Starts the definition of the error map in the code.
#define BGN_ERR_MAP(__name__) EErrorMapEntry __name__[] = {
/// Adds an error map entry.
#define ERR_MAP_ENTRY(id, err) {id, err},
/// Ends (closes) the error map definition.
#define END_ERR_MAP() \
   {                  \
      0, NULL         \
   }                  \
   }                  \
   ;

/// Declares exception class derived from EError with no constructor parameters.
#define DECLARE_ERROR(__e__)                                \
   class __e__ : public EError                              \
   {                                                        \
      virtual const cpStr Name() const { return #__e__; }   \
   }
/// Declares exception class derived from EError with no constructor parameters and developer defined constructor body.
#define DECLARE_ERROR_ADVANCED(__e__)                       \
   class __e__ : public EError                              \
   {                                                        \
   public:                                                  \
      __e__();                                              \
      virtual const cpStr Name() const { return #__e__; }   \
   }
/// Declares exception class derived from EError with an Int as a constructor parameter and developer defined constructor body.
#define DECLARE_ERROR_ADVANCED2(__e__)                      \
   class __e__ : public EError                              \
   {                                                        \
   public:                                                  \
      __e__(Int err);                                       \
      virtual const cpStr Name() const { return #__e__; }   \
   }
/// Declares exception class derived from EError with an Int and a const char * as constructor parameters and developer defined constructor body.
#define DECLARE_ERROR_ADVANCED3(__e__)                      \
   class __e__ : public EError                              \
   {                                                        \
   public:                                                  \
      __e__(Int err, cpChar msg);                           \
      virtual const cpStr Name() const { return #__e__; }   \
   }
/// Declares exception class derived from EError with an const char* as a constructor parameter and developer defined constructor body.
#define DECLARE_ERROR_ADVANCED4(__e__)                      \
   class __e__ : public EError                              \
   {                                                        \
   public:                                                  \
      __e__(cpStr msg);                                     \
      virtual const cpStr Name() const { return #__e__; }   \
   }

/// @brief The base class for exceptions derived from std::exception.
class EError : public std::exception
{
public:
   /// @brief Error severity levels.
   enum Severity
   {
      /// Informational
      Info,
      /// Warning
      Warning,
      /// Error
      Error
   };
   /// @brief The unsigned long error code (if appropriate)
   Dword m_dwError;
   /// @brief Represents the severity of the error.
   Severity m_eSeverity;

public:
   /// @brief Default constructor.
   EError()
   {
      m_eSeverity = Info;
      m_dwError = 0;
   }
   /// @brief Additonal constructor.
   /// @param eSeverity the severity level for this error.
   /// @param pszText this optional parameter is the text associated with this error.
   EError(Severity eSeverity, cpStr pszText=NULL)
   {
      m_eSeverity = eSeverity;
      m_dwError = 0;
      if (pszText)
         setText(pszText);
   }
   /// @brief Additonal constructor.
   /// @param eSeverity the severity level for this error.
   /// @param err numeric error code.
   /// @param pszText this optional parameter is the text associated with this error.
   EError(Severity eSeverity, Dword err, cpStr pszText=NULL)
   {
      m_eSeverity = eSeverity;
      m_dwError = err;
      if (pszText)
         setText(pszText);
   }
   /// @brief Additonal constructor.
   /// @param eSeverity the severity level for this error.
   /// @param txt this optional parameter is the text associated with this error.
   EError(Severity eSeverity, const std::string &txt)
   {
      m_eSeverity = eSeverity;
      m_dwError = 0;
      setText(txt.c_str());
   }
   /// @brief Additonal constructor.
   /// @param eSeverity the severity level for this error.
   /// @param err numeric error code.
   /// @param txt this optional parameter is the text associated with this error.
   EError(Severity eSeverity, Dword err, const std::string &txt)
   {
      m_eSeverity = eSeverity;
      m_dwError = err;
      setText(txt.c_str());
   }
   /// @brief Copy constructor.
   EError(const EError &val)
   {
      copy(val);
   }
   /// @brief Assignment operator.
   /// @param val The source object to copy.
   /// @return the current object
   EError &operator=(const EError &val)
   {
      return copy(val);
   }
   /// @brief The const char* extractor.
   /// @return the current description associated with this error object
   operator cpStr() { return m_str.c_str(); }
   /// @brief Copies the contents of the specified object to this object.
   /// @param val The source object to copy.
   /// @return the current object
   EError &copy(const EError &val)
   {
      m_str.assign(val.m_str);
      m_dwError = val.m_dwError;
      m_eSeverity = val.m_eSeverity;
      return *this;
   }

   /// @brief Returns the name of this object.
   /// @return the name of class
   /// @details
   /// This virtual method returns the name of the class.  This method should
   /// be overridden in any derived class and set appropriately.  The
   /// DECLARE_ERROR* macros handle this automatically.
   virtual const cpStr Name() const
   {
      return "EError";
   }
   /// @brief Clears the current contents of this error object.
   Void clear()
   {
      clear();
   }
   /// @brief Returns the text associated with this error.
   /// @return the current description associated with this error object
   cpStr getText()
   {
      return *this;
   }
   /// @brief Sets the text associated with this error.
   /// @param pszText the text for this error
   Void setText(cpStr pszText)
   {
      m_str.assign(pszText);
   }
   /// @brief Sets the text associated with this error.
   /// @param pszFormat A printf style format string.
   /// @param ... Optional parameters to be used as arguments to the format
   ///             string.
   Void setTextf(cpStr pszFormat, ...);
   /// @brief appends the specified text to the current text of this error.
   /// @param pszText The text to append.
   Void appendText(cpStr pszText)
   {
      m_str.append(pszText);
   }
   /// @brief appends the specified text to the current error text using a
   ///        printf style format string and parameters.
   /// @param pszText A printf style format string.
   /// @param ... Optinal parameters to be used as arguments to the format
   ///            string.
   Void appendTextf(cpStr pszText, ...);

   /// @brief Sets the severity level of this error object.
   /// @param eSeverity The new severity level.
   Void setSeverity(Severity eSeverity)
   {
      m_eSeverity = eSeverity;
   }
   /// @brief Sets the severity level of this error object to "Error".
   Void setSevere()
   {
      m_eSeverity = Error;
   }
   /// @brief Sets the severity level of this error object to "Warning".
   Void setWarning()
   {
      m_eSeverity = Warning;
   }
   /// @brief Sets the severity level of this error object to "Info".
   Void setInfo()
   {
      m_eSeverity = Info;
   }
   /// @brief Returns True if the severity is "Error".
   /// @return True - severity is "Error", False - severity is not "Error"
   Bool isSevere()
   {
      return m_eSeverity == Error;
   }
   /// @brief Returns True if the severity is "Warning".
   /// @return True - severity is "Warning", False - severity is not "Warning"
   Bool isWarning()
   {
      return m_eSeverity == Warning;
   }
   /// @brief Returns True if the severity is "Info".
   /// @return True - severity is "Info", False - severity is not "Info"
   Bool isInfo()
   {
      return m_eSeverity == Info;
   }
   /// @brief Returns True if the severity is "Error".
   /// @return True - severity is "Error", False - severity is not "Error"
   Bool isError()
   {
      return m_eSeverity == Error;
   }
   /// @brief Returns True if the severity is "Error" or "Warning".
   /// @return True - is "Error" or "Warning", False - another value
   Bool isErrorOrWarning()
   {
      return m_eSeverity != Info;
   }
   /// @brief Returns the current severity setting of this error object.
   /// @return the current severity value of this error object
   Severity getSeverity()
   {
      return m_eSeverity;
   }
   /// @brief Returns the text based on the current error object's severity.
   /// @return the description of the severity of this error object
   cpStr getSeverityText()
   {
      return m_pszSeverity[m_eSeverity];
   }
   /// @brief Returns the text asociated with the specified severity code.
   /// @param eSeverity The severity code to retrieve the associated description.
   /// @return the description of the supplied severity value
   static cpStr getSeverityText(Severity eSeverity)
   {
      return m_pszSeverity[eSeverity];
   }

   /// @brief Returns the current value of m_dwError.
   /// @return the current error value
   Dword getLastOsError()
   {
      return m_dwError;
   }
   /// @brief Sets the value of m_dwError.
   /// @param dwError The new error value.
   Void setLastOsError(Dword dwError = -1);
   /// @brief Sets the value of m_dwError and appends the text description of
   ///        the error to the current error text.
   Void appendLastOsError(Dword dwError = -1);

   /// @brief Returns a description for the specified application error code.
   /// @param nError The error code to retrieve the description for.
   /// @param pErrors The associated error map that contains the error definitions.
   /// @return the description of the error value
   static cpStr getError(Int nError, EErrorMapEntry *pErrors);

   /// @brief The overloaded definition of the std::exception::what() method.
   /// @return the text message associated with the exception
   virtual const char* what() const throw ()
   {
      return m_str.c_str();
   }

protected:
   /// @cond DOXYGEN_EXCLUDE
   static cpStr m_pszSeverity[];
   /// @endcond

private:
   EString m_str;
};

#endif // #define __eerror_h_included
