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

#ifndef __egetopt_h_included
#define __egetopt_h_included

#include "eerror.h"
#include "estring.h"

class EGetOpt
{
public:
   enum ArgType
   {
      no_argument,
      required_argument,
      optional_argument
   };

   enum DataType
   {
      dtNone,
      dtString,
      dtInt32,
      dtInt64,
      dtUInt32,
      dtUInt64,
      dtDouble,
      dtBool
   };

   struct Option
   {
      EString shortName;
      EString longName;
      ArgType argType;
      DataType dataType;
   };

   EGetOpt();
   ~EGetOpt();

   Void setPrefix(const EString &path) { setPrefix(path.c_str()); }
   Void setPrefix(cpStr path) { m_prefix = path; }

   Void loadCmdLine(Int argc, pStr *argv, const EGetOpt::Option *options);
   Void loadFile(cpStr filename);

   Long getCmdLine(cpStr path, Long def) const;
   LongLong getCmdLine(cpStr path, LongLong def) const;
   ULong getCmdLine(cpStr path, ULong def) const;
   ULongLong getCmdLine(cpStr path, ULongLong def) const;
   Double getCmdLine(cpStr path, Double def) const;
   cpStr getCmdLine(cpStr path, cpStr def) const;
   Bool getCmdLine(cpStr path, Bool def) const;

   std::vector<EString> getCmdLineArgs() const;

   Long get(cpStr path, Long def) const;
   LongLong get(cpStr path, LongLong def) const;
   ULong get(cpStr path, ULong def) const;
   ULongLong get(cpStr path, ULongLong def) const;
   Double get(cpStr path, Double def) const;
   cpStr get(cpStr path, cpStr def) const;
   Bool get(cpStr path, Bool def) const;

   UInt getCount(cpStr path) const;
   Long get(UInt idx, cpStr path, cpStr member, Long def) const;
   ULong get(UInt idx, cpStr path, cpStr member, ULong def) const;
   LongLong get(UInt idx, cpStr path, cpStr member, LongLong def) const;
   ULongLong get(UInt idx, cpStr path, cpStr member, ULongLong def) const;
   Double get(UInt idx, cpStr path, cpStr member, Double def) const;
   cpStr get(UInt idx, cpStr path, cpStr member, cpStr def) const;
   Bool get(UInt idx, cpStr path, cpStr member, Bool def) const;

   Void print() const;

protected:
private:
   const EGetOpt::Option *findOption(cpStr name, const EGetOpt::Option *options);

   pVoid m_json;
   EString m_prefix;
};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

class EGetOptError_MissingRequiredArgument : public EError
{
public:
   EGetOptError_MissingRequiredArgument(cpStr pszFile);
   virtual cpStr Name() { return "EGetOptError_MissingRequiredArgument"; }
};

class EGetOptError_UnsupportedArgType : public EError
{
public:
   EGetOptError_UnsupportedArgType(EGetOpt::ArgType argType);
   virtual cpStr Name() { return "EGetOptError_UnsupportedArgType"; }
};

class EGetOptError_UnsupportedDataType : public EError
{
public:
   EGetOptError_UnsupportedDataType(EGetOpt::DataType argType);
   virtual cpStr Name() { return "EGetOptError_UnsupportedDataType"; }
};

class EGetOptError_UnsupportedBooleanValue : public EError
{
public:
   EGetOptError_UnsupportedBooleanValue(cpStr val);
   virtual cpStr Name() { return "EGetOptError_UnsupportedBooleanValue"; }
};

class EGetOptError_FileParsing : public EError
{
public:
   EGetOptError_FileParsing(cpStr val);
   virtual cpStr Name() { return "EGetOptError_FileParsing"; }
};

#endif // #define __egetopt_h_included
