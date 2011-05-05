/* 

checks if merging of freed blocks seems to work correctly

Author: Robert R�nngren 050218

*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>


#define SIZE 16384

int main(int argc, char * argv[])
{
  
  char *p1, *p2,  *oldBrk, *newBrk1, *newBrk2;
  unsigned long largeSize = SIZE;

  oldBrk = (char *)sbrk(0);

  p1 = (char *)malloc(1);

  newBrk1 = (char *) sbrk(0);

  largeSize = ((unsigned long)(newBrk1-oldBrk) > SIZE ? (unsigned long)(newBrk1-oldBrk) : SIZE);

  free(p1);

  printf("-- Testing merging of deallocated large blocks ( >= %u bytes)\n", (unsigned)largeSize);

  p1 = (char *)malloc(largeSize);
  p2 = (char *)malloc(largeSize);

  if(p1 == NULL || p2 == NULL) printf("* ERROR: unable to allocate memory of size %u bytes\n", (unsigned) largeSize);

  newBrk1 = (char *)sbrk(0);

  free(p1);
  free(p2);

  p1 = (char *)malloc(largeSize * 2);

  if(p1 == NULL) printf("* ERROR: unable to allocate memory of size %u bytes\n", (unsigned)largeSize);

  newBrk2 = (char *)sbrk(0);

  if(((double)(newBrk2 - oldBrk))/((double)(newBrk1 - oldBrk)) > 1)
    printf("* ERROR: not good enough usage of memory (probably incorrect merging of deallocated memory blocks)\nYour implementation used %u bytes more than expected\n", ((unsigned)(newBrk2-newBrk1)));
  else
    printf("Test passed OK\n");
  exit(0);
}
