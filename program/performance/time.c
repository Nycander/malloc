#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include "../tst.h"
#include "../brk.h"

#define TIMES 10000000

int main(int argc, char *argv[])
{
	int i, *p, *q, *r;
  	size_t pagesize = sysconf(_SC_PAGESIZE);
	for(i = 0; i < TIMES; i++)
	{
		p = malloc(pagesize);
		q = malloc(pagesize * 2 + 1);
		r = malloc(1);
		free(p);
		free(q);
		free(r);
	}
	
	return 0;
}