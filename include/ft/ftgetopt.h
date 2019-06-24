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

#ifndef __ftgetopt_h_included
#define __ftgetopt_h_included

#include "fterror.h"
#include "ftstring.h"

class FTGetOpt
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
      FTString shortName;
      FTString longName;
      ArgType argType;
      DataType dataType;
   };

   FTGetOpt();
   ~FTGetOpt();

   Void setPrefix(const FTString &path) { setPrefix(path.c_str()); }
   Void setPrefix(cpStr path) { m_prefix = path; }

   Void loadCmdLine(Int argc, pStr *argv, const FTGetOpt::Option *options);
   Void loadFile(cpStr filename);

   Long getCmdLine(cpStr path, Long def) const;
   LongLong getCmdLine(cpStr path, LongLong def) const;
   ULong getCmdLine(cpStr path, ULong def) const;
   ULongLong getCmdLine(cpStr path, ULongLong def) const;
   Double getCmdLine(cpStr path, Double def) const;
   cpStr getCmdLine(cpStr path, cpStr def) const;
   Bool getCmdLine(cpStr path, Bool def) const;

   std::vector<FTString> getCmdLineArgs() const;

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
   const FTGetOpt::Option *findOption(cpStr name, const FTGetOpt::Option *options);

   pVoid m_json;
   FTString m_prefix;
};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

class FTGetOptError_MissingRequiredArgument : public FTError
{
public:
   FTGetOptError_MissingRequiredArgument(cpStr pszFile);
   virtual cpStr Name() { return "FTGetOptError_MissingRequiredArgument"; }
};

class FTGetOptError_UnsupportedArgType : public FTError
{
public:
   FTGetOptError_UnsupportedArgType(FTGetOpt::ArgType argType);
   virtual cpStr Name() { return "FTGetOptError_UnsupportedArgType"; }
};

class FTGetOptError_UnsupportedDataType : public FTError
{
public:
   FTGetOptError_UnsupportedDataType(FTGetOpt::DataType argType);
   virtual cpStr Name() { return "FTGetOptError_UnsupportedDataType"; }
};

class FTGetOptError_UnsupportedBooleanValue : public FTError
{
public:
   FTGetOptError_UnsupportedBooleanValue(cpStr val);
   virtual cpStr Name() { return "FTGetOptError_UnsupportedBooleanValue"; }
};

class FTGetOptError_FileParsing : public FTError
{
public:
   FTGetOptError_FileParsing(cpStr val);
   virtual cpStr Name() { return "FTGetOptError_FileParsing"; }
};

#endif // #define __ftgetopt_h_included
