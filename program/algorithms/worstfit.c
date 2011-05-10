#include <unistd.h>
#include <stdio.h>

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

    Header *prev_p;
    Header *current_p;
    Header *next_p;

    prev_p    = freelist;
    current_p = prev_p->s.next;
    next_p    = current_p->s.next;

    for(;;)
    {
        if(current_p->s.size >= block->s.size) /* The only difference between best and worst?*/
        {
            prev_p->s.next = block;
            block->s.next = current_p;
            if(freelist == current_p)
                freelist = block;
            break;
        }
        if(current_p == freelist)
        {
            prev_p->s.next = block;
            block->s.next = current_p;
            break;
        }
        /* Update pointers */
        prev_p = current_p;
        current_p = next_p;
        next_p = next_p->s.next;
    }
}

/* malloc: general-purpose storage allocator */
void *malloc(size_t nbytes)
{
    /* Don't allocate blocks with zero bytes */
    if (nbytes == 0)
    {
        return NULL;
    }

    Header *current_p;
    Header *prev_p;



    unsigned int nunits;
    /* Use number magic to get good roundoff */
    nunits = (nbytes+sizeof(Header)-1)/sizeof(Header) + 1;


    /* First run, initialize the free list */	
    if (freelist== NULL) 
    {
        base.s.next = freelist = &base;
        base.s.size = 0;
    }

    prev_p = freelist;
    current_p = prev_p->s.next;

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
                    freelist = current_p->s.next;
                }
                /* Remove the slot */
                prev_p->s.next = current_p->s.next;
            }
            else
            {
                if(current_p == freelist)
                    freelist = current_p->s.next;
                Header* tmp = current_p;
                /* Remove units from slot */
                current_p->s.size -= nunits;
                /* Create a new slot at the end of the big slot */ 
                current_p += current_p->s.size;
                current_p->s.size = nunits;

                prev_p->s.next = tmp->s.next;
                fprintf(stderr, "putting block...");
                putBlock(tmp);
                fprintf(stderr, "DONE.\n");
            }
            return (void *)(current_p+1);
        }

        /* Have we looked at all elements in the list? */
        fprintf(stderr, "\\  /  ");
        if (current_p == freelist)
        {
        fprintf(stderr, "Wrapedidoo");
        if (current_p == freelist)
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
        current_p = current_p->s.next;
    }
}

/* free: put block target in free list */
void free(void *target)
{
    if (target == NULL)
        return;


    Header *current_p;
    Header *prev_p;
    Header *next_p;

    prev_p    = freelist;
    current_p = prev_p->s.next;
    next_p    = current_p->s.next;

    Header *block_p = (Header *)target - 1; /* point to block header */

    /* look for merge opportunities, merge and take the merged block
     * out of linked list.*/
    do
    {
        if(current_p + current_p->s.size == block_p) /* freed block at end of empty space?*/
        {
            if(current_p == freelist)
            {
                freelist = next_p;
                prev_p->s.next = freelist;
            }
            else
            {
                prev_p->s.next = next_p;
            }

            current_p->s.size += block_p->s.size;
            block_p = current_p;
        }
        if(block_p + block_p->s.size == current_p) /* freed block at start of empty space?*/
        {
            if(current_p == freelist)
            {
                freelist = next_p;
                prev_p->s.next = freelist;
            }
            else
            {
                prev_p->s.next = next_p;
            }

            block_p->s.size += current_p->s.size;
        }

        /* Update pointers */
        prev_p    = current_p;
        current_p = next_p;
        next_p    = next_p->s.next;
    }
    while (current_p != freelist);

    for(;;)
    {
        if(current_p->s.size <= block_p->s.size)
        {
            prev_p->s.next = block_p;
            block_p->s.next = current_p;
            if(freelist == current_p)
                freelist = block_p;
            break;
        }
        if(current_p == freelist)
        {
            prev_p->s.next = block_p;
            block_p->s.next = current_p;
            break;
        }
        /* Update pointers */
        prev_p    = current_p;
        current_p = next_p;
        next_p    = next_p->s.next;
    }
}


void *realloc(void * oldpointer, size_t new_size)
{
    /* If next is NULL, then the call is equivalent to malloc */
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

