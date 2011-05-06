#include <unistd.h>
#include <string.h>
#include "../malloc.h"
#include "../brk.h"

#define NALLOC 1024 /* minimum #units to request */

#define NRQUICKLISTS 5
#define SMALLEST_LIST 1

typedef long int Align; /* for alignment to long boundary */
union header
{
	/* block header */
	struct
	{
		union header *next; /* next block if on free list */
		unsigned int size; /* size of this block */
	} s;
	Align x; /* force alignment of blocks */
};

typedef union header Header;

static Header base[NRQUICKLISTS]; /* empty list to get started */
static Header * freelists[NRQUICKLISTS];

void init_freelists()
{
	int i;
	for(i = 0; i < NRQUICKLISTS; i++)
	{
		base[i].s.next = freelists[i] = &base[i];
		base[i].s.size = 0;
	}
}

/* morecore: ask system for more memory */
static Header* morecore(unsigned int number_of_units)
{
	char * previous_program_break;
	Header *new_slot;

	if (number_of_units < NALLOC)
	{
		number_of_units = NALLOC;
	}
	
	previous_program_break = sbrk(number_of_units * sizeof(Header));
	if (previous_program_break == (char *) -1) /* no space at all */
		return NULL;
	
	new_slot = (Header *) previous_program_break;
	new_slot->s.size = number_of_units;
	free((void *)(new_slot+1));
	return NULL;
}

static Header* allocate_blocks(Header * list, unsigned int units, unsigned int blocks)
{
	char * previous_program_break;
	int i;
	unsigned int number_of_units = units*blocks;	
	Header *new_slot;
	
	previous_program_break = sbrk(number_of_units * sizeof(Header));
	if (previous_program_break == (char *) -1) /* no space at all */
		return NULL;
	
	for(i = 0; i < blocks; i++)
	{
		new_slot = (Header *) previous_program_break + sizeof(Header)*units*i;
		new_slot->s.size = blocks;
		free((void*)(new_slot+1));
	}
	return list;
}

/* malloc: general-purpose storage allocator */
void *malloc(size_t nbytes)
{
	unsigned int nunits;
	int i, max_units;

	/* Don't allocate blocks with zero bytes */
	if (nbytes == 0)
	{
		return NULL;
	}
	
	/* Use number magic to get good roundoff */
	nunits = (nbytes+sizeof(Header)-1)/sizeof(Header) + 1;
	
	/* First run, initialize the free list */	
	if (freelists[0] == NULL) 
	{
		init_freelists();
	}
	
	max_units = SMALLEST_LIST;

	/* Find a list which fits this block */
	for(i = 0; i < NRQUICKLISTS; i++, max_units *= 2)
	{
		/* Does not fit yet? -- Next! */
		if (max_units < nunits)
			continue;
		
		/* No blocks here? -- Allocate some! */
		if (freelists[i]->s.size == 0)
			if (allocate_blocks(freelists[i], max_units, 3) == NULL)
				return NULL; /* Out of memory :( */
		
		/* TODO */
	}

}

/* free: put block in free list */
void free(void *target)
{
	if (target == NULL)
		return;
	/* TODO */
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

