#include <string.h>
#if   STRATEGY==1
	#include "algorithms/firstfit.c"
#elif STRATEGY==2
	#include "algorithms/bestfit.c"
#elif STRATEGY==3
	#include "algorithms/worstfit.c"
#elif STRATEGY==4
	#include "algorithms/quickfit.c"
#else
	#include "algorithms/bookmalloc.c"
#endif

