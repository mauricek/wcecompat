/*  wcecompat: Windows CE C Runtime Library "compatibility" library.
 *
 *  Copyright (C) 2001-2002 Essemer Pty Ltd.  All rights reserved.
 *  http://www.essemer.com.au/
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */


#include <winsock2.h>
#include <sys/timeb.h>

int   timezone, _timezone, altzone;

SYSTEMTIME wce_tm2SYSTEMTIME(struct tm *t)
{
	SYSTEMTIME s;

	s.wYear      = t->tm_year + 1900;
	s.wMonth     = t->tm_mon  + 1;
	s.wDayOfWeek = t->tm_wday;
	s.wDay       = t->tm_mday;
	s.wHour      = t->tm_hour;
	s.wMinute    = t->tm_min;
	s.wSecond    = t->tm_sec;
	s.wMilliseconds = 500;

	return s;
}

FILETIME wce_int642FILETIME(__int64 t)
{
	FILETIME f;

	f.dwHighDateTime = (DWORD)((t >> 32) & 0x00000000FFFFFFFF);
	f.dwLowDateTime  = (DWORD)( t        & 0x00000000FFFFFFFF);
	return f;
}

FILETIME wce_getFILETIMEFromYear(WORD year)
{
	SYSTEMTIME s={0};
	FILETIME f;

	s.wYear      = year;
	s.wMonth     = 1;
	s.wDayOfWeek = 1;
	s.wDay       = 1;
	s.wMilliseconds = 500;

	SystemTimeToFileTime( &s, &f );
	return f;
}

/* __int64 <--> FILETIME */
__int64 wce_FILETIME2int64(FILETIME f)
{
	__int64 t;

	t = f.dwHighDateTime;
	t <<= 32;
	t |= f.dwLowDateTime;
	return t;
}

time_t wce_getYdayFromSYSTEMTIME(const SYSTEMTIME* s)
{
	__int64 t;
	FILETIME f1, f2;

	f1 = wce_getFILETIMEFromYear( s->wYear );
	SystemTimeToFileTime( s, &f2 );

	t = wce_FILETIME2int64(f2)-wce_FILETIME2int64(f1);

	return (time_t)((t/_onesec_in100ns)/(60*60*24));
}

struct tm wce_SYSTEMTIME2tm(SYSTEMTIME *s)
{
	struct tm t;

	t.tm_year  = s->wYear - 1900;
	t.tm_mon   = s->wMonth- 1;
	t.tm_wday  = s->wDayOfWeek;
	t.tm_mday  = s->wDay;
	t.tm_yday  = wce_getYdayFromSYSTEMTIME(s);
	t.tm_hour  = s->wHour;
	t.tm_min   = s->wMinute;
	t.tm_sec   = s->wSecond;
	t.tm_isdst = 0;

	return t;
}

time_t wce_FILETIME2time_t(const FILETIME* f)
{
	FILETIME f1601, f1970;
	__int64 t, offset;

	f1601 = wce_getFILETIMEFromYear(1601);
	f1970 = wce_getFILETIMEFromYear(1970);

	offset = wce_FILETIME2int64(f1970) - wce_FILETIME2int64(f1601);

	t = wce_FILETIME2int64(*f);

	t -= offset;
	return (time_t)(t / _onesec_in100ns);
}

int ftime(struct timeb *tp)
{
	SYSTEMTIME s;
	FILETIME   f;

	GetLocalTime(&s);
	SystemTimeToFileTime( &s, &f );
	s.wMilliseconds = 500;/// no milliseconds, someone could complain
	tp->dstflag  = 0;
	tp->timezone = _timezone/60;
	tp->time     = wce_FILETIME2time_t(&f);
	tp->millitm  = s.wMilliseconds; 

	return 0;
}