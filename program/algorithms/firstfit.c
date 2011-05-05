#include <unistd.h>

#include "../malloc.h"
#include "../brk.h"

#define NALLOC 1024 /* minimum #units to request */

typedef long Align; /* for alignment to long boundary */
union header
{
	/* block header */
	struct
	{
		union header *pointer; /* next block if on free list */
		unsigned size; /* size of this block */
	} s;
	Align x; /* force alignment of blocks */
};

typedef union header Header;

static Header base; /* empty list to get started */
static Header *freelist = NULL;


/* morecore: ask system for more memory */
static Header *morecore(unsigned nu)
{
	char *cp;
	Header *up;
	if (nu < NALLOC)
		nu = NALLOC;
	
	cp = sbrk(nu * sizeof(Header));
	if (cp == (char *) -1) /* no space at all */
		return NULL;
	
	up = (Header *) cp;
	up->s.size = nu;
	free((void *)(up+1));
	return freelist;
}

/* malloc: general-purpose storage allocator */
void *malloc(size_t nbytes)
{
	Header *current_pointer, *previous_pointer;
	unsigned int nunits;

	/* Don't allocate blocks with zero bytes */
	if (nbytes == 0)
	{
		return NULL;
	}

	/* Use number magic to get good roundoff */
	nunits = (nbytes+sizeof(Header)-1)/sizeof(Header) + 1;

	previous_pointer = freelist;

	/* First run, initialize the free list */	
	if (previous_pointer == NULL) 
	{
		base.s.pointer = freelist = previous_pointer = &base;
		base.s.size = 0;
	}

	current_pointer = previous_pointer->s.pointer;

	/* Go through all elements in the free list */
	for (;;)
	{
		/* Is this slot big enough? */
		if (current_pointer->s.size >= nunits) 
		{ 
			/* Is it exactly big enough? */
			if (current_pointer->s.size == nunits) 
			{
				/* Remove the slot */
				previous_pointer->s.pointer = current_pointer->s.pointer;
			}
			else 
			{
				/* Remove bytes from slot */
				current_pointer->s.size -= nunits;
				/* Create a new slot at the end of the big slot */ 
				current_pointer += current_pointer->s.size;
				current_pointer->s.size = nunits;
			}
			freelist = previous_pointer;
			return (void *)(current_pointer+1);
		}
	
		/* Have we looked at all elements in the list? */
		if (current_pointer == freelist)
		{
			/* Ask for more memory! */
			if ((current_pointer = morecore(nunits)) == NULL)
			{
				/* Memory full */
				return NULL;
			}
		}
		
		/* Increment pointers */
		previous_pointer = current_pointer;
		current_pointer = current_pointer->s.pointer
	}
}

/* free: put block ap in free list */
void free(void *ap)
{
	if (ap == NULL)
		return;
	
	Header *bp, *p;
	bp = (Header *)ap - 1; /* point to block header */
	for (p = freelist; !(bp > p && bp < p->s.pointer); p = p->s.pointer)
		if (p >= p->s.pointer && (bp > p || bp < p->s.pointer))
			break; /* freed block at start or end of arena */
	
	if (bp + bp->s.size == p->s.pointer)
	{ 
		/* join to upper nbr */
		bp->s.size += p->s.pointer->s.size;
		bp->s.pointer = p->s.pointer->s.pointer;
	}
	else
		bp->s.pointer = p->s.pointer;
	
	if (p + p->s.size == bp)
	{
		/* join to lower nbr */
		p->s.size += bp->s.size;
		p->s.pointer = bp->s.pointer;
	}
	else
		p->s.pointer = bp;
	
	freelist = p;
}


void *realloc(void * oldpointer, size_t new_size)
{
	/* If pointer is NULL, then the call is equivalent to malloc */
	if (oldpointer == NULL)
	{
		return malloc(new_size);
	}
	/* If size is equal to zero then the call is equivalent to free */
	if (new_size == 0)
	{
		free(oldpointer);
		return NULL;
	}
	int old_size = (((Header*)oldpointer - 1)->s.size-1)*sizeof(Header);

	/* Allocate new space, copy contents and free old space */
	void * newpointer = malloc(new_size);
	
	int copy_size = (old_size < new_size ? old_size : new_size);

	newpointer = memcpy(newpointer, oldpointer, copy_size);
	free(oldpointer);
	return newpointer;
}

