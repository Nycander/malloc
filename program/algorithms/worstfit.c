#include <unistd.h>
#include <string.h>
#include "../malloc.h"
#include "../brk.h"

#define NALLOC 1024 /* minimum #units to request */

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

static Header base; /* empty list to get started */
static Header *freelist = NULL;


/* morecore: ask system for more memory */
static Header *morecore(unsigned int number_of_units)
{
    char * previous_program_break;
    Header *new_slot;
    if (number_of_units < NALLOC)
        number_of_units = NALLOC;

    previous_program_break = sbrk(number_of_units * sizeof(Header));
    if (previous_program_break == (char *) -1) /* no space at all */
        return NULL;

    new_slot = (Header *) previous_program_break;
    new_slot->s.size = number_of_units;
    free((void *)(new_slot+1));
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


    /* First run, initialize the free list */	
    if (freelist== NULL) 
    {
        base.s.next = freelist = &base;
        base.s.size = 0;
    }

    previous_pointer = freelist;
    current_pointer = previous_pointer->s.next;

    Header *before_largest = freelist;
    Header *largest = previous_pointer->s.next;

    /* Go through all elements in the free list */
    for (;;previous_pointer = current_pointer,
            current_pointer  = current_pointer->s.next)
    {
        /* Is this slot big enough? */
        if (current_pointer->s.size > largest->s.size)
        {
            largest = current_pointer;
            before_largest = previous_pointer;
        }
        /* Is it exactly big enough? */
        if (current_pointer == freelist) 
        { /* back to the beginning */
            if(largest->s.size < nunits)
            {
                if((current_pointer = morecore(nunits)) == NULL)
                { /* no memory left*/
                    return NULL;
                }
            }else{
                break;
            }
        }
    }
    if(largest->s.size == nunits)
    {/* Fits the block perfectly */
        before_largest->s.next = largest->s.next;
        if(largest == freelist)
        {
            freelist = largest->s.next;
        }
    }
    else
    {/*allocate tail and head*/
        largest->s.size -= nunits;
        largest += largest->s.size;
        largest->s.size = nunits;
    }
    return (void *) (largest+1); /* point tó the data of current.*/
}
/* free: put block in free list */
void free(void *target)
{
    if (target == NULL)
        return;

    Header *p;
    Header *target_head = (Header *)target - 1; /* Point to block header */

    /* Go through the free list in an attempt to find a block to merge with */
    for(p = freelist; ;p = p->s.next)
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

