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


#include <io.h>
#include <stdio.h>
#include <errno.h>
#include <winsock2.h>
#include <fcntl.h>

int _wcelocking (int _FileHandle, int _LockMode, long _NumOfBytes)
{
	/// not supported on WINCE
	return 0;
}

int _wcerename(const char *oldname, const char *newname)
{
	WCHAR wold[MAX_PATH]={0};
	WCHAR wnew [MAX_PATH]={0};	
	int rc;

	mbstowcs (wold,oldname,MAX_PATH);
	mbstowcs (wnew,newname,MAX_PATH);


	/* replace with MoveFile. */
	rc = MoveFileW(wold, wnew);

	return rc==TRUE ? 0 : -1;
}

int _wceaccess(const char *filename, int flags)
{
	WCHAR fname [MAX_PATH];
	WCHAR modero [4] = L"rb";
	WCHAR modewo [4] = L"wb";
	WCHAR moderw [4] = L"w+";
	FILE* f = NULL;
	
	mbstowcs(fname,filename,MAX_PATH);	
	if (flags == 0 || flags == 4)
	{
		f = _wfopen(fname,modero);
	}
	else if (flags == 2)
	{
		f = _wfopen(fname,modewo);
	}
	else if (flags == 6)
	{
		f = _wfopen(fname,moderw);
	}

	if (f)
	{
		fclose (f);
		return 0;
	}

	return -1;
}

/* Remove file by name */
int _wceremove(const char* path)
{
	TCHAR pathUnc[MAX_PATH+1];
	MultiByteToWideChar(CP_ACP, 0, path, -1, pathUnc, MAX_PATH);
	return !DeleteFile(pathUnc);
}

/* in our case unlink is the same as remove */
int _wceunlink(const char* path)
{
	return _wceremove(path);
}

int _wceopen(const char *file, int mode, int pmode)
{
	/// TODO : since this function do not support parameters checking (i.e. would be regular to specifi O_CREATE and O_APPEND together), 
	/// use with caution referring to documentation 
	WCHAR wfile [MAX_PATH] = {0};
	DWORD access=GENERIC_READ, share=FILE_SHARE_READ|FILE_SHARE_WRITE, create=OPEN_EXISTING, attr=FILE_ATTRIBUTE_NORMAL;
	DWORD err = 0;
	HANDLE h;

	if( (mode & O_RDWR) || (mode & O_APPEND))
	{
		access = GENERIC_READ|GENERIC_WRITE;
	}
	
	if( (mode & O_WRONLY))
	{		
		access = GENERIC_WRITE;
	}

	if( (mode & O_TRUNC))
	{
		access = GENERIC_WRITE;
		create = TRUNCATE_EXISTING;
	}

	if (mode & O_NOINHERIT)
	{
		share = 0;
	}
	
	if (mode & O_TEMPORARY)
	{
		attr = FILE_ATTRIBUTE_TEMPORARY;
	}
	
	if (mode & _O_SHORT_LIVED)
	{
		attr = FILE_ATTRIBUTE_TEMPORARY|FILE_FLAG_DELETE_ON_CLOSE;
	}
	
	if( (mode & O_CREAT))
	{
			create = CREATE_ALWAYS;
	}
	
	if (mode & O_RANDOM)
	{
		attr |= FILE_FLAG_RANDOM_ACCESS;
	}
	
	if (mode & O_SEQUENTIAL)
	{
		attr |= FILE_FLAG_SEQUENTIAL_SCAN;
	}
	
	mbstowcs(wfile,file,MAX_PATH);
	h = CreateFileW(wfile, access, share, NULL, create, attr, NULL );
	if (h == INVALID_HANDLE_VALUE)
	{
		err = GetLastError();
		if (err == ERROR_FILE_NOT_FOUND || err == ERROR_PATH_NOT_FOUND)
			errno = ENOENT;
		else if (err == ERROR_ACCESS_DENIED)
			errno = EACCES;
		else if (err == ERROR_ALREADY_EXISTS)
			errno = EEXIST;
		return -1;
	}
	else
	{
		if( (mode & O_EXCL))
		{
				CloseHandle(h);
				errno = EEXIST;
				return -1;
		}	
	}
	return (int)h;
}

FILE* _wcefdopen( int handle, const char *mode )
{
	WCHAR wmode [32];
	FILE* fp = NULL;

	mbstowcs(wmode,mode,32);
	fp = _wfdopen( (void*)handle, wmode );

	return fp;
}

int _wceclose (int fd)
{	
	if ((HANDLE)fd != INVALID_HANDLE_VALUE)
	{
		if (CloseHandle((HANDLE)fd))
			return 0;
	}
	return -1;
}

int _wceread(int fd, void *buffer, int length)
{
	DWORD dw = 0;
	ReadFile( (HANDLE)fd, buffer, length, &dw, NULL );
	return (int)dw;
}

int _wcewrite(int fd, const void *buffer, unsigned count)
{
	DWORD dw;
	WriteFile( (HANDLE)fd, buffer, count, &dw, NULL );
	return (int)dw;
}

long _wcelseek(int handle, long offset, int origin)
{
	DWORD flag, ret;

	switch(origin)
	{
		case SEEK_SET: 
			flag = FILE_BEGIN;   
		break;
		case SEEK_CUR: 
			flag = FILE_CURRENT; 
		break;
		case SEEK_END: 
			flag = FILE_END;     
		break;
		
		default:       
			flag = FILE_CURRENT; 
		break;
	}

	ret = SetFilePointer( (HANDLE)handle, offset, NULL, flag );
	if (ret == -1)
	{
		if (GetLastError () != NO_ERROR)
			return -1;
	}
	return ret;
}

