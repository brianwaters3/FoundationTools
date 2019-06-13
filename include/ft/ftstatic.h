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

#ifndef __ftstatic_h_included
#define __ftstatic_h_included

#include "ftbase.h"
#include "ftgetopt.h"

enum {
    STATIC_INIT_TYPE_SHARED_OBJECT_MANAGER,
    STATIC_INIT_TYPE_PRIORITY,
    STATIC_INIT_TYPE_THREADS,            // initialized thread info

    STATIC_INIT_TYPE_STRING_MANAGER,
    STATIC_INIT_TYPE_FILE_MANAGER,
    STATIC_INIT_TYPE_DLLS,
    STATIC_INIT_TYPE_BEFORE_OTHER,
    STATIC_INIT_TYPE_OTHER,
    STATIC_INIT_TYPE_AFTER_ALL			//For anything that relies upon others
};

class FTStatic
{
public:
    static Void Initialize(FTGetOpt& opt);
    static Void UnInitialize();

    virtual Int getInitType() { return STATIC_INIT_TYPE_OTHER; }

    virtual Void init(FTGetOpt& opt)   {}
    virtual Void uninit() {}

    FTStatic();
    virtual ~FTStatic();

protected:
    static FTStatic *m_pFirst; 
    FTStatic *m_pNext;
};

#endif // #define __ftstatic_h_included
