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
		union header *ptr; /* next block if on free list */
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
	Header *p, *prevp;
	unsigned int nunits = (nbytes+sizeof(Header)-1)/sizeof(Header) + 1;
	if ((prevp = freelist) == NULL) 
	{
		/* no free list yet */
		base.s.ptr = freelist = prevp = &base;
		base.s.size = 0;
	}
	for (p = prevp->s.ptr; ; prevp = p, p = p->s.ptr)
	{
		if (p->s.size >= nunits) 
		{ 
			/* big enough */
			if (p->s.size == nunits) 
			{
				/* exactly */
				prevp->s.ptr = p->s.ptr;
			}
			else 
			{
				/* allocate tail end */
				p->s.size -= nunits;
				p += p->s.size;
				p->s.size = nunits;
			}
			freelist = prevp;
			return (void *)(p+1);
		}
		if (p == freelist) /* wrapped around free list */
			if ((p = morecore(nunits)) == NULL)
				return NULL; /* none left */
	}
}

/* free: put block ap in free list */
void free(void *ap)
{
	Header *bp, *p;
	bp = (Header *)ap - 1; /* point to block header */
	for (p = freelist; !(bp > p && bp < p->s.ptr); p = p->s.ptr)
		if (p >= p->s.ptr && (bp > p || bp < p->s.ptr))
			break; /* freed block at start or end of arena */
	
	if (bp + bp->s.size == p->s.ptr)
	{ 
		/* join to upper nbr */
		bp->s.size += p->s.ptr->s.size;
		bp->s.ptr = p->s.ptr->s.ptr;
	}
	else
		bp->s.ptr = p->s.ptr;
	
	if (p + p->s.size == bp)
	{
		/* join to lower nbr */
		p->s.size += bp->s.size;
		p->s.ptr = bp->s.ptr;
	}
	else
		p->s.ptr = bp;
	
	freelist = p;
}


void *realloc(void * oldpointer, size_t new_size)
{
	/* If ptr is NULL, then the call is equivalent to malloc */
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
