/////////////////////////////////////////////////////////////////////////
// (c) 2009 Brian Waters (bdwaters@sbcglobal.net) All rights reserved. //
/////////////////////////////////////////////////////////////////////////
#include "ftmessage.h"

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

Void FTMessage::getLength(ULong &length)
{
}

Void FTMessage::serialize(pVoid pBuffer, ULong &nOffset)
{
}

Void FTMessage::unserialize(pVoid pBuffer, ULong &nOffset)
{
}

Void FTMessage::pack(Bool val, pVoid pBuffer, ULong &nOffset)
{
    *((Bool*)(&((pStr)pBuffer)[nOffset])) = val;
    nOffset += sizeof(Bool);
}

Void FTMessage::pack(Char val, pVoid pBuffer, ULong &nOffset)
{
    *((Char*)(&((pStr)pBuffer)[nOffset])) = val;
    nOffset += sizeof(Char);
}

Void FTMessage::pack(UChar val, pVoid pBuffer, ULong &nOffset)
{
    *((UChar*)(&((pStr)pBuffer)[nOffset])) = val;
    nOffset += sizeof(UChar);
}

Void FTMessage::pack(Short val, pVoid pBuffer, ULong &nOffset)
{
    *((Short*)(&((pStr)pBuffer)[nOffset])) = val;
    nOffset += sizeof(Short);
}

Void FTMessage::pack(UShort val, pVoid pBuffer, ULong &nOffset)
{
    *((UShort*)(&((pStr)pBuffer)[nOffset])) = val;
    nOffset += sizeof(UShort);
}

//Void FTMessage::pack(Int val, pVoid pBuffer, ULong &nOffset)
//{
//    *((Int*)(&((pStr)pBuffer)[nOffset])) = val;
//    nOffset += sizeof(Int);
//}
//
//Void FTMessage::pack(UInt val, pVoid pBuffer, ULong &nOffset)
//{
//    *((UInt*)(&((pStr)pBuffer)[nOffset])) = val;
//    nOffset += sizeof(UInt);
//}
//
Void FTMessage::pack(Long val, pVoid pBuffer, ULong &nOffset)
{
    *((Long*)(&((pStr)pBuffer)[nOffset])) = val;
    nOffset += sizeof(Long);
}

Void FTMessage::pack(ULong val, pVoid pBuffer, ULong &nOffset)
{
    *((ULong*)(&((pStr)pBuffer)[nOffset])) = val;
    nOffset += sizeof(ULong);
}

Void FTMessage::pack(LongLong val, pVoid pBuffer, ULong &nOffset)
{
    *((LongLong*)(&((pStr)pBuffer)[nOffset])) = val;
    nOffset += sizeof(LongLong);
}

Void FTMessage::pack(ULongLong val, pVoid pBuffer, ULong &nOffset)
{
    *((ULongLong*)(&((pStr)pBuffer)[nOffset])) = val;
    nOffset += sizeof(ULongLong);
}

Void FTMessage::pack(Float val, pVoid pBuffer, ULong &nOffset)
{
    *((Float*)(&((pStr)pBuffer)[nOffset])) = val;
    nOffset += sizeof(Float);
}

Void FTMessage::pack(Double val, pVoid pBuffer, ULong &nOffset)
{
    *((Double*)(&((pStr)pBuffer)[nOffset])) = val;
    nOffset += sizeof(Double);
}

Void FTMessage::pack(cpStr val, pVoid pBuffer, ULong &nOffset)
{
    UShort len = (UShort)strlen(val);
    pack(len, pBuffer, nOffset);
    memcpy(&((pStr)pBuffer)[nOffset], val, len);
    nOffset += len;
}

Void FTMessage::pack(FTTimerElapsed &val, pVoid pBuffer, ULong &nOffset)
{
    *((fttime_t*)(&((pStr)pBuffer)[nOffset])) = val;
    nOffset += sizeof(fttime_t);
}

Void FTMessage::pack(FTTime &val, pVoid pBuffer, ULong &nOffset)
{
    //((timeval*)(&((pStr)pBuffer)[nOffset]))->tv_sec = val.getTimeVal().tv_sec;
    *((LongLong*)(&((pStr)pBuffer)[nOffset])) = (LongLong)val.getTimeVal().tv_sec;
    nOffset += sizeof(LongLong);

    //((timeval*)(&((pStr)pBuffer)[nOffset]))->tv_usec = val.getTimeVal().tv_usec;
    *((LongLong*)(&((pStr)pBuffer)[nOffset])) = (LongLong)val.getTimeVal().tv_usec;
    nOffset += sizeof(LongLong);
}

Void FTMessage::pack(FTString& val, pVoid pBuffer, ULong &nOffset)
{
    UShort len = (UShort)val.length();
    pack(len, pBuffer, nOffset);
    memcpy(&((pStr)pBuffer)[nOffset], val.c_str(), len);
    nOffset += len;
}

Void FTMessage::unpack(Bool &val, pVoid pBuffer, ULong &nOffset)
{
    val = *((Bool*)(&((pStr)pBuffer)[nOffset]));
    nOffset += sizeof(val);
}

Void FTMessage::unpack(Char &val, pVoid pBuffer, ULong &nOffset)
{
    val = *((Char*)(&((pStr)pBuffer)[nOffset]));
    nOffset += sizeof(val);
}

Void FTMessage::unpack(UChar &val, pVoid pBuffer, ULong &nOffset)
{
    val = *((UChar*)(&((pStr)pBuffer)[nOffset]));
    nOffset += sizeof(val);
}

Void FTMessage::unpack(Short &val, pVoid pBuffer, ULong &nOffset)
{
    val = *((Short*)(&((pStr)pBuffer)[nOffset]));
    nOffset += sizeof(val);
}

Void FTMessage::unpack(UShort &val, pVoid pBuffer, ULong &nOffset)
{
    val = *((UShort*)(&((pStr)pBuffer)[nOffset]));
    nOffset += sizeof(val);
}

//Void FTMessage::unpack(Int &val, pVoid pBuffer, ULong &nOffset)
//{
//    val = *((Int*)(&((pStr)pBuffer)[nOffset]));
//    nOffset += sizeof(val);
//}
//
//Void FTMessage::unpack(UInt &val, pVoid pBuffer, ULong &nOffset)
//{
//    val = *((UInt*)(&((pStr)pBuffer)[nOffset]));
//    nOffset += sizeof(val);
//}

Void FTMessage::unpack(Long &val, pVoid pBuffer, ULong &nOffset)
{
    val = *((Long*)(&((pStr)pBuffer)[nOffset]));
    nOffset += sizeof(val);
}

Void FTMessage::unpack(ULong &val, pVoid pBuffer, ULong &nOffset)
{
    val = *((ULong*)(&((pStr)pBuffer)[nOffset]));
    nOffset += sizeof(val);
}

Void FTMessage::unpack(LongLong &val, pVoid pBuffer, ULong &nOffset)
{
    val = *((LongLong*)(&((pStr)pBuffer)[nOffset]));
    nOffset += sizeof(val);
}

Void FTMessage::unpack(ULongLong &val, pVoid pBuffer, ULong &nOffset)
{
    val = *((ULongLong*)(&((pStr)pBuffer)[nOffset]));
    nOffset += sizeof(val);
}

Void FTMessage::unpack(Float &val, pVoid pBuffer, ULong &nOffset)
{
    val = *((Float*)(&((pStr)pBuffer)[nOffset]));
    nOffset += sizeof(val);
}

Void FTMessage::unpack(Double &val, pVoid pBuffer, ULong &nOffset)
{
    val = *((Double*)(&((pStr)pBuffer)[nOffset]));
    nOffset += sizeof(val);
}

Void FTMessage::unpack(pStr val, pVoid pBuffer, ULong &nOffset)
{
    UShort len = 0;

    unpack(len, pBuffer, nOffset);
    memcpy(val, &((pStr)pBuffer)[nOffset], len);
    val[len] = '\0';
    nOffset += len;
}

Void FTMessage::unpack(FTTimerElapsed &val, pVoid pBuffer, ULong &nOffset)
{
    val.Set(*((fttime_t*)(&((pStr)pBuffer)[nOffset])));
    nOffset += sizeof(fttime_t);
}

Void FTMessage::unpack(FTTime &val, pVoid pBuffer, ULong &nOffset)
{
    timeval tv;

//    tv.tv_sec = ((timeval*)(&((pStr)pBuffer)[nOffset]))->tv_sec;
//    tv.tv_usec = ((timeval*)(&((pStr)pBuffer)[nOffset]))->tv_usec;
//    nOffset += sizeof(timeval);

#ifdef FT_WINDOWS
#pragma warning(disable : 4244)
#endif
    tv.tv_sec = (time_t)*((LongLong*)(&((pStr)pBuffer)[nOffset]));
    nOffset += sizeof(LongLong);
    tv.tv_usec = (time_t)*((LongLong*)(&((pStr)pBuffer)[nOffset]));
    nOffset += sizeof(LongLong);
#ifdef FT_WINDOWS
#pragma warning(default : 4244)
#endif

    val.set(tv);
}

Void FTMessage::unpack(FTString &val, pVoid pBuffer, ULong &nOffset)
{
    UShort len = 0;

    unpack(len, pBuffer, nOffset);
    val.assign(&((pStr)pBuffer)[nOffset], len);
    nOffset += len;
}
