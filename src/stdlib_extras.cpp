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


#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <ctype.h>

int _fmode;		/* default file translation mode */


void abort(void)
{
	exit(errno);
}


void* bsearch(const void* key, const void* base, size_t nmemb, size_t size,
		int (*compar)(const void*, const void*))
{
	while (nmemb-- > 0)
	{
		if (compar(key, base) == 0)
			return (void*)base;

		base = (char*)base + size;
	}

	return NULL;
}

unsigned __int64 _strtoui64(const char *nptr, char **endptr, int base)
{
	unsigned __int64 v=0;

	while(isspace(*nptr)) ++nptr;

	if (*nptr == '+') ++nptr;
	if (!base) {
		if (*nptr=='0') {
			base=8;
			if ((*(nptr+1)=='x')||(*(nptr+1)=='X')) {
				nptr+=2;
				base=16;
			}
		}
		else
			base=10;
	}
	while(*nptr) {
		register unsigned char c=*nptr;
		c=(c>='a'?c-'a'+10:c>='A'?c-'A'+10:c-'0');
		if (c>=base) break;
		v=v*base+c;
		++nptr;
	}
	if (endptr) *endptr=(char *)nptr;
	return v;
}

