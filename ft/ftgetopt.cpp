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

#include <iostream>

#include "ft.h"
#include "ftgetopt.h"

#define RAPIDJSON_NAMESPACE ftgetoptrapidjson
#include "rapidjson/filereadstream.h"
#include "rapidjson/document.h"
#include "rapidjson/prettywriter.h"

#define CMDLINE "cmdline"
#define PROGRAM "program"
#define RAW "raw"
#define ARGS "args"
#define CMDLINEARGS "cmdline/args"

#define FILEBUFFER  512

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

FTGetOptError_MissingRequiredArgument::FTGetOptError_MissingRequiredArgument(cpStr arg)
{
    setSevere();
    setTextf("Missing required argument for %s", arg);
}

FTGetOptError_UnsupportedArgType::FTGetOptError_UnsupportedArgType(FTGetOpt::ArgType argType)
{
    FTString at;

    setSevere();
    switch (argType)
    {
        case FTGetOpt::no_argument:   { at = "no_argument"; break; }
        case FTGetOpt::required_argument:   { at = "required_argument"; break; }
        case FTGetOpt::optional_argument:   { at = "optional_argument"; break; }
        default:
        {
            at.format("%d", argType);
            break;
        }
    }
    setTextf("Unsupported option argument type [%s]", at.c_str());
}

FTGetOptError_UnsupportedDataType::FTGetOptError_UnsupportedDataType(FTGetOpt::DataType dataType)
{
    FTString at;

    setSevere();
    switch (dataType)
    {
        case FTGetOpt::dtString:    { at = "dtString"; break; }
        case FTGetOpt::dtInt32:     { at = "dtInt32"; break; }
        case FTGetOpt::dtInt64:     { at = "dtInt64"; break; }
        case FTGetOpt::dtUInt32:    { at = "dtUInt32"; break; }
        case FTGetOpt::dtUInt64:    { at = "dtUInt64"; break; }
        case FTGetOpt::dtDouble:    { at = "dtDouble"; break; }
        case FTGetOpt::dtBool:      { at = "dtBool"; break; }
        default:
        {
            at.format("%d", dataType);
            break;
        }
    }
    setTextf("Unsupported option data type [%s]", at.c_str());
}

FTGetOptError_UnsupportedBooleanValue::FTGetOptError_UnsupportedBooleanValue(cpStr val)
{
    setSevere();
    setTextf("Unsupported boolean argument value [%s]", val);
}

FTGetOptError_FileParsing::FTGetOptError_FileParsing(cpStr val)
{
    setSevere();
    setTextf("Unable to open [%s] for parsing.", val);
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

static Void _print(RAPIDJSON_NAMESPACE::Value &node)
{
    RAPIDJSON_NAMESPACE::StringBuffer sb;
    RAPIDJSON_NAMESPACE::PrettyWriter<RAPIDJSON_NAMESPACE::StringBuffer> writer(sb);

    node.Accept(writer);
    std::cout << sb.GetString() << std::endl;
}

static bool resolvePath(const RAPIDJSON_NAMESPACE::Value &root, RAPIDJSON_NAMESPACE::Value::ConstMemberIterator &node, cpStr path)
{
    auto strings = FTUtility::split(path,"/");
    RAPIDJSON_NAMESPACE::Value name;
    RAPIDJSON_NAMESPACE::Value::ConstMemberIterator n;
    Bool first = true;

    for (FTString &s : strings)
    {
        s.trim();
        name = RAPIDJSON_NAMESPACE::StringRef(s.c_str());
        if (first)
        {
            n = root.FindMember(name);
            if (n == root.MemberEnd())
                return false;
            first = false;
        }
        else
        {
            n = node->value.FindMember(name);
            if (n == node->value.MemberEnd())
                return false;
        }

        node = n;
    }

    return true;
}

static bool findOneOf(const RAPIDJSON_NAMESPACE::Value &root, RAPIDJSON_NAMESPACE::Value::ConstMemberIterator &node, cpStr list)
{
    auto paths = FTUtility::split(list, ",");

    for (FTString &path : paths)
    {
        if (resolvePath(root,node,path))
            return true;
    }

    return false;
}

static bool merge(RAPIDJSON_NAMESPACE::Value &dst, RAPIDJSON_NAMESPACE::Value &src, RAPIDJSON_NAMESPACE::Document::AllocatorType& allocator)
{
    for (auto srcIt = src.MemberBegin(); srcIt != src.MemberEnd(); ++srcIt)
    {
        auto dstIt = dst.FindMember(srcIt->name);
        if (dstIt == dst.MemberEnd())
        {
            RAPIDJSON_NAMESPACE::Value dstName ;
            dstName.CopyFrom(srcIt->name, allocator);
            RAPIDJSON_NAMESPACE::Value dstVal ;
            dstVal.CopyFrom(srcIt->value, allocator) ;

            dst.AddMember(dstName, dstVal, allocator);

            dstName.CopyFrom(srcIt->name, allocator);
            dstIt = dst.FindMember(dstName);
            if (dstIt == dst.MemberEnd())
                return false ;
        }
        else
        {
            auto srcT = srcIt->value.GetType() ;
            auto dstT = dstIt->value.GetType() ;
            if(srcT != dstT)
                return false ;

            if (srcIt->value.IsArray())
            {
                for (auto arrayIt = srcIt->value.Begin(); arrayIt != srcIt->value.End(); ++arrayIt)
                {
                    RAPIDJSON_NAMESPACE::Value dstVal ;
                    dstVal.CopyFrom(*arrayIt, allocator) ;
                    dstIt->value.PushBack(dstVal, allocator);
                }
            }
            else if (srcIt->value.IsObject())
            {
                if(!merge(dstIt->value, srcIt->value, allocator))
                    return false;
            }
            else
            {
                dstIt->value.CopyFrom(srcIt->value, allocator) ;
            }
        }
    }

    return true ;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

struct _FTGetOpt
{
    RAPIDJSON_NAMESPACE::Document s_json;
};

#define ADDRAW(__node__,__arg__,__allocator__) \
{ \
    __node__[RAW].PushBack(RAPIDJSON_NAMESPACE::Value(__arg__, __allocator__), __allocator__); \
}

FTGetOpt::FTGetOpt()
{
    _FTGetOpt *o = new _FTGetOpt();
    o->s_json.SetObject();

    RAPIDJSON_NAMESPACE::Value val;

    val.SetObject();
    o->s_json.AddMember(CMDLINE, val, o->s_json.GetAllocator());

    RAPIDJSON_NAMESPACE::Value &cl = o->s_json[CMDLINE];

    val.SetString("");
    cl.AddMember(PROGRAM, val, o->s_json.GetAllocator());

    val.SetArray();
    cl.AddMember(RAW, val, o->s_json.GetAllocator());

    val.SetArray();
    cl.AddMember(ARGS, val, o->s_json.GetAllocator());

    m_json = o;
}

FTGetOpt::~FTGetOpt()
{
    if (m_json)
    {
        delete ((_FTGetOpt*)m_json);
    }
}

Void FTGetOpt::print() const
{
    _print( ((_FTGetOpt*)m_json)->s_json ) ;
}

Long FTGetOpt::getCmdLine(cpStr path, Long def) const
{
    RAPIDJSON_NAMESPACE::Value &root = ((_FTGetOpt*)m_json)->s_json;
    RAPIDJSON_NAMESPACE::Value::ConstMemberIterator node;
    
    if (!resolvePath(root,node,CMDLINE))
        return def;

    if (findOneOf(node->value,node,path))
    {
        if (node->value.IsString())
            return std::stol(node->value.GetString());

        if (node->value.IsInt())
            return node->value.GetInt();
    }

    return def;
}

LongLong FTGetOpt::getCmdLine(cpStr path, LongLong def) const
{
    RAPIDJSON_NAMESPACE::Value &root = ((_FTGetOpt*)m_json)->s_json;
    RAPIDJSON_NAMESPACE::Value::ConstMemberIterator node;
    
    if (!resolvePath(root,node,CMDLINE))
        return def;

    if (findOneOf(node->value,node,path))
    {
        if (node->value.IsString())
            return std::stoll(node->value.GetString());

        if (node->value.IsInt64())
            return node->value.GetInt64();
    }

    return def;
}

ULong FTGetOpt::getCmdLine(cpStr path, ULong def) const
{
    RAPIDJSON_NAMESPACE::Value &root = ((_FTGetOpt*)m_json)->s_json;
    RAPIDJSON_NAMESPACE::Value::ConstMemberIterator node;
    
    if (!resolvePath(root,node,CMDLINE))
        return def;

    if (findOneOf(node->value,node,path))
    {
        if (node->value.IsString())
            return std::stoul(node->value.GetString());
        
        if (node->value.IsUint())
            return node->value.GetUint();
    }

    return def;
}

ULongLong FTGetOpt::getCmdLine(cpStr path, ULongLong def) const
{
    RAPIDJSON_NAMESPACE::Value &root = ((_FTGetOpt*)m_json)->s_json;
    RAPIDJSON_NAMESPACE::Value::ConstMemberIterator node;
    
    if (!resolvePath(root,node,CMDLINE))
        return def;

    if (findOneOf(node->value,node,path))
    {
        if (node->value.IsString())
            return std::stoull(node->value.GetString());
        
        if (node->value.IsUint64())
            return node->value.GetUint64();
    }

    return def;
}

Double FTGetOpt::getCmdLine(cpStr path, Double def) const
{
    RAPIDJSON_NAMESPACE::Value &root = ((_FTGetOpt*)m_json)->s_json;
    RAPIDJSON_NAMESPACE::Value::ConstMemberIterator node;
    
    if (!resolvePath(root,node,CMDLINE))
        return def;

    if (findOneOf(node->value,node,path))
    {
        if (node->value.IsString())
            return std::stod(node->value.GetString());
        
        if (node->value.IsDouble())
            return node->value.GetDouble();
    }

    return def;
}


cpStr FTGetOpt::getCmdLine(cpStr path, cpStr def) const
{
    RAPIDJSON_NAMESPACE::Value &root = ((_FTGetOpt*)m_json)->s_json;
    RAPIDJSON_NAMESPACE::Value::ConstMemberIterator node;

    if (!resolvePath(root,node,CMDLINE))
        return def;

    if (findOneOf(node->value,node,path))
    {
        if (node->value.IsString())
            return node->value.GetString();
    }

    return def;
}

Bool FTGetOpt::getCmdLine(cpStr path, Bool def) const
{
    RAPIDJSON_NAMESPACE::Value &root = ((_FTGetOpt*)m_json)->s_json;
    RAPIDJSON_NAMESPACE::Value::ConstMemberIterator node;
    
    if (!resolvePath(root,node,CMDLINE))
        return def;

    if (findOneOf(node->value,node,path))
    {
        auto t = node->value.GetType();
        if (node->value.IsBool())
            return node->value.GetBool();
    }

    return def;
}

std::vector<FTString> FTGetOpt::getCmdLineArgs() const
{
    RAPIDJSON_NAMESPACE::Document &root = ((_FTGetOpt*)m_json)->s_json;
    RAPIDJSON_NAMESPACE::Value::ConstMemberIterator node;
    std::vector<FTString> v;
    FTString s;

    if (resolvePath(root,node,CMDLINEARGS))
    {
        if (node->value.IsArray())
        {
            for (auto it = node->value.Begin(); it != node->value.End(); it++)
            {
                s = it->GetString();
                v.push_back(s);
            }
        }
    }

    return v;
}

Long FTGetOpt::get(cpStr path, Long def) const
{
    RAPIDJSON_NAMESPACE::Value &root = ((_FTGetOpt*)m_json)->s_json;
    RAPIDJSON_NAMESPACE::Value::ConstMemberIterator node;
    
    if (!m_prefix.empty() && !resolvePath(root,node,m_prefix.c_str()))
        return def;
    
    if (resolvePath(m_prefix.empty() ? root : node->value,node,path))
    {
        if (node->value.IsString())
            return std::stol(node->value.GetString());

        if (node->value.IsInt())
            return node->value.GetInt();
    }

    return def;
}

LongLong FTGetOpt::get(cpStr path, LongLong def) const
{
    RAPIDJSON_NAMESPACE::Value &root = ((_FTGetOpt*)m_json)->s_json;
    RAPIDJSON_NAMESPACE::Value::ConstMemberIterator node;
    
    if (!m_prefix.empty() && !resolvePath(root,node,m_prefix.c_str()))
        return def;
    
    if (resolvePath(m_prefix.empty() ? root : node->value,node,path))
    {
        if (node->value.IsString())
            return std::stoll(node->value.GetString());
            
        if (node->value.IsInt64())
            return node->value.GetInt64();
    }

    return def;
}

ULong FTGetOpt::get(cpStr path, ULong def) const
{
    RAPIDJSON_NAMESPACE::Value &root = ((_FTGetOpt*)m_json)->s_json;
    RAPIDJSON_NAMESPACE::Value::ConstMemberIterator node;
    
    if (!m_prefix.empty() && !resolvePath(root,node,m_prefix.c_str()))
        return def;
    
    if (resolvePath(m_prefix.empty() ? root : node->value,node,path))
    {
        if (node->value.IsString())
            return std::stoul(node->value.GetString());

        if (node->value.IsUint())
            return node->value.GetUint();
    }

    return def;
}

ULongLong FTGetOpt::get(cpStr path, ULongLong def) const
{
    RAPIDJSON_NAMESPACE::Value &root = ((_FTGetOpt*)m_json)->s_json;
    RAPIDJSON_NAMESPACE::Value::ConstMemberIterator node;
    
    if (!m_prefix.empty() && !resolvePath(root,node,m_prefix.c_str()))
        return def;
    
    if (resolvePath(m_prefix.empty() ? root : node->value,node,path))
    {
        if (node->value.IsString())
            return std::stoull(node->value.GetString());
            
        if (node->value.IsUint64())
            return node->value.GetUint64();
    }

    return def;
}

Double FTGetOpt::get(cpStr path, Double def) const
{
    RAPIDJSON_NAMESPACE::Value &root = ((_FTGetOpt*)m_json)->s_json;
    RAPIDJSON_NAMESPACE::Value::ConstMemberIterator node;
    
    if (!m_prefix.empty() && !resolvePath(root,node,m_prefix.c_str()))
        return def;
    
    if (resolvePath(m_prefix.empty() ? root : node->value,node,path))
    {
        if (node->value.IsString())
            return std::stod(node->value.GetString());

        if (node->value.IsDouble())
            return node->value.GetDouble();
    }

    return def;
}

cpStr FTGetOpt::get(cpStr path, cpStr def) const
{
    RAPIDJSON_NAMESPACE::Value &root = ((_FTGetOpt*)m_json)->s_json;
    RAPIDJSON_NAMESPACE::Value::ConstMemberIterator node;

    if (!m_prefix.empty() && !resolvePath(root,node,m_prefix.c_str()))
        return def;
    
    if (resolvePath(m_prefix.empty() ? root : node->value,node,path))
    {
        if (node->value.IsString())
            return node->value.GetString();
    }

    return def;
}

Bool FTGetOpt::get(cpStr path, Bool def) const
{
    RAPIDJSON_NAMESPACE::Value &root = ((_FTGetOpt*)m_json)->s_json;
    RAPIDJSON_NAMESPACE::Value::ConstMemberIterator node;
    
    if (!m_prefix.empty() && !resolvePath(root,node,m_prefix.c_str()))
        return def;
    
    if (resolvePath(m_prefix.empty() ? root : node->value,node,path))
    {
        if (node->value.IsBool())
            return node->value.GetBool();
    }

    return def;
}

UInt FTGetOpt::getCount(cpStr path) const
{
    RAPIDJSON_NAMESPACE::Value &root = ((_FTGetOpt*)m_json)->s_json;
    RAPIDJSON_NAMESPACE::Value::ConstMemberIterator node;
    UInt cnt = 0;

    if (!m_prefix.empty() && !resolvePath(root,node,m_prefix.c_str()))
        return cnt;

    if (resolvePath(m_prefix.empty() ? root : node->value,node,path))
    {
        if (node->value.IsArray())
            cnt = node->value.Capacity();
    }

    return cnt;
}

Long FTGetOpt::get(UInt idx, cpStr path, cpStr member, Long def) const
{
    RAPIDJSON_NAMESPACE::Value &root = ((_FTGetOpt*)m_json)->s_json;
    RAPIDJSON_NAMESPACE::Value::ConstMemberIterator node;
    
    if (!m_prefix.empty() && !resolvePath(root,node,m_prefix.c_str()))
        return def;

    if (resolvePath(m_prefix.empty() ? root : node->value,node,path))
    {
        if (node->value.IsArray())
        {
            if (resolvePath(node->value[idx],node,member))
            {
                if (node->value.IsString())
                    return std::stol(node->value.GetString());
                    
                if (node->value.IsInt())
                    return node->value.GetInt();
            }
        }
    }

    return def;
}

LongLong FTGetOpt::get(UInt idx, cpStr path, cpStr member, LongLong def) const
{
    RAPIDJSON_NAMESPACE::Value &root = ((_FTGetOpt*)m_json)->s_json;
    RAPIDJSON_NAMESPACE::Value::ConstMemberIterator node;
    
    if (!m_prefix.empty() && !resolvePath(root,node,m_prefix.c_str()))
        return def;
    
    if (resolvePath(m_prefix.empty() ? root : node->value,node,path))
    {
        if (node->value.IsArray())
        {
            if (resolvePath(node->value[idx],node,member))
            {
                if (node->value.IsString())
                    return std::stoll(node->value.GetString());
                    
                if (node->value.IsInt64())
                    return node->value.GetInt64();
            }
        }
    }

    return def;
}

ULong FTGetOpt::get(UInt idx, cpStr path, cpStr member, ULong def) const
{
    RAPIDJSON_NAMESPACE::Value &root = ((_FTGetOpt*)m_json)->s_json;
    RAPIDJSON_NAMESPACE::Value::ConstMemberIterator node;
    
    if (!m_prefix.empty() && !resolvePath(root,node,m_prefix.c_str()))
        return def;
    
    if (resolvePath(m_prefix.empty() ? root : node->value,node,path))
    {
        if (node->value.IsArray())
        {
            if (resolvePath(node->value[idx],node,member))
            {
                if (node->value.IsString())
                    return std::stoul(node->value.GetString());
                    
                if (node->value.IsUint())
                    return node->value.GetUint();
            }
        }
    }

    return def;
}

ULongLong FTGetOpt::get(UInt idx, cpStr path, cpStr member, ULongLong def) const
{
    RAPIDJSON_NAMESPACE::Value &root = ((_FTGetOpt*)m_json)->s_json;
    RAPIDJSON_NAMESPACE::Value::ConstMemberIterator node;
    
    if (!m_prefix.empty() && !resolvePath(root,node,m_prefix.c_str()))
        return def;

    if (resolvePath(m_prefix.empty() ? root : node->value,node,path))
    {
        if (node->value.IsArray())
        {
            if (resolvePath(node->value[idx],node,member))
            {
                if (node->value.IsString())
                    return std::stoull(node->value.GetString());
                    
                if (node->value.IsUint64())
                    return node->value.GetUint64();
            }
        }
    }

    return def;
}

Double FTGetOpt::get(UInt idx, cpStr path, cpStr member, Double def) const
{
    RAPIDJSON_NAMESPACE::Value &root = ((_FTGetOpt*)m_json)->s_json;
    RAPIDJSON_NAMESPACE::Value::ConstMemberIterator node;
    
    if (!m_prefix.empty() && !resolvePath(root,node,m_prefix.c_str()))
        return def;

    if (resolvePath(m_prefix.empty() ? root : node->value,node,path))
    {
        if (node->value.IsArray())
        {
            if (resolvePath(node->value[idx],node,member))
            {
                if (node->value.IsString())
                    return std::stod(node->value.GetString());
                    
                if (node->value.IsDouble())
                    return node->value.GetDouble();
            }
        }
    }

    return def;
}

cpStr FTGetOpt::get(UInt idx, cpStr path, cpStr member, cpStr def) const
{
    RAPIDJSON_NAMESPACE::Value &root = ((_FTGetOpt*)m_json)->s_json;
    RAPIDJSON_NAMESPACE::Value::ConstMemberIterator node;

    if (!m_prefix.empty() && !resolvePath(root,node,m_prefix.c_str()))
        return def;
    
    if (resolvePath(m_prefix.empty() ? root : node->value,node,path))
    {
        if (node->value.IsArray())
        {
            if (resolvePath(node->value[idx],node,member))
            {
                if (node->value.IsString())
                    return node->value.GetString();
            }
        }
    }

    return def;
}

Bool FTGetOpt::get(UInt idx, cpStr path, cpStr member, Bool def) const
{
    RAPIDJSON_NAMESPACE::Value &root = ((_FTGetOpt*)m_json)->s_json;
    RAPIDJSON_NAMESPACE::Value::ConstMemberIterator node;
    
    if (!m_prefix.empty() && !resolvePath(root,node,m_prefix.c_str()))
        return def;
    
    if (resolvePath(m_prefix.empty() ? root : node->value,node,path))
    {
        if (node->value.IsArray())
        {
            if (resolvePath(node->value[idx],node,member))
            {
                if (node->value.IsBool())
                    return node->value.GetBool();
            }
        }
    }

    return def;
}

const FTGetOpt::Option *FTGetOpt::findOption(cpStr name, const FTGetOpt::Option *options)
{
    if (!name)
        return NULL;

    Int len = strlen(name);
    Bool isShort = len == 2 && name[0] == '-';
    Bool isLong = len >= 3 && name[0] == '-' && name[1] == '-';

    while (!options->shortName.empty() || !options->longName.empty())
    {
        if (isShort && options->shortName == name)
            return options;
        else if (isLong && options->longName == name)
            return options;
        options++;
    }

    return NULL;
}

Void FTGetOpt::loadCmdLine(Int argc, pStr *argv, const FTGetOpt::Option *options)
{
    RAPIDJSON_NAMESPACE::Document &root = ((_FTGetOpt*)m_json)->s_json;
    RAPIDJSON_NAMESPACE::Document::AllocatorType& allocator = root.GetAllocator();

    RAPIDJSON_NAMESPACE::Value& cl = root[CMDLINE];

    const FTGetOpt::Option *opt = NULL;
    RAPIDJSON_NAMESPACE::Value key;
    RAPIDJSON_NAMESPACE::Value val;

    for (Int i=0; i<argc; i++)
    {
        ADDRAW(cl, argv[i], allocator);

        if (i == 0)
        {
            cl[PROGRAM].SetString(argv[i], allocator);
        }
        else
        {
            if (opt)
            {
                switch (opt->argType)
                {
                    case required_argument:
                    {
                        switch (opt->dataType)
                        {
                            case FTGetOpt::dtString:
                            {
                                val.SetString(argv[i], allocator);
                                break;
                            }
                            case FTGetOpt::dtInt32:
                            {
                                val.SetInt(std::stol(argv[i]));
                                break;
                            }
                            case FTGetOpt::dtInt64:
                            {
                                val.SetInt64(std::stoll(argv[i]));
                                break;
                            }
                            case FTGetOpt::dtUInt32:
                            {
                                val.SetUint(std::stoul(argv[i]));
                                break;
                            }
                            case FTGetOpt::dtUInt64:
                            {
                                val.SetUint64(std::stoull(argv[i]));
                                break;
                            }
                            case FTGetOpt::dtDouble:
                            {
                                val.SetDouble(std::stod(argv[i]));
                                break;
                            }
                            case FTGetOpt::dtBool:
                            {
                                FTString arg(argv[i]);
                                arg.tolower();
                                if (arg == "true" || arg == "yes")
                                    val.SetBool(true);
                                else if (arg == "false" || arg == "no")
                                    val.SetBool(false);
                                else
                                    throw FTGetOptError_UnsupportedBooleanValue(argv[i]);
                                break;
                            }
                            default:
                            {
                                throw FTGetOptError_UnsupportedDataType(opt->dataType);
                                break;
                            }
                        }

                        if (cl.HasMember(key))
                            cl[key] = val;
                        else
                            cl.AddMember(key, val, allocator);

                        opt = NULL;

                        break;
                    }
                    default:
                    {
                        throw FTGetOptError_UnsupportedArgType(opt->argType);
                        break;
                    }
                }
            }
            else
            {
                opt = findOption(argv[i], options);
                key.SetString(argv[i], allocator);

                if (opt)
                {
                    switch (opt->argType)
                    {
                        case no_argument:
                        {
                            RAPIDJSON_NAMESPACE::Value val(true);
                            if (cl.HasMember(key))
                                cl[key] = val;
                            else
                                cl.AddMember(key, val, allocator);
                            opt = NULL;
                            break;
                        }
                        case required_argument:
                        {
                            break;
                        }
                        default:
                        {
                            throw FTGetOptError_UnsupportedArgType(opt->argType);
                            break;
                        }
                    }
                }
                else
                {
                    RAPIDJSON_NAMESPACE::Value& a = cl[ARGS];
                    val.SetString(argv[i], allocator);
                    a.PushBack(val, allocator);
                }                
            }            
        }        
    }
}

Void FTGetOpt::loadFile(cpStr filename)
{
    RAPIDJSON_NAMESPACE::Document &dst = ((_FTGetOpt*)m_json)->s_json;
    RAPIDJSON_NAMESPACE::Document src;
    FILE *fp;
    Char buf[ FILEBUFFER];

    fp = fopen(filename, "r");
    if (!fp)
        throw FTGetOptError_FileParsing(filename);

    RAPIDJSON_NAMESPACE::FileReadStream is(fp, buf, sizeof(buf));
    src.ParseStream(is);
    fclose(fp);

    merge(dst, src, dst.GetAllocator());
}