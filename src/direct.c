#include <io.h>
#include <stdio.h>
#include <errno.h>
#include <winsock2.h>

char cwd[MAX_PATH+1] = "";
char *_getcwd(char *buffer, int maxlen)
{
	TCHAR fileUnc[MAX_PATH+1];
	char* plast;

	if(cwd[0] == 0)
	{
		GetModuleFileName(NULL, fileUnc, MAX_PATH);
		WideCharToMultiByte(CP_ACP, 0, fileUnc, -1, cwd, MAX_PATH, NULL, NULL);
		plast = strrchr(cwd, '\\');
		if(plast)
			*plast = 0;
		/* Special trick to keep start menu clean... */
		if(_stricmp(cwd, "\\windows\\start menu") == 0)
			strcpy(cwd, "\\Apps");
	}
	if(buffer)
		strncpy(buffer, cwd, maxlen);
	return cwd;
}

int _rmdir(const char * dir)
{
	WCHAR wd [MAX_PATH] = {0};
	int rc;
	mbstowcs (wd,dir,MAX_PATH);
	/* replace with RemoveDirectory. */
	rc = RemoveDirectory(wd);

	return rc==TRUE ? 0 : -1;
}

/* Make directory, Unix style */
int _mkdir(char* dirname, int mode)
{
	char  path[MAX_PATH+1];
	TCHAR pathUnc[MAX_PATH+1];
	char* ptr;
	int res = 0;
	strncpy(path, dirname, MAX_PATH);
	if(*path == '/')
		*path = '\\';
	/* Run through the string and attempt creating all subdirs on the path */
	for(ptr = path+1; *ptr; ptr ++)
	{
		if(*ptr == '\\' || *ptr == '/')
		{
			*ptr = 0;
			MultiByteToWideChar(CP_ACP, 0, path, -1, pathUnc, MAX_PATH);
			res = CreateDirectory(pathUnc, 0);
			*ptr = '\\';
		}
	}
	MultiByteToWideChar(CP_ACP, 0, path, -1, pathUnc, MAX_PATH);
	res = CreateDirectory(pathUnc, 0);
	if (res == 0)
		return -1;
	return 0;
}
