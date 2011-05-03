#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <stdio.h>
#include "tst.h"

#define SIZE 128
#define BIGSIZE 100
#define TIMES 100
#define BIGTIMES 10
#define SMALLSTRING 64
#define TIMESPAGE 10

#define MAX(a,b) ((a > b) ? (a) : (b))

int main(int argc, char *argv[]){
  int i,j;
  float worst;
  char *a[SIZE], *b[BIGSIZE];
  size_t pagesize;
  caddr_t lowbreak, highbreak, maxbreak = 0;
  char *progname;

  if (argc > 0)
    progname = argv[0];
  else
    progname = "";
  /* to generate big memory refs later */
  pagesize = sysconf(_SC_PAGESIZE);

  MESSAGE("Testing memory utility\n");
  lowbreak = sbrk(0);

  MESSAGE("Getting small pieces of memory\n");
  for(i = 0; i < TIMES; i++){
    for(j = 0; j < SIZE; j++){
      a[j] = malloc(SMALLSTRING);
    }
    highbreak = sbrk(0);
    maxbreak = MAX(maxbreak, highbreak);
    for(j = 0; j < SIZE; j++){
      free(a[j]);
    }
    if ( i % 10 == 0 ) 
      fprintf(stderr, "%s: Using total of 0x%x of memory\n",
	      progname, highbreak - lowbreak); 

  }

  worst = (maxbreak - lowbreak)/ (SMALLSTRING * SIZE * 1.0);
  fprintf(stderr, "%s: Using %2.2f times worst case calculation\n", 
	  progname, worst);
  if ( worst > 2 ) {
    MESSAGE("* ERROR: Test indicates excessive memory usage\n");
  } else {
    MESSAGE("Small memory handled OK\n");
  }

  MESSAGE("Getting big blocks of memory\n");
  for(i = 0; i < BIGTIMES; i++){
    for(j = 0; j < BIGSIZE; j++){
      b[j] = malloc(TIMESPAGE*pagesize);
    }
    highbreak = sbrk(0);
    maxbreak = MAX(maxbreak, highbreak);
    for(j = 0; j < BIGSIZE; j++){
      free(b[j]);
    }
    fprintf(stderr, "%s: Using total of 0x%x of memory\n", 
	    progname, highbreak - lowbreak);

  }
  MESSAGE("Allocations versus worst case memory usage:\n");
  worst = (maxbreak - lowbreak)/ (BIGSIZE * pagesize * TIMESPAGE * 1.0);
  fprintf(stderr, 
	  "%s: Using %2.2f times worst case calculation\n", 
	  progname, worst);
  if ( worst > 2 ) {
    MESSAGE("* ERROR: Test indicates excessive memory usage\n");
  } else {
    MESSAGE("Big memory handled OK\n");
  }
  return 0;
}

