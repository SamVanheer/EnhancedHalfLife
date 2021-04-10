/***
*
*	Copyright (c) 1999, Valve LLC. All rights reserved.
*	
*	This product contains software technology licensed from Id 
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc. 
*	All Rights Reserved.
*
*   Use, distribution, and modification of this source code and/or resulting
*   object code is restricted to non-commercial enhancements to products from
*   Valve LLC.  All other use, distribution, or modification is prohibited
*   without written permission from Valve LLC.
*
****/
//
//	MDC - copying from cstrike\cl_dll so career-mode stuff can catch messages
//  in this dll. (and C++ifying it)
//

#pragma once

inline byte* gpBuf;
inline int giSize;
inline int giRead;
inline int giBadRead;

//--------------------------------------------------------------------------------------------------------------
inline void BEGIN_READ( void *buf, int size )
{
	giRead = 0;
	giBadRead = 0;
	giSize = size;
	gpBuf = (byte*)buf;
}

inline int READ_CHAR()
{
	int     c;

	if (giRead + 1 > giSize)
	{
		giBadRead = true;
		return -1;
	}

	c = (signed char)gpBuf[giRead];
	giRead++;

	return c;
}

inline int READ_BYTE()
{
	int     c;

	if (giRead + 1 > giSize)
	{
		giBadRead = true;
		return -1;
	}

	c = (unsigned char)gpBuf[giRead];
	giRead++;

	return c;
}

inline int READ_SHORT()
{
	int     c;

	if (giRead + 2 > giSize)
	{
		giBadRead = true;
		return -1;
	}

	c = (short)(gpBuf[giRead] + (gpBuf[giRead + 1] << 8));

	giRead += 2;

	return c;
}

inline int READ_WORD()
{
	return READ_SHORT();
}

inline int READ_LONG()
{
	int     c;

	if (giRead + 4 > giSize)
	{
		giBadRead = true;
		return -1;
	}

	c = gpBuf[giRead] + (gpBuf[giRead + 1] << 8) + (gpBuf[giRead + 2] << 16) + (gpBuf[giRead + 3] << 24);

	giRead += 4;

	return c;
}

inline float READ_FLOAT()
{
	union
	{
		byte    b[4];
		float   f;
		int     l;
	} dat;

	dat.b[0] = gpBuf[giRead];
	dat.b[1] = gpBuf[giRead + 1];
	dat.b[2] = gpBuf[giRead + 2];
	dat.b[3] = gpBuf[giRead + 3];
	giRead += 4;

	//	dat.l = LittleLong (dat.l);

	return dat.f;
}

inline char* READ_STRING()
{
	static char     string[2048];
	int             l, c;

	string[0] = 0;

	l = 0;
	do
	{
		if (giRead + 1 > giSize)
			break; // no more characters

		c = READ_CHAR();
		if (c == -1 || c == 0)
			break;
		string[l] = c;
		l++;
	} while (l < sizeof(string) - 1);

	string[l] = 0;

	return string;
}

inline float READ_COORD()
{
	return (float)(READ_SHORT() * (1.0 / 8));
}

inline float READ_ANGLE()
{
	return (float)(READ_CHAR() * (360.0 / 256));
}

inline float READ_HIRESANGLE()
{
	return (float)(READ_SHORT() * (360.0 / 65536));
}

inline int READ_OK()
{
	return !giBadRead;
}

//--------------------------------------------------------------------------------------------------------------
class BufferWriter
{
public:
	BufferWriter()
	{
		Init(nullptr, 0);
	}
	BufferWriter(unsigned char* buffer, int bufferLen)
	{
		Init(buffer, bufferLen);
	}
	void Init( unsigned char *buffer, int bufferLen )
	{
		m_overflow = false;
		m_buffer = buffer;
		m_remaining = bufferLen;
		m_overallLength = bufferLen;
	}

	void WriteByte( unsigned char data )
	{
		if (!m_buffer || !m_remaining)
		{
			m_overflow = true;
			return;
		}

		*m_buffer = data;
		++m_buffer;
		--m_remaining;
	}
	void WriteLong( int data )
	{
		if (!m_buffer || m_remaining < 4)
		{
			m_overflow = true;
			return;
		}

		m_buffer[0] = data & 0xff;
		m_buffer[1] = (data >> 8) & 0xff;
		m_buffer[2] = (data >> 16) & 0xff;
		m_buffer[3] = data >> 24;
		m_buffer += 4;
		m_remaining -= 4;
	}
	void WriteString( const char *str )
	{
		if (!m_buffer || !m_remaining)
		{
			m_overflow = true;
			return;
		}

		if (!str)
			str = "";

		int len = strlen(str) + 1;
		if (len > m_remaining)
		{
			m_overflow = true;
			str = "";
			len = 1;
		}

		strcpy((char*)m_buffer, str);
		m_remaining -= len;
		m_buffer += len;
	}

	bool HasOverflowed();
	int GetSpaceUsed()
	{
		return m_overallLength - m_remaining;
	}

protected:
	unsigned char *m_buffer;
	int m_remaining;
	bool m_overflow;
	int m_overallLength;
};

//--------------------------------------------------------------------------------------------------------------
