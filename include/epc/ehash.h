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

#ifndef __ehash_h_included
#define __ehash_h_included

#include "ebase.h"
#include "estring.h"

class EHash
{
public:
   static ULong getHash(EString &str);
   static ULong getHash(cpChar val, ULong len);
   static ULong getHash(cpUChar val, ULong len);

protected:
private:
   static ULong m_crcTable[256];
};

#endif // #define __ehash_h_included