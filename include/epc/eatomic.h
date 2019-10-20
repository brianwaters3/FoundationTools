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

#ifndef __eatomic_h_included
#define __eatomic_h_included

/// @file
/// @brief Macros for performing CPU atomic/interlaced operations.

/// atomic decrement - decrements a by 1
#define atomic_dec(a) __sync_sub_and_fetch(&a, 1)
/// atomic increment - increments a by 1
#define atomic_inc(a) __sync_add_and_fetch(&a, 1)
/// atomic decrement - decrements a by 1
#define atomic_dec_fetch(a) __sync_sub_and_fetch(&a, 1)
/// atomic increment - increments a by 1
#define atomic_inc_fetch(a) __sync_add_and_fetch(&a, 1)
/// atomic decrement - decrements a by 1
#define atomic_fetch_dec(a) __sync_fetch_and_sub(&a, 1)
/// atomic increment - increments a by 1
#define atomic_fetch_inc(a) __sync_fetch_and_add(&a, 1)
/// atomic compare and swap - if a equals b then a is replaced with c
#define atomic_cas(a, b, c) __sync_val_compare_and_swap(&a, b, c)
/// atomic swap - replaces a with b
#define atomic_swap(a, b) __sync_lock_test_and_set(&a, b)
/// atomic and - performs a logical and between a and b putting the result in a
#define atomic_and(a, b) __sync_fetch_and_and(&a, b)
/// atomic or - performs a logical or between a and b putting the result in a
#define atomic_or(a, b) __sync_fetch_and_or(&a, b)
/// atomic set - replaces a with b
#define atomic_set(a, b) __sync_lock_test_and_set(&a, b)

#endif // #define __eatomic_h_included
