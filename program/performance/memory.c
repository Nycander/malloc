#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include "../tst.h"
#include "../brk.h"

#include <stdio.h>

#define TIMES 1200
#define MAXSIZE 16384

void test(int size)
{
	int i, expected_size, actual_size;
	char *m[TIMES];
  	void *start, *stop;
	unsigned long expected_sum = 0, actual_sum = 0;

	for(i = 0; i < TIMES; i++)
	{
		expected_size = size > 0 ? size : rand()%MAXSIZE;
		expected_sum += expected_size;
		start = sbrk(0);
		m[i] = malloc(expected_size);
		stop = sbrk(0);
		actual_size = (int)(stop - start);
		actual_sum += actual_size;

		if (m[i] == NULL)
		{
			fprintf(stderr, "OH FUCK\n");
			exit(12);
		}
	}

	printf("%2.4f\t", (1.0*actual_sum/(TIMES*1.0))/(1.0*expected_sum/(TIMES*1.0)));

	for(i = 0; i < TIMES; i++)
		free(m[i]);
}

int main(int argc, char *argv[])
{
	/*
	int pagesize = sysconf(_SC_PAGESIZE);
	fprintf(stderr, "page size = %d\n", pagesize);
	 page size @ u3.csc.kth.se = 4096
	*/
  	test(atoi(argv[1]));

	return 0;
}
