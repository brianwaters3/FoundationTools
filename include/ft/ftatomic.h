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

#ifndef __ftatomic_h_included
#define __ftatomic_h_included

#if defined(FT_WINDOWS)
#include <intrin.h>
#define atomic_inc(val) _InterlockedIncrement(&val)
#define atomic_dec(val) _InterlockedDecrement(&val)
#define atomic_cas(a,b,c) _InterlockedCompareExchange(&a,c,b)
#define atomic_swap(a,b) _InterlockedExchange(&a,b)
#define atomic_and(a,b) _InterlockedAnd((pLong)&a,(Long)b)
#define atomic_or(a,b) _InterlockedOr((pLong)&a,(Long)b)
#pragma intrinsic (_InterlockedIncrement, _InterlockedDecrement, _InterlockedCompareExchange, _InterlockedExchange, _InterlockedAnd, _InterlockedOr)
#elif defined(FT_GCC)
#include <ext/atomicity.h>
#define atomic_dec(a) __sync_sub_and_fetch((pULong)&a,1)
#define atomic_inc(a) __sync_add_and_fetch((pULong)&a,1)
#define atomic_cas(a,b,c) __sync_val_compare_and_swap((pULong)&a,b,c)
#define atomic_swap(a,b) __sync_lock_test_and_set((pULong)&a,b);
#define atomic_and(a,b) __sync_fetch_and_and((pULong)&a,b)
#define atomic_or(a,b) __sync_fetch_and_or((pULong)&a,b)
#elif defined(FT_SOLARIS)
#include <atomic.h>
#define atomic_dec(a) atomic_dec_ulong_nv((pULong)&a)
#define atomic_inc(a) atomic_inc_ulong_nv((pULong)&a)
#define atomic_cas(a,b,c) atomic_cas_ulong((pULong)&a,b,c)
#define atomic_swap(a,b) atomic_swap_ulong((pULong)&a,b);
#define atomic_and(a,b) atomic_and_ulong((pULong)&a,b)
#define atomic_or(a,b) atomic_or_ulong((pULong)&a,b)
#else
#error "Unrecoginzed platform"
#endif

#endif // #define __ftatomic_h_included

