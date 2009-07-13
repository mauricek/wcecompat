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


#include <sys/stat.h>
#include <stdio.h>
#include <errno.h>
#include "ts_string.h"
#include "internal.h"
#include <sys/timeb.h>

/* Very limited implementation of stat. Used by UI.C, MEMORY-P.C (latter is not critical) */
int stat(const char *fname, struct stat *ss)
{
	TCHAR fnameUnc[MAX_PATH+1];
	HANDLE handle;
	WIN32_FIND_DATA wfd;
	int len;

	if(fname == NULL || ss == NULL)
		return -1;

	/* Special case (dummy on WinCE) */
	len = strlen(fname);
	if(len >= 2 && fname[len-1] == '.' && fname[len-2] == '.' &&
		(len == 2 || fname[len-3] == '\\'))
	{
		/* That's everything implemented so far */
		memset(ss, 0, sizeof(struct stat));
		ss->st_size = 1024;
		ss->st_mode |= S_IFDIR;
		return 0;
	}

	MultiByteToWideChar(CP_ACP, 0, fname, -1, fnameUnc, MAX_PATH);
	handle = FindFirstFile(fnameUnc, &wfd);
	if(handle == INVALID_HANDLE_VALUE)
	{
		errno = ENOENT;
		return ENOENT;
	}
	else
	{
		/* That's everything implemented so far */
		memset(ss, 0, sizeof(struct stat));
		ss->st_size = wfd.nFileSizeLow;
		if(wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			ss->st_mode |= S_IFDIR;
		else
			ss->st_mode |= S_IFREG;

		FindClose(handle);
	}
	return 0;
}

int fstat(int file, struct stat *sbuf)
{
	/* GetFileSize & GetFileTime */
	DWORD dwSize;
	FILETIME ctime, atime, mtime;

	dwSize = GetFileSize( (HANDLE)file, NULL );
	if( dwSize == 0xFFFFFFFF )
		return -1;

	sbuf->st_size = dwSize;
	sbuf->st_dev  = 0;
	sbuf->st_rdev = 0;
	sbuf->st_mode = _S_IFREG;
	sbuf->st_nlink= 1;

	GetFileTime( (HANDLE)file, &ctime, &atime, &mtime );
	sbuf->st_ctime = wce_FILETIME2time_t(&ctime);
	sbuf->st_atime = wce_FILETIME2time_t(&atime);
	sbuf->st_mtime = wce_FILETIME2time_t(&mtime);

	return 0;
}

