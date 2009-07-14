#include <io.h>
#include <stdio.h>

void* signal(int sig, void* func)
{
	return (void*)sig;
}

int raise(int sig)
{
	return 0;
}
