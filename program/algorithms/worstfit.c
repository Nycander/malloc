#include <unistd.h>

#include "../malloc.h"
#include "../brk.h"

#define NALLOC 1024 /* minimum #units to request */

typedef long int Align; /* for alignment to long boundary */
union header
{
    /* block header */
    struct
    {
        union header *pointer; /* next block if on free list */
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
    char *cp;
    Header *new_slot;
    if (number_of_units < NALLOC)
        number_of_units = NALLOC;

    cp = sbrk(number_of_units * sizeof(Header));
    if (cp == (char *) -1) /* no space at all */
        return NULL;

    new_slot = (Header *) cp;
    new_slot->s.size = number_of_units;
    free((void *)(new_slot+1));
    return freelist;
}

/* malloc: general-purpose storage allocator */
/* WARNING BAD CODE STEAL THE OLD ONE! */
void *malloc(size_t nbytes)
{
    Header *current_pointer, *previous_pointer;
    unsigned int nunits;

    /* Don't allocate blocks with zero bytes */
    if (nbytes == 0)
    {
        return NULL;
    }

    /* Use number_of_unitsmber magic to get good roundoff */
    nunits = (nbytes+sizeof(Header)-1)/sizeof(Header) + 1;

    previous_pointer = freelist;

    /* First run, initialize the free list */
    if (previous_pointer == NULL)
    {
        base.s.pointer = freelist = previous_pointer = &base;
        base.s.size = 0;
    }

    current_pointer = previous_pointer->s.pointer;

    for (;;)
    {
        /* Is this slot big enough? */
        if (current_pointer->s.size == max)
        {
            /* Is it exactly big enough? */
            if (current_pointer->s.size == nunits)
            {
                /* Remove the slot */
                previous_pointer->s.pointer = current_pointer->s.pointer;
            }
            else
            {
                /* Remove units from slot */
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
            current_pointer = morecore(nunits);
            if (current_pointer == NULL)
            {
                /* Memory full */
                return NULL;
            }

        }

        /* Increment pointers */
        previous_pointer = current_pointer;
        current_pointer = current_pointer->s.pointer;
    }
}

/* free: put block target in free list */
void free(void *target)
{
    if (target == NULL)
        return;

    Header *target_head, *current_p, prev_p, next_p;
    target_head = (Header *)target - 1; /* point to block header */
    Header *block = target_head;
    int first = 1;
    int merges = 0;
    current_p = freelist;
    prev_p = freelist;
    /* look for merge opportunities, merge and take the merged block
     * out of linked list.*/
    while (current_p != freelist || first == 1)
    {
        if(target_head.s.pointer == current_p.s.pointer+current_p.s.size) /* freed block at end of empty space?*/
        {
            if(first==1) freelist = next_p;
            prev_p.s.pointer-> = next_p;
            block = prev_p;
            block
            merges = 1;
        }
        if(target_head+target_head.s.size == current_p) /* freed block at start of empty space?*/
        {
            if(first==1) freelist = next_p;
            prev_p.s.pointer-> = next_p;
            block = prev_p;
            if(merges == 1)
            {

                break;
            }
        }
        break; /* freed block at start or end of arena */

        /* Update pointers */
        first = 0;
        prev_p = current_p->s.pointer;
        current_p = current_p->s.pointer;
        next_p = current_p->s.pointer;
    }

    if (target_head + target_head->s.size == current_p->s.pointer)
    {
        /* join to new_slotper nbr */
        target_head->s.size += current_p->s.pointer->s.size;
        target_head->s.pointer = current_p->s.pointer->s.pointer;
    }
    else
        target_head->s.pointer = current_p->s.pointer;

    if (current_p + current_p->s.size == target_head)
    {
        /* join to lower nbr */
        current_p->s.size += target_head->s.size;
        p->s.pointer = target_head->s.pointer;
    }
    else
        p->s.pointer = target_head;

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

