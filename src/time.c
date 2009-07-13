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


#include <time.h>
#include "sys/timeb.h"
#include <stdio.h>
#include <winsock2.h>
#include <errno.h>
#include "internal.h"

// Contains the time zone string
//
char tz_name[2][32];

static const FILETIME	ftJan1970 = {3577643008,27111902};

static struct tm st_tm;

// Contains the days of the week abreviation
//
static char *aday[] = {
	"", "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"
};

// Contains the days of the week full name
//
static char *day[] = {
	"", "Sunday", "Monday", "Tuesday", "Wednesday",
	"Thursday", "Friday", "Saturday"
};

// Contains the months of the year abreviation
//
static char *amonth[] = {
	"", "Jan", "Feb", "Mar", "Apr", "May", "Jun",
	"Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
};

// Contains the months of the year full name
//
static char *month[] = {
	"", "January", "February", "March", "April", "May", "June",
	"July", "August", "September", "October", "November", "December"
};

#define DELTA_EPOCH_IN_MICROSECS  11644473600000000ULL

// Compare two 64-bit values.
// Returns:
//   <0 if a is less than b
//   0  if a is equal to b
//   >0 if a is greater than b
static int cmp64(const ULARGE_INTEGER* a, const ULARGE_INTEGER* b)
{
	if (a == NULL || b == NULL)
		return -1;	// error but no error return value

	if (a->HighPart < b->HighPart)
		return -1;
	else if (a->HighPart == b->HighPart)
	{
		if (a->LowPart < b->LowPart)
			return -1;
		else if (a->LowPart == b->LowPart)
			return 0;
		else	// if (a->LowPart > b->LowPart)
			return 1;
	}
	else	// if (a->HighPart < b->HighPart)
		return 1;
}


// Returns position of top non-zero bit,
// eg. -1 = none set, 0 = first bit, 31 = top bit
static int topBit(DWORD value)
{
	if (value == 0)
		return -1;
	else if (value & 0xffff0000)
	{	// bit in 0xffff0000 is set
		if (value & 0xff000000)
		{	// bit in 0xff000000 is set
			if (value & 0xf0000000)
			{	// bit in 0xf0000000 is set
				if (value & 0xc0000000)
				{	// bit in 0xc0000000 is set
					if (value & 0x80000000)
						return 31;
					else
						return 30;
				}
				else
				{	// bit in 0x30000000 is set
					if (value & 0x20000000)
						return 29;
					else
						return 28;
				}
			}
			else
			{	// bit in 0x0f000000 is set
				if (value & 0x0c000000)
				{	// bit in 0x0c000000 is set
					if (value & 0x08000000)
						return 27;
					else
						return 26;
				}
				else
				{	// bit in 0x03000000 is set
					if (value & 0x02000000)
						return 25;
					else
						return 24;
				}
			}
		}
		else
		{	// bit in 0x00ff0000 is set
			if (value & 0x00f00000)
			{	// bit in 0x00f00000 is set
				if (value & 0x00c00000)
				{	// bit in 0x00c00000 is set
					if (value & 0x00800000)
						return 23;
					else
						return 22;
				}
				else
				{	// bit in 0x00300000 is set
					if (value & 0x00200000)
						return 21;
					else
						return 20;
				}
			}
			else
			{	// bit in 0x000f0000 is set
				if (value & 0x000c0000)
				{	// bit in 0x000c0000 is set
					if (value & 0x00080000)
						return 19;
					else
						return 18;
				}
				else
				{	// bit in 0x00030000 is set
					if (value & 0x00020000)
						return 17;
					else
						return 16;
				}
			}
		}
	}
	else
	{	// bit in 0x0000ffff is set
		if (value & 0x0000ff00)
		{	// bit in 0x0000ff00 is set
			if (value & 0x0000f000)
			{	// bit in 0x0000f000 is set
				if (value & 0x0000c000)
				{	// bit in 0x0000c000 is set
					if (value & 0x00008000)
						return 15;
					else
						return 14;
				}
				else
				{	// bit in 0x00003000 is set
					if (value & 0x00002000)
						return 13;
					else
						return 12;
				}
			}
			else
			{	// bit in 0x00000f00 is set
				if (value & 0x00000c00)
				{	// bit in 0x00000c00 is set
					if (value & 0x00000800)
						return 11;
					else
						return 10;
				}
				else
				{	// bit in 0x00000300 is set
					if (value & 0x00000200)
						return 9;
					else
						return 8;
				}
			}
		}
		else
		{	// bit in 0x000000ff is set
			if (value & 0x000000f0)
			{	// bit in 0x000000f0 is set
				if (value & 0x000000c0)
				{	// bit in 0x000000c0 is set
					if (value & 0x00000080)
						return 7;
					else
						return 6;
				}
				else
				{	// bit in 0x00000030 is set
					if (value & 0x00000020)
						return 5;
					else
						return 4;
				}
			}
			else
			{	// bit in 0x0000000f is set
				if (value & 0x0000000c)
				{	// bit in 0x0000000c is set
					if (value & 0x00000008)
						return 3;
					else
						return 2;
				}
				else
				{	// bit in 0x00000003 is set
					if (value & 0x00000002)
						return 1;
					else
						return 0;
				}
			}
		}
	}
}


// Returns position of top non-zero bit,
// eg. -1 = none set, 0 = first bit, 63 = top bit
static int topBit64(ULARGE_INTEGER* value)
{
	int		result;

	if (value == NULL)
		return 0;

	result = topBit(value->HighPart);
	if (result != -1)
		return result+32;
	else
		return topBit(value->LowPart);
}


static void shl64(ULARGE_INTEGER* value, int shift)
{
	ULARGE_INTEGER	result;

	if (value == NULL || shift <= 0)
		return;

	result.HighPart = (value->HighPart << shift) | (value->LowPart >> (32-shift));
	result.LowPart = (value->LowPart << shift);

	value->HighPart = result.HighPart;
	value->LowPart = result.LowPart;
}


static void shr64(ULARGE_INTEGER* value, int shift)
{
	ULARGE_INTEGER	result;

	if (value == NULL || shift <= 0)
		return;

	result.HighPart = (value->HighPart >> shift);
	result.LowPart = (value->LowPart >> shift) | (value->HighPart << (32-shift));

	value->HighPart = result.HighPart;
	value->LowPart = result.LowPart;
}


// Add valueToAdd to value (doesn't handle overflow)
static void add64(ULARGE_INTEGER* value, ULARGE_INTEGER* valueToAdd)
{
	if (value == NULL || valueToAdd == NULL)
		return;

	value->LowPart += valueToAdd->LowPart;
	if (value->LowPart < valueToAdd->LowPart)
		value->HighPart++;	// carry to HighPart
	value->HighPart += valueToAdd->HighPart;
}


// Subtract valueToSubtract from value (doesn't handle underflow)
static void sub64(ULARGE_INTEGER* value, ULARGE_INTEGER* valueToSubtract)
{
	if (value == NULL || valueToSubtract == NULL)
		return;

	if (value->LowPart < valueToSubtract->LowPart)
		value->HighPart--;	// borrow from HighPart
	value->HighPart -= valueToSubtract->HighPart;
	value->LowPart -= valueToSubtract->LowPart;
}


static void mul64(ULARGE_INTEGER* value, DWORD multiplier)
{
	ULARGE_INTEGER	result = { 0, 0 };
	ULARGE_INTEGER	temp;
	WORD			multiplierHigh = (WORD)(multiplier >> 16);
	WORD			multiplierLow = (WORD)(multiplier & 0xffff);

	if (value == NULL)
		return;

	//
	// split the DWORD in two, and multiply seperately to avoid overflow
	//

	// firstly, the lower 16-bits of the multiplier

	temp.HighPart = 0;
	temp.LowPart = (value->LowPart & 0xffff) * multiplierLow;
	add64(&result, &temp);

	temp.LowPart = (value->LowPart >> 16) * multiplierLow;
	shl64(&temp, 16);
	add64(&result, &temp);

	temp.LowPart = 0;
	temp.HighPart = (value->HighPart & 0xffff) * multiplierLow;
	add64(&result, &temp);

	temp.HighPart = (value->HighPart >> 16) * multiplierLow;
	shl64(&temp, 16);
	add64(&result, &temp);

	// secondly, the higher 16-bits of the multiplier

	temp.HighPart = 0;
	temp.LowPart = (value->LowPart & 0xffff) * multiplierHigh;
	shl64(&temp, 16);
	add64(&result, &temp);

	temp.LowPart = (value->LowPart >> 16) * multiplierHigh;
	shl64(&temp, 32);
	add64(&result, &temp);

	temp.LowPart = 0;
	temp.HighPart = (value->HighPart & 0xffff) * multiplierHigh;
	shl64(&temp, 16);
	add64(&result, &temp);

	temp.HighPart = (value->HighPart >> 16) * multiplierHigh;
	shl64(&temp, 32);
	add64(&result, &temp);

	// return the result
	value->HighPart = result.HighPart;
	value->LowPart = result.LowPart;
}


static void div64(ULARGE_INTEGER* value, DWORD divisor)
{
	ULARGE_INTEGER	result = { 0, 0 };
	ULARGE_INTEGER	shiftedDivisor;	// divisor shifted to left
	ULARGE_INTEGER	shiftedOne;		// '1' shifted to left by same number of bits as divisor
	int				shift;

	if (value == NULL)
		return;
	if (divisor == 0)
	{
		value->LowPart = 0;
		value->HighPart = 0;
		return;
	}
	if (value->HighPart == 0)
	{
		if (value->LowPart != 0)
			value->LowPart /= divisor;
		return;
	}

	// shift divisor up (into shifted) as far as it can go before it is greater than value
	shift = topBit64(value) - topBit(divisor);
	shiftedDivisor.LowPart = divisor;
	shiftedDivisor.HighPart = 0;
	shiftedOne.LowPart = 1;
	shiftedOne.HighPart = 0;
	shl64(&shiftedDivisor, shift);
	shl64(&shiftedOne, shift);
	while (shift >= 0)
	{
		if (cmp64(&shiftedDivisor, value) <= 0)
		{
			add64(&result, &shiftedOne);
			sub64(value, &shiftedDivisor);
		}
		shr64(&shiftedDivisor, 1);
		shr64(&shiftedOne, 1);
		shift--;
	}

	value->HighPart = result.HighPart;
	value->LowPart = result.LowPart;
}

// Convert Win32 FILETIME into time_t
time_t w32_filetime_to_time_t(FILETIME* ft)
{
	// make sure ft is at least ftJan1970
	if (cmp64((ULARGE_INTEGER*)ft, (ULARGE_INTEGER*)&ftJan1970) < 0)
	{
		errno = -1;
		return -1;
	}

	// subtract ftJan1970 from ft
	sub64((ULARGE_INTEGER*)ft, (ULARGE_INTEGER*)&ftJan1970);

	// divide ft by 10,000,000 to convert from 100-nanosecond units to seconds
	div64((ULARGE_INTEGER*)ft, 10000000);

	// bound check result
	if (ft->dwHighDateTime != 0 || ft->dwLowDateTime >= 2147483648)
	{
		errno = -1;
		return -1;		// value is too big to return in time_t
	}

	return (time_t)ft->dwLowDateTime;
}


time_t time(time_t* t)
{
	SYSTEMTIME		stNow;
	FILETIME		ftNow;
	time_t			tt;

	// get system time
	GetSystemTime(&stNow);
	stNow.wMilliseconds = 500;
	if (!SystemTimeToFileTime(&stNow, &ftNow))
	{
		errno = -1;
		return -1;
	}

	tt = w32_filetime_to_time_t(&ftNow);

	if (t != NULL)
		*t = tt;
	return tt;
}


clock_t clock(void)
{
	return (clock_t)GetTickCount();
}


void UnixTimeToFileTime(time_t t, LPFILETIME pft)
{
	// Note that LONGLONG is a 64-bit value
	LONGLONG ll;

	ll = Int32x32To64(t, 10000000) + 116444736000000000;
	pft->dwLowDateTime = (DWORD)ll;
	pft->dwHighDateTime = ll >> 32;
}

// Returns to day of the year (0..365)
// Year: eg. 2001
// Month: 0.11
// Day: 1..31
int dayOfYear(int year, int month, int day)
{
	int		result;
	BOOL	isLeapYear = FALSE;

	if ((year % 4 == 0) && !(year % 100 == 0) && (year % 400 == 0))
		isLeapYear = TRUE;

	result = day-1;
	if (month > 0)
		result += 31;	// Jan
	if (month > 1)
		result += 28 + (isLeapYear ? 1 : 0);
	if (month > 2)
		result += 31;	// Mar
	if (month > 3)
		result += 30;	// Apr
	if (month > 4)
		result += 31;	// May
	if (month > 5)
		result += 30;	// Jun
	if (month > 6)
		result += 31;	// Jul
	if (month > 7)
		result += 31;	// Aug
	if (month > 8)
		result += 30;	// Sep
	if (month > 9)
		result += 31;	// Oct
	if (month > 10)
		result += 30;	// Nov

	return result;
}

struct tm* localtime(const time_t* clock)
{
	FILETIME				ftUtc;
	FILETIME				ftLocal;
	SYSTEMTIME				stLocal;
	TIME_ZONE_INFORMATION	tzi;
	DWORD					tziResult;

	if (clock == NULL)
		return NULL;

	// convert time_t to FILETIME
	UnixTimeToFileTime(*clock,&ftUtc);
	
	// convert to local FILETIME
	if (!FileTimeToLocalFileTime(&ftUtc, &ftLocal))
		return NULL;

	// convert to SYSTEMTIME
	if (!FileTimeToSystemTime(&ftLocal, &stLocal))
		return NULL;
	stLocal.wMilliseconds = 500;
	
	// determine if we're operating in daylight savings time
	tziResult = GetTimeZoneInformation(&tzi);

	// fill return structure
	st_tm.tm_sec = stLocal.wSecond;
	st_tm.tm_min = stLocal.wMinute;
	st_tm.tm_hour = stLocal.wHour;
	st_tm.tm_mday = stLocal.wDay;
	st_tm.tm_mon = stLocal.wMonth;
	st_tm.tm_year = stLocal.wYear;
	st_tm.tm_wday = stLocal.wDayOfWeek;
	st_tm.tm_yday = dayOfYear(stLocal.wYear, stLocal.wMonth-1, stLocal.wDay);
	if (tziResult == TIME_ZONE_ID_UNKNOWN)
		st_tm.tm_isdst = -1;
	else if (tziResult == TIME_ZONE_ID_STANDARD)
		st_tm.tm_isdst = 0;
	else if (tziResult == TIME_ZONE_ID_DAYLIGHT)
		st_tm.tm_isdst = 1;

	return &st_tm;
}


struct tm* gmtime(const time_t* clock)
{
	FILETIME	ftUtc;
	SYSTEMTIME stUtc;

	if (clock == NULL)
		return NULL;

	// convert time_t to FILETIME
	UnixTimeToFileTime(*clock,&ftUtc);

	// convert to SYSTEMTIME
	if (!FileTimeToSystemTime(&ftUtc, &stUtc))
		return NULL;

	stUtc.wMilliseconds = 500;
	// fill return structure
	st_tm.tm_sec = stUtc.wSecond;
	st_tm.tm_min = stUtc.wMinute;
	st_tm.tm_hour = stUtc.wHour;
	st_tm.tm_mday = stUtc.wDay;
	st_tm.tm_mon = stUtc.wMonth;
	st_tm.tm_year = stUtc.wYear;
	st_tm.tm_wday = stUtc.wDayOfWeek;
	st_tm.tm_yday = dayOfYear(stUtc.wYear, stUtc.wMonth-1, stUtc.wDay);
	st_tm.tm_isdst = 0;

	return &st_tm;

}

// static void strfmt(char *str, char *fmt);
// 
// simple sprintf for strftime
// 
// each format descriptor is of the form %n
// where n goes from zero to four
// 
// 0    -- string %s
// 1..4 -- int %?.?d
// 
static void strfmt(char *str, const char *fmt, ...)
{
	int ival, ilen;
	char *sval;
	static int pow[5] = { 1, 10, 100, 1000, 10000 };
	va_list vp;

	va_start(vp, fmt);
	while (*fmt)
	{
		if (*fmt++ == '%')
		{
			ilen = *fmt++ - '0';
			if (ilen == 0)                // zero means string arg
			{
				sval = va_arg(vp, char*);
				while (*sval)
					*str++ = *sval++;
			}
			else                          // always leading zeros
			{
				ival = va_arg(vp, int);
				while (ilen)
				{
					ival %= pow[ilen--];
					*str++ = (char)('0' + ival / pow[ilen]);
				}
			}
		}
		else  *str++ = fmt[-1];
	}
	*str = '\0';
	va_end(vp);
}

size_t strftime(char *s, size_t maxs, const char *f, const struct tm *t)
{
	int			w;
	char		*p, *q, *r;
	static char	buf[26];

	p = s;
	q = s + maxs - 1;
	while ((*f != '\0'))
	{
		if (*f++ == '%')
		{
			r = buf;
			switch (*f++)
			{
			case '%' :
				r = "%";
				break;

			case 'a' :
				r = aday[t->tm_wday];
				break;

			case 'A' :
				r = day[t->tm_wday];
				break;

			case 'b' :
				r = amonth[t->tm_mon];
				break;

			case 'B' :
				r = month[t->tm_mon];
				break;

			case 'c' :
				strfmt(r, "%0 %0 %2 %2:%2:%2 %4",
					aday[t->tm_wday], amonth[t->tm_mon],
					t->tm_mday,t->tm_hour, t->tm_min,
					t->tm_sec, t->tm_year);
				break;

			case 'd' :
				strfmt(r,"%2",t->tm_mday);
				break;

			case 'H' :
				strfmt(r,"%2",t->tm_hour);
				break;

			case 'I' :
				strfmt(r,"%2",(t->tm_hour%12)?t->tm_hour%12:12);
				break;

			case 'j' :
				strfmt(r,"%3",t->tm_yday);
				break;

			case 'm' :
				strfmt(r,"%2",t->tm_mon);
				break;

			case 'M' :
				strfmt(r,"%2",t->tm_min);
				break;

			case 'p' :
				r = (t->tm_hour>11)?"PM":"AM";
				break;

			case 'S' :
				strfmt(r,"%2",t->tm_sec);
				break;

			case 'U' :
				w = t->tm_yday/7;
				if (t->tm_yday%7 > t->tm_wday)
					w++;
				strfmt(r, "%2", w);
				break;

			case 'W' :
				w = (t->tm_yday + DAYSPERWEEK -
					(t->tm_wday ?
					(t->tm_wday - 1) :
				(DAYSPERWEEK - 1))) / DAYSPERWEEK;
				strfmt(r, "%2", w);
				break;

			case 'w' :
				strfmt(r,"%1",t->tm_wday);
				break;

			case 'x' :
				strfmt(r, "%2/%2/%2", t->tm_mon,
					t->tm_mday, t->tm_year);
				break;

			case 'X' :
				strfmt(r, "%2:%2:%2", t->tm_hour,
					t->tm_min, t->tm_sec);
				break;

			case 'y' :
				strfmt(r,"%2",t->tm_year%100);
				break;

			case 'Y' :
				strfmt(r,"%4",t->tm_year);
				break;

			case 'Z' :
				r = (t->tm_isdst && tz_name[1][0])?tz_name[1]:tz_name[0];
				break;

			default:
				buf[0] = '%';		// reconstruct the format
				buf[1] = f[-1];
				buf[2] = '\0';
				if (buf[1] == 0)
					f--;			// back up if at end of string
			}
			while (*r)
			{
				if (p == q)
				{
					*q = '\0';
					return 0;
				}
				*p++ = *r++;
			}
		}
		else
		{
			if (p == q)
			{
				*q = '\0';
				return 0;
			}
			*p++ = f[-1];
		}
	}
	*p = '\0';
	return p - s;
}

time_t mktime(struct tm* pt)
{
	SYSTEMTIME ss, ls, s;
	FILETIME   sf, lf, f;
	__int64 diff;

	GetSystemTime(&ss);
	GetLocalTime(&ls);
	SystemTimeToFileTime( &ss, &sf );
	SystemTimeToFileTime( &ls, &lf );

	diff = (wce_FILETIME2int64(lf)-wce_FILETIME2int64(sf))/_onesec_in100ns;

	s = wce_tm2SYSTEMTIME(pt);
	SystemTimeToFileTime( &s, &f );
	return wce_FILETIME2time_t(&f) - (time_t)diff;
}

void _tzset ()
{

}

int gettimeofday(struct timeval *tv, struct timezone *tz)
{
	FILETIME ft;
	unsigned __int64 tmpres = 0;
	SYSTEMTIME ls;
	if (NULL != tv)
	{
		GetLocalTime(&ls); /// or GetSystemTime should be used here ?!
		ls.wMilliseconds = 500;
		SystemTimeToFileTime( &ls, &ft );

		tmpres |= ft.dwHighDateTime;
		tmpres <<= 32;
		tmpres |= ft.dwLowDateTime;

		/*converting file time to unix epoch*/
		tmpres /= 10;  /*convert into microseconds*/
		tmpres -= DELTA_EPOCH_IN_MICROSECS; 
		tv->tv_sec = (long)(tmpres / 1000000UL);
		tv->tv_usec = (long)(tmpres % 1000000UL);
	}

	return 0;
}