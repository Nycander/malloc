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

void putBlock(Header* block)
{

    Header *current_p, *prev_p, *next_p;

    prev_p = freelist;
    current_p = prev_p->s.pointer;
    next_p = current_p->s.pointer;

    for(;;)
    {
        if(current_p->s.size <= block->s.size)
        {
            prev_p->s.pointer = block;
            block->s.pointer = current_p;
            if(freelist == current_p)
                freelist = block;
            break;
        }
        if(current_p == freelist)
        {
            prev_p->s.pointer = block;
            block->s.pointer = current_p;
            break;
        }
        /* Update pointers */
        prev_p = current_p;
        current_p = next_p;
        next_p = next_p->s.pointer;
    }
}

/* malloc: general-purpose storage allocator */
void *malloc(size_t nbytes)
{
    Header *current_p, *prev_p;
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
        base.s.pointer = freelist = &base;
        base.s.size = 0;
    }

    prev_p = freelist;
    current_p = prev_p->s.pointer;

    /* Go through all elements in the free list */
    for (;;)
    {
        /* Is this slot big enough? */
        if (current_p->s.size >= nunits) 
        {
            /* Is it exactly big enough? */
            if (current_p->s.size == nunits) 
            {
                if(current_p == freelist)
                {
                    freelist = current_p->s.pointer;
                }
                /* Remove the slot */
                prev_p->s.pointer = current_p->s.pointer;
            }
            else 
            {
                if(current_p == freelist)
                    freelist = current_p->s.pointer;
                Header* tmp = current_p;
                /* Remove units from slot */
                current_p->s.size -= nunits;
                /* Create a new slot at the end of the big slot */ 
                current_p += current_p->s.size;
                current_p->s.size = nunits;
                prev_p->s.pointer = tmp->s.pointer;
                putBlock(tmp);
            }
            return (void *)(current_p+1);
        }

        /* Have we looked at all elements in the list? */
        if (current_p == freelist)
        {
            /* Ask for more memory! */
            current_p = morecore(nunits);
            if (current_p == NULL)
            {
                /* Memory full */
                return NULL;
            }

        }

        /* Increment pointers */
        prev_p = current_p;
        current_p = current_p->s.pointer;
    }
}

/* free: put block target in free list */
void free(void *target)
{
    if (target == NULL)
        return;

    Header *target_head, *current_p, *prev_p, *next_p;

    prev_p = freelist;
    current_p = prev_p->s.pointer;
    next_p = current_p->s.pointer;

    target_head = (Header *)target - 1; /* point to block header */
    Header *block = target_head;
    /* look for merge opportunities, merge and take the merged block
     * out of linked list.*/
    do{
        if(current_p + current_p->s.size == target_head) /* freed block at end of empty space?*/
        {
            if(current_p == freelist)
            {
                freelist = next_p;
                prev_p->s.pointer = freelist;
            }
            else
            {
                prev_p->s.pointer = next_p;
            }
            /*fixa cirkel länkning för sista elementet.*/

            current_p->s.size += block->s.size;
            block = current_p;
        }
        if(target_head + target_head->s.size == current_p) /* freed block at start of empty space?*/
        {
            if(current_p == freelist)
            {
                freelist = next_p;
                prev_p->s.pointer = freelist;
            }
            else
            {
                prev_p->s.pointer = next_p;
            }

            block->s.size += current_p->s.size;
        }

        /* Update pointers */
        prev_p = current_p;
        current_p = next_p;
        next_p = next_p->s.pointer;
    }
    while (current_p != freelist);

    for(;;)
    {
        if(current_p->s.size <= block->s.size)
        {
            prev_p->s.pointer = block;
            block->s.pointer = current_p;
            if(freelist == current_p)
                freelist = block;
            break;
        }
        if(current_p == freelist)
        {
            prev_p->s.pointer = block;
            block->s.pointer = current_p;
            break;
        }
        /* Update pointers */
        prev_p = current_p;
        current_p = next_p;
        next_p = next_p->s.pointer;
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

    /*newpointer =*/ memcpy(newpointer, oldpointer, copy_size);
    free(oldpointer);
    return newpointer;
}

