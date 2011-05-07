#include <unistd.h>
#include <string.h>
#include <stdio.h>
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

/* init_freelists: initialize the free lists */
void init_freelists()
{
	int i;
	for(i = 0; i < NRQUICKLISTS; i++)
	{
		freelists[i] = &base[i];
		freelists[i]->s.next = NULL;
		freelists[i]->s.size = 0;
	}

	freelists[NRQUICKLISTS-1]->s.next = &base[NRQUICKLISTS-1];
}

/* allocate_blocks: ask system for more memory */
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
		new_slot = (Header *) (previous_program_break + sizeof(Header)*units*i);
		new_slot->s.size = units;
		free((void*)(new_slot+1));
	}
	return list;
}

/* malloc: general-purpose storage allocator */
void *malloc(size_t nbytes)
{
	Header *current_pointer, *previous_pointer;
	unsigned int nunits;
	int i, max_units;
	Header * match;

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
	
	/* Find a list which fits this block (ignore last list)*/
	for(i = 0; i < NRQUICKLISTS-1; i++, max_units *= 2)
	{
		/* Does not fit yet? -- Next! */
		if (max_units < nunits)
			continue;
		
		/* No blocks here? -- Allocate some! */
		if (freelists[i] == NULL || freelists[i]->s.size == 0)
			if (allocate_blocks(freelists[i], max_units, 1) == NULL)
				return NULL; /* Out of memory :( */
		
		/* Pop first free slot */
		match = freelists[i];
		freelists[i] = freelists[i]->s.next;
	
		return (void*)(match+1);
	}
	
	/* No block has been found yet since we haven't returned. */
	/* Perform First-fit on the last freelist */

	previous_pointer = freelists[NRQUICKLISTS-1];
	current_pointer = previous_pointer->s.next;

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
				previous_pointer->s.next = current_pointer->s.next;
			}
			else 
			{
				/* Remove units from slot */
				current_pointer->s.size -= nunits;
				/* Create a new slot at the end of the big slot */ 
				current_pointer += current_pointer->s.size;
				current_pointer->s.size = nunits;
			}
			return (void *)(current_pointer+1);
		}
		
		/* Have we looked at all elements in the list? */
		if (current_pointer == freelists[NRQUICKLISTS-1])
		{
			/* Ask for more memory! */
			current_pointer = allocate_blocks(freelists[NRQUICKLISTS-1],
					(nunits < NALLOC ? NALLOC : nunits), 1);
			if (current_pointer == NULL)
			{
				/* Memory full */
				return NULL;
			}
		}

		/* Increment pointers */
		previous_pointer = current_pointer;
		current_pointer = current_pointer->s.next;
	}
}

/* free: put block in free list */
void free(void *target)
{
	int i, max_units;
	Header *target_head;
	Header *p;
	
	if (target == NULL)
		return;

	/* Point to block header */
	target_head = (Header *)target - 1; 
	max_units = SMALLEST_LIST;
	for(i = 0; i < NRQUICKLISTS-1; i++, max_units *= 2)
	{
		/* Does not fit yet? -- Next! */
		if (max_units < target_head->s.size)
			continue;
		
		/* It fits, put it in there and return. */
		target_head->s.next = freelists[i];
		freelists[i] = target_head;
		return;
	}
	
	/* It did not fit in our quicklists! */
	/* Perform firstfit-free */
	
	/* TODO: Patch this with worstfit / bestfit */

	/* Go through the free list in an attempt to find a block to merge with */
	for(p = freelists[NRQUICKLISTS-1]; ;p = p->s.next)
	{
		/* Is the freë́'d block in between two free blocks? */
		if (p <= target_head && p->s.next >= target_head)
			break;
		
		/* Are we not counting upwards in memory? */
		/* Eqv. - Have we checked all slots? */
		if (p >= p->s.next)
		{
			/* Free'd block at start of area */
			if (target_head > p)
				break;
		
			/* Free'd block at end of area */
			if (target_head < p->s.next)
				break;
		}
	}
	/* After this loop, p should be set to a block next to the target block in memory */

	/* Join target to p.next */
	if (target_head + target_head->s.size == p->s.next)
	{ 
		target_head->s.size += p->s.next->s.size;
		target_head->s.next = p->s.next->s.next;
	}
	else
	{
		/* Merge up failed, just link them together */
		target_head->s.next = p->s.next;
	}
	
	/* Join target to p */
	if (p + p->s.size == target_head)
	{
		p->s.size += target_head->s.size;
		p->s.next = target_head->s.next;
	}
	else
	{
		/* Merge down failed, just link them together */
		p->s.next = target_head;
	}
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

