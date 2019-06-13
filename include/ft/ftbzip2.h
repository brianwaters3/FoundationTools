////////////////////////////////////////////////////
// (c) 2009 Brian Waters (bdwaters@sbcglobal.net) //
////////////////////////////////////////////////////
#ifndef __ftbzip2_h_included
#define __ftbzip2_h_included

#include "ftstring.h"
#include "fterror.h"
#include "bzlib.h"

DECLARE_ERROR_ADVANCED4(FTBZip2Error_ReadOpen);
DECLARE_ERROR_ADVANCED4(FTBZip2Error_WriteOpen);
DECLARE_ERROR_ADVANCED2(FTBZip2Error_Bzip2ReadInit);
DECLARE_ERROR_ADVANCED2(FTBZip2Error_Bzip2WriteInit);
DECLARE_ERROR_ADVANCED2(FTBZip2Error_Bzip2Read);
DECLARE_ERROR_ADVANCED2(FTBZip2Error_Bzip2Write);

class FTBzip2
{
public:
    enum Operation { bz2opNone, bz2opRead, bz2opWrite };

	FTBzip2();
	~FTBzip2();

	FTString& setFileName(cpStr filename)	{ m_filename = filename; return m_filename; }
	FTString& getFileName()					{ return m_filename; }

	cChar setTerminator(cChar c)			{ m_term = c; return getTerminator(); }
	cChar getTerminator()					{ return m_term; }

	Int getLastError()						{ return m_bzerror; }
	static cpStr getErrorDesc(Int e);

	ULongLong getBytesIn()					{ return m_bytesin; }
	ULongLong getBytesOut()					{ return m_bytesout; }

	Bool isOpen()							{ return m_fh ? True : False; }

	Void readOpen(cpStr filename);
	Void writeOpen(cpStr filename);
	Void close();
	Int read(pUChar pbuf, Int length);
	Int readLine(pStr pbuf, Int length);
	Int write(pUChar pbuf, Int length);

private:
	FTString	m_filename;
	FILE*		m_fh;
	BZFILE*		m_bfh;
	Char		m_term;

	Int			m_bzerror;
	Operation	m_operation;
	ULongLong	m_bytesin;
	ULongLong	m_bytesout;
	Int			m_len;
	Int			m_ofs;
	UChar		m_buf[65536];
};

#endif // #define __ftbzip2_h_included
