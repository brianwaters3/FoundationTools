/////////////////////////////////////////////////////////////////////////
// (c) 2009 Brian Waters (bdwaters@sbcglobal.net) All rights reserved. //
/////////////////////////////////////////////////////////////////////////

#include "ftbase.h"
#include "ftbzip2.h"
#include <string.h>

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

FTBZip2Error_ReadOpen::FTBZip2Error_ReadOpen(cpChar msg)
{
    setSevere();
    setTextf("Error opening [%s] for reading", msg);
	appendLastOsError();
}

FTBZip2Error_WriteOpen::FTBZip2Error_WriteOpen(cpChar msg)
{
    setSevere();
    setTextf("Error opening [%s] for writing", msg);
	appendLastOsError();
}

FTBZip2Error_Bzip2ReadInit::FTBZip2Error_Bzip2ReadInit(Int err)
{
    setSevere();
    setTextf("Error initializing bzip2 for reading - %s (%d)", FTBzip2::getErrorDesc(err), err);
}

FTBZip2Error_Bzip2WriteInit::FTBZip2Error_Bzip2WriteInit(Int err)
{
    setSevere();
    setTextf("Error initializing bzip2 for writing - %s (%d)", FTBzip2::getErrorDesc(err), err);
}

FTBZip2Error_Bzip2Read::FTBZip2Error_Bzip2Read(Int err)
{
    setSevere();
    setTextf("Error reading bzip2 block - %s (%d)", FTBzip2::getErrorDesc(err), err);
}

FTBZip2Error_Bzip2Write::FTBZip2Error_Bzip2Write(Int err)
{
    setSevere();
    setTextf("Error writing bzip2 block - %s (%d)", FTBzip2::getErrorDesc(err), err);
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

FTBzip2::FTBzip2()
{
	m_fh = NULL;
	m_bfh = NULL;
	m_term = '\n';
	m_bzerror = BZ_OK;
	m_operation = bz2opNone;
	m_bytesin = 0;
	m_bytesout = 0;
}

FTBzip2::~FTBzip2()
{
	close();
}

Void FTBzip2::readOpen(cpStr filename)
{
#if defined(FT_WINDOWS)
    errno_t err = fopen_s(&m_fh, filename, "rb");
	if (err != 0)
		m_fh = NULL;
#elif defined(FT_GCC) || defined(FT_SOLARIS)
    m_fh = fopen(filename, "rb");
#else
#error "Unrecognized platform"
#endif

	if (!m_fh)
		throw new FTBZip2Error_ReadOpen(filename);

	m_bfh = BZ2_bzReadOpen(&m_bzerror, m_fh, 0, 0, NULL, 0);
	if (m_bzerror != BZ_OK)
	{
		close();
		throw new FTBZip2Error_Bzip2ReadInit(m_bzerror);
	}

	m_len = sizeof(m_buf);
	m_ofs = sizeof(m_buf);

	m_operation = bz2opRead;
}

Void FTBzip2::writeOpen(cpStr filename)
{
	throw new FTError(FTError::Error, "FTBzip2::writeOpen() is not implemented");
}

void FTBzip2::close()
{
	if (m_bfh)
	{
		unsigned int inl=0, inh=0, outl=0, outh=0;

		switch (m_operation)
		{
			case bz2opRead:		BZ2_bzReadClose(&m_bzerror, m_bfh);									break;
			case bz2opWrite:	BZ2_bzWriteClose64(&m_bzerror, m_bfh, 0, &inl, &inh, &outl, &outh);	break;
			default: break;
		}

		ulonginteger_t v;

		v.uli.lowPart = (Dword)inl;
		v.uli.highPart = (Dword)inh;
		m_bytesin = v.quadPart;

		v.uli.lowPart = (Dword)outl;
		v.uli.highPart = (Dword)outh;
		m_bytesout = v.quadPart;

		m_operation = bz2opNone;

		m_bfh = NULL;
	}

	if (m_fh)
	{
		::fclose(m_fh);
		m_fh = NULL;
	}
}

Int FTBzip2::read(pUChar pbuf, Int length)
{
	Int amtRead = 0;

	while (amtRead < length)
	{
		if (m_ofs == m_len)
		{
			if (m_len < (Int)sizeof(m_buf))
				break;

			m_len = BZ2_bzRead(&m_bzerror, m_bfh, m_buf, sizeof(m_buf));
			if (m_bzerror != BZ_OK && m_bzerror != BZ_STREAM_END)
				throw new FTBZip2Error_Bzip2Read(m_bzerror);

			m_ofs = 0;
		}

		Int amt = min(m_len - m_ofs, length - amtRead);

		memcpy(&pbuf[amtRead], &m_buf[m_ofs], amt);

		m_ofs += amt;
		amtRead += amt;
	}

	return amtRead;
}

Int FTBzip2::readLine(pStr pbuf, Int length)
{
	Int amtRead = 0;

	while (amtRead < length - 1)
	{
		if (m_ofs == m_len)
		{
			if (m_len < (Int)sizeof(m_buf))
				break;

			m_len = BZ2_bzRead(&m_bzerror, m_bfh, m_buf, sizeof(m_buf));
			if (m_bzerror != BZ_OK && m_bzerror != BZ_STREAM_END)
				throw new FTBZip2Error_Bzip2Read(m_bzerror);

			m_ofs = 0;

			if (m_len == 0)
				break;
		}

		pUChar p = (pUChar)memchr(&m_buf[m_ofs], m_term, m_len - m_ofs);
		Int amt = (p == NULL) ? m_len - m_ofs : (Int)(p - &m_buf[m_ofs] + 1);

		memcpy(&pbuf[amtRead], &m_buf[m_ofs], amt);

		m_ofs += amt;
		amtRead += amt;

		if (pbuf[amtRead-1] == m_term)
			break;
	}

	pbuf[amtRead] = '\0';

	return amtRead;
}

Int FTBzip2::write(pUChar pbuf, Int length)
{
	throw new FTError(FTError::Error, "FTBzip2::write() is not implemented");
}

cpStr FTBzip2::getErrorDesc(Int e)
{
	switch (e)
	{
		case BZ_OK:					return "BZ_OK";
		case BZ_RUN_OK:				return "BZ_RUN_OK";
		case BZ_FLUSH_OK:			return "BZ_FLUSH_OK";
		case BZ_FINISH_OK:			return "BZ_FINISH_OK";
		case BZ_STREAM_END:			return "BZ_STREAM_END";
		case BZ_SEQUENCE_ERROR:		return "BZ_SEQUENCE_ERROR";
		case BZ_PARAM_ERROR:		return "BZ_PARAM_ERROR";
		case BZ_MEM_ERROR:			return "BZ_MEM_ERROR";
		case BZ_DATA_ERROR:			return "BZ_DATA_ERROR";
		case BZ_DATA_ERROR_MAGIC:	return "BZ_DATA_ERROR_MAGIC";
		case BZ_IO_ERROR:			return "BZ_IO_ERROR";
		case BZ_UNEXPECTED_EOF:		return "BZ_UNEXPECTED_EOF";
		case BZ_OUTBUFF_FULL:		return "BZ_OUTBUFF_FULL";
		case BZ_CONFIG_ERROR:		return "BZ_CONFIG_ERROR";
	}

	return "UNKNOWN";
}
