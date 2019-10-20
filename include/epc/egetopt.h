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

/// @file
/// @brief Manages configuration parameters from a file and the command line.
///
/// @details
/// Contains configuration parameters optained from a JSON formatted
/// configuration file as well as parameters supplied on the command line.
///
class EGetOpt
{
public:
   /// @brief Indicates if an argument associated with a command line argument is required.
   enum ArgType
   {
      /// no argument required
      no_argument,
      /// an argument is required
      required_argument,
      /// the argument is optional *** NOT IMPLEMENTED ***
      optional_argument
   };

   /// @brief The data type of the command line argument.
   enum DataType
   {
      /// no argument
      dtNone,
      /// string
      dtString,
      /// 32-bit integer
      dtInt32,
      /// 64-bit integer
      dtInt64,
      /// 32-bit unsigned integer
      dtUInt32,
      /// 64-bit unsigned integer
      dtUInt64,
      /// 8-byte floating point number
      dtDouble,
      /// boolean
      dtBool
   };

   /// @brief Describes the defined command line arguments.
   struct Option
   {
      /// the short name of the argument e.g.: -a
      EString shortName;
      /// the long name of the argument e.g.: --argument
      EString longName;
      /// indicates if an argument is required or not
      ArgType argType;
      /// the data type of the argument
      DataType dataType;
   };

   /// @brief Class constructor.
   EGetOpt();
   /// @brief Class destructor.
   ~EGetOpt();

   /// @brief Sets the search prefix.
   ///
   /// @param path The prefix path.
   ///
   Void setPrefix(const EString &path) { setPrefix(path.c_str()); }
   /// @brief Sets the search prefix.
   ///
   /// @param path The prefix path.
   ///
   Void setPrefix(cpStr path) { m_prefix = path; }

   /// @brief Parses and loads the command line arguments.
   ///
   /// @param argc The number of command line arguments.
   /// @param argv A pointer to the array of command line string arguments.
   /// @param options The array of supported arguments.
   ///
   /// @throws EGetOptError_UnsupportedDataType
   /// @throws EGetOptError_UnsupportedBooleanValue
   /// @throws EGetOptError_UnsupportedArgType
   ///
   Void loadCmdLine(Int argc, pStr *argv, const EGetOpt::Option *options);
   /// @brief Parses and loads configuration values from the specified JSON file.
   ///
   /// @param filename The name of the JSON file to load.
   ///
   /// @throws EGetOptError_FileParsing
   ///
   Void loadFile(cpStr filename);

   /// @brief Returns the value of the specified command line argument as a 32-bit integer.
   ///
   /// @param path The path of the command line argument.
   /// @param def The value to return if the requested path is not found.
   ///
   Long getCmdLine(cpStr path, Long def) const;
   /// @brief Returns the value of the specified command line argument as a 64-bit integer.
   ///
   /// @param path The path of the command line argument.
   /// @param def The value to return if the requested path is not found.
   ///
   LongLong getCmdLine(cpStr path, LongLong def) const;
   /// @brief Returns the value of the specified command line argument as an unsigned 32-bit integer.
   ///
   /// @param path The path of the command line argument.
   /// @param def The value to return if the requested path is not found.
   ///
   ULong getCmdLine(cpStr path, ULong def) const;
   /// @brief Returns the value of the specified command line argument as an unsigned 64-bit integer.
   ///
   /// @param path The path of the command line argument.
   /// @param def The value to return if the requested path is not found.
   ///
   ULongLong getCmdLine(cpStr path, ULongLong def) const;
   /// @brief Returns the value of the specified command line argument as a double (8-byte floating point).
   ///
   /// @param path The path of the command line argument.
   /// @param def The value to return if the requested path is not found.
   ///
   Double getCmdLine(cpStr path, Double def) const;
   /// @brief Returns the value of the specified command line argument as a string.
   ///
   /// @param path The path of the command line argument.
   /// @param def The value to return if the requested path is not found.
   ///
   cpStr getCmdLine(cpStr path, cpStr def) const;
   /// @brief Returns the value of the specified command line argument as a boolean.
   ///
   /// @param path The path of the command line argument.
   /// @param def The value to return if the requested path is not found.
   ///
   Bool getCmdLine(cpStr path, Bool def) const;

   /// @brief Returns a std::vector containing positional command line arguments.
   std::vector<EString> getCmdLineArgs() const;

   /// @brief Returns a std::vector containing the "raw" string command line arguments.
   std::vector<EString> getCmdLineRaw() const;

   /// @brief Returns the value of the specified configuration value as a 32-bit integer.
   ///
   /// @param path The path of the requested configuration parameter.
   /// @param def The value to return if the requested path is not found.
   ///
   Long get(cpStr path, Long def) const;
   /// @brief Returns the value of the specified configuration value as a 64-bit integer.
   ///
   /// @param path The path of the requested configuration parameter.
   /// @param def The value to return if the requested path is not found.
   ///
   LongLong get(cpStr path, LongLong def) const;
   /// @brief Returns the value of the specified configuration value as an unsigned 32-bit integer.
   ///
   /// @param path The path of the requested configuration parameter.
   /// @param def The value to return if the requested path is not found.
   ///
   ULong get(cpStr path, ULong def) const;
   /// @brief Returns the value of the specified configuration value as a unsigned 64-bit integer.
   ///
   /// @param path The path of the requested configuration parameter.
   /// @param def The value to return if the requested path is not found.
   ///
   ULongLong get(cpStr path, ULongLong def) const;
   /// @brief Returns the value of the specified configuration value as a double (8-byte floating point).
   ///
   /// @param path The path of the requested configuration parameter.
   /// @param def The value to return if the requested path is not found.
   ///
   Double get(cpStr path, Double def) const;
   /// @brief Returns the value of the specified configuration value as a string.
   ///
   /// @param path The path of the requested configuration parameter.
   /// @param def The value to return if the requested path is not found.
   ///
   cpStr get(cpStr path, cpStr def) const;
   /// @brief Returns the value of the specified configuration value as a boolean.
   ///
   /// @param path The path of the requested configuration parameter.
   /// @param def The value to return if the requested path is not found.
   ///
   Bool get(cpStr path, Bool def) const;

   /// @brief returns the number of configuration entries specified in a JSON array.
   ///
   /// @param path The path of the requested configuration parameter.
   ///
   UInt getCount(cpStr path) const;
   /// @brief Returns the value associated with an array.
   ///
   /// @param idx The index associated with the parent JSON object.
   /// @param path The path of the parent object of the requested configuration value.
   /// @param member The member name of the requested configurtion value.
   /// @param def The value to return if the requested path/member is not found.
   /// 
   template<typename T>
   T get(UInt idx, cpStr path, cpStr member, T def) const
   {
      EString mbr;
      mbr.format("%u/%s", idx, member);
      EString pth = combinePath(path, mbr.c_str());
      return get(pth, def);
   }

   /// @brief Prints the current loaded values (command line and file).
   Void print() const;

private:
   EString combinePath(cpStr path1, cpStr path2) const;
   const Option *findOption(cpStr name, const Option *options);

   pVoid m_json;
   EString m_prefix;
};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

/// @cond DOXYGEN_EXCLUDE
class EGetOptError_MissingRequiredArgument : public EError
{
public:
   EGetOptError_MissingRequiredArgument(cpStr pszFile);
   virtual const cpStr Name() const { return "EGetOptError_MissingRequiredArgument"; }
};

class EGetOptError_UnsupportedArgType : public EError
{
public:
   EGetOptError_UnsupportedArgType(EGetOpt::ArgType argType);
   virtual const cpStr Name() const { return "EGetOptError_UnsupportedArgType"; }
};

class EGetOptError_UnsupportedDataType : public EError
{
public:
   EGetOptError_UnsupportedDataType(EGetOpt::DataType argType);
   virtual const cpStr Name() const { return "EGetOptError_UnsupportedDataType"; }
};

class EGetOptError_UnsupportedBooleanValue : public EError
{
public:
   EGetOptError_UnsupportedBooleanValue(cpStr val);
   virtual const cpStr Name() const { return "EGetOptError_UnsupportedBooleanValue"; }
};

class EGetOptError_FileParsing : public EError
{
public:
   EGetOptError_FileParsing(cpStr val);
   virtual const cpStr Name() const { return "EGetOptError_FileParsing"; }
};
/// @endcond

#endif // #define __egetopt_h_included
