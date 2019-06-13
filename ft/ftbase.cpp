/////////////////////////////////////////////////////////////////////////
// (c) 2009 Brian Waters (bdwaters@sbcglobal.net) All rights reserved. //
/////////////////////////////////////////////////////////////////////////
#include "ftinternal.h"
#include "fttbase.h"
#include "ftsynch2.h"

//extern FTLogger _logCtrl;

Void FoundationTools::Initialize(FTGetOpt& options)
{
    FTStatic::Initialize(options);
    FTThreadBasic::Initialize();
}

Void FoundationTools::UnInitialize()
{
    FTSynchObjects::getSynchObjCtrlPtr()->logObjectUsage();

    FTThreadBasic::UnInitialize();
    FTStatic::UnInitialize();
}

Int FoundationTools::m_internalLogId = -1;
Int FoundationTools::m_appid = 0;
