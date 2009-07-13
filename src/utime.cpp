#include <winsock2.h>
#include <sys/timeb.h>
#include <sys/utime.h>

int _utime(const char *f, struct utimbuf *t)
{
	HANDLE h;
	FILETIME atime={0}, mtime={0};
	__int64 time64;
	int rc;
	WCHAR wf [MAX_PATH] = {0};

	mbstowcs(wf,f,MAX_PATH);

	h = CreateFileW(wf, GENERIC_WRITE, 
		FILE_SHARE_READ|FILE_SHARE_WRITE,
		NULL, OPEN_EXISTING, 0, 0);

	if( h==INVALID_HANDLE_VALUE )
		return -1;

	time64 = Int32x32To64(t->actime, 10000000) + 116444736000000000;
	atime.dwLowDateTime  = (DWORD)time64;
	atime.dwHighDateTime = (DWORD)(time64 >> 32);
	time64 = Int32x32To64(t->modtime, 10000000) + 116444736000000000;
	mtime.dwLowDateTime  = (DWORD)time64;
	mtime.dwHighDateTime = (DWORD)(time64 >> 32);

	rc = SetFileTime(h, NULL, &atime, &mtime);
	return rc==TRUE ? 0 : -1;
}
