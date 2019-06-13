/////////////////////////////////////////////////////////////////////////
// (c) 2009 Brian Waters (bdwaters@sbcglobal.net) All rights reserved. //
/////////////////////////////////////////////////////////////////////////
#include "ftstatic.h"

FTStatic* FTStatic::m_pFirst = NULL;

Void FTStatic::Initialize(FTGetOpt& opt)
{
    for (Int t=0; t<=STATIC_INIT_TYPE_AFTER_ALL; t++)
    {
        for (FTStatic *p = FTStatic::m_pFirst; p != NULL; p = p->m_pNext)
        {
            if (p->getInitType() == t)
                p->init(opt);
        }
    }
}

void FTStatic::UnInitialize()
{
    for (Int t=STATIC_INIT_TYPE_AFTER_ALL; t>=0; t--)
    {
        for (FTStatic *p = FTStatic::m_pFirst; p != NULL; p = p->m_pNext)
        {
            if (p->getInitType() == t)
                p->uninit();
        }
    }
}

FTStatic::~FTStatic()
{
    FTStatic *last = NULL;

    for (FTStatic *p = m_pFirst; p != NULL && p != this; p = p->m_pNext)
        last = p;

    if (last)
        last->m_pNext = m_pNext;
    else
        m_pFirst = m_pNext;

    m_pNext = NULL;
}

FTStatic::FTStatic()
{
    m_pNext = m_pFirst;
    m_pFirst = this;
}