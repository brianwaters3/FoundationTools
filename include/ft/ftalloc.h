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

#ifndef __ftalloc_h_included
#define __ftalloc_h_included

#include "ftsynch.h"

#define FTALLOCATOR_THREAD_SAFE

#include <new>
#include <cassert>
#include <vector>

template<unsigned ElemSize>
class FTAllocator_ElemAllocator : public FTStatic
{
    typedef std::size_t Data_t;
    static const Data_t BlockElements = 512;

    static const Data_t DSize = sizeof(Data_t);
    static const Data_t ElemSizeInDSize = (ElemSize + (DSize-1)) / DSize;
    static const Data_t UnitSizeInDSize = ElemSizeInDSize + 1;
    static const Data_t BlockSize = BlockElements*UnitSizeInDSize;

    class MemBlock
    {
        Data_t* block;
        Data_t firstFreeUnitIndex, allocatedElementsAmount, endIndex;

     public:
        MemBlock():
            block(0),
            firstFreeUnitIndex(Data_t(-1)),
            allocatedElementsAmount(0)
        {}

        bool isFull() const
        {
            return allocatedElementsAmount == BlockElements;
        }

        void clear()
        {
            delete[] block;
            block = 0;
            firstFreeUnitIndex = Data_t(-1);
        }

        void* allocate(Data_t vectorIndex)
        {
            if(firstFreeUnitIndex == Data_t(-1))
            {
                if(!block)
                {
                    block = new Data_t[BlockSize];
                    if(!block) return 0;
                    endIndex = 0;
                }

                Data_t* retval = block + endIndex;
                endIndex += UnitSizeInDSize;
                retval[ElemSizeInDSize] = vectorIndex;
                ++allocatedElementsAmount;
                return retval;
            }
            else
            {
                Data_t* retval = block + firstFreeUnitIndex;
                firstFreeUnitIndex = *retval;
                ++allocatedElementsAmount;
                return retval;
            }
        }

        void deallocate(Data_t* ptr)
        {
            *ptr = firstFreeUnitIndex;
            firstFreeUnitIndex = ptr - block;

            if(--allocatedElementsAmount == 0)
                clear();
        }
    };

    struct BlocksVector
    {
        std::vector<MemBlock> data;

        BlocksVector() { data.reserve(1024); }

        ~BlocksVector()
        {
            for(size_t i = 0; i < data.size(); ++i)
                data[i].clear();
        }
    };

    static BlocksVector blocksVector;
    static std::vector<Data_t> blocksWithFree;

#ifdef FTALLOCATOR_THREAD_SAFE
    static FTMutex mutex;
#endif

 public:
    virtual Int getInitType() { return STATIC_INIT_TYPE_PRIORITY; }
    Void init(FTGetOpt& options)
    {
#ifdef FTALLOCATOR_THREAD_SAFE
        mutex.init(NULL);
#endif
    }
    Void uninit()
    {
#ifdef FTALLOCATOR_THREAD_SAFE
        mutex.destroy();
#endif
    }

    static void* allocate()
    {
#ifdef FTALLOCATOR_THREAD_SAFE
        FTMutexLock l(mutex);
#endif

        if(blocksWithFree.empty())
        {
            blocksWithFree.push_back(blocksVector.data.size());
            blocksVector.data.push_back(MemBlock());
        }

        const Data_t index = blocksWithFree.back();
        MemBlock& block = blocksVector.data[index];
        void* retval = block.allocate(index);

        if(block.isFull())
            blocksWithFree.pop_back();

        return retval;
    }

    static void deallocate(void* ptr)
    {
        if(!ptr) return;

#ifdef FTALLOCATOR_THREAD_SAFE
        FTMutexLock l(mutex);
#endif

        Data_t* unitPtr = (Data_t*)ptr;
        const Data_t blockIndex = unitPtr[ElemSizeInDSize];
        MemBlock& block = blocksVector.data[blockIndex];

        if(block.isFull())
            blocksWithFree.push_back(blockIndex);
        block.deallocate(unitPtr);
    }
};

template<unsigned ElemSize>
typename FTAllocator_ElemAllocator<ElemSize>::BlocksVector
FTAllocator_ElemAllocator<ElemSize>::blocksVector;

template<unsigned ElemSize>
std::vector<typename FTAllocator_ElemAllocator<ElemSize>::Data_t>
FTAllocator_ElemAllocator<ElemSize>::blocksWithFree;

#ifdef FTALLOCATOR_THREAD_SAFE
template<unsigned ElemSize>
FTMutex FTAllocator_ElemAllocator<ElemSize>::mutex(False);
#endif

template<typename Ty>
class FTAllocator
{
 public:
    typedef size_t size_type;
    typedef ptrdiff_t difference_type;
    typedef Ty *pointer;
    typedef const Ty *const_pointer;
    typedef Ty& reference;
    typedef const Ty& const_reference;
    typedef Ty value_type;

    pointer address(reference val) const { return &val; }
    const_pointer address(const_reference val) const { return &val; }

    template<class Other>
    struct rebind
    {
        typedef FTAllocator<Other> other;
    };

    FTAllocator() throw() {}

    template<class Other>
    FTAllocator(const FTAllocator<Other>&) throw() {}

    template<class Other>
    FTAllocator& operator=(const FTAllocator<Other>&) { return *this; }

    pointer allocate(size_type count, const void* = 0)
    {
        assert(count == 1);
        return static_cast<pointer>
            (FTAllocator_ElemAllocator<sizeof(Ty)>::allocate());
    }

    void deallocate(pointer ptr, size_type)
    {
        FTAllocator_ElemAllocator<sizeof(Ty)>::deallocate(ptr);
    }

    void construct(pointer ptr, const Ty& val)
    {
        new ((void *)ptr) Ty(val);
    }

    void destroy(pointer ptr)
    {
        ptr->Ty::~Ty();
    }

    size_type max_size() const throw() { return 1; }
};

#endif // #define __ftalloc_h_included
