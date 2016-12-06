/*
 * mm-naive.c - The fastest, least memory-efficient malloc package.
 * 
 * In this naive approach, a block is allocated by simply incrementing
 * the brk pointer.  A block is pure payload. There are no headers or
 * footers.  Blocks are never coalesced or reused. Realloc is
 * implemented directly using mm_malloc and mm_free.
 *
 * NOTE TO STUDENTS: Replace this header comment with your own header
 * comment that gives a high level description of your solution.
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>

#include "mm.h"
#include "memlib.h"

/*********************************************************
 * NOTE TO STUDENTS: Before you do anything else, please
 * provide your student information in the following struct.
 ********************************************************/
student_t student = {
    /* POVIS ID */
    "khpark0312",
    /* Your full name */
    "Kang-Hee Park",
    /* Your email address */
    "khpark0312@postech.ac.kr",
};


/*********************
 * CONSTANT AND MACROS
 *********************/

/* single word (4) or double word (8) alignment */
#define ALIGNMENT 8
/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~0x7)

#define SIZE_T_SIZE (ALIGN(sizeof(size_t)))

#define NBINS 128
#define NSMALLBINS 64

#define PREV_INUSE 1
#define prev_inuse(p) (p->size & PREV_INUSE)

/*******************
 * TYPES AND STRUCTS
 *******************/

struct malloc_chunk {
    size_t prev_size;   /* Used only if free */
    size_t size;        /* Always used */

    struct malloc_chunk *fd;    /* Used only if free */
    struct malloc_chunk *bk;
};

struct malloc_bin {
    struct malloc_chunk *head;
    struct malloc_chunk *tail;
};

struct malloc_state {
    struct malloc_chunk *top;

    struct malloc_bin bins[NBINS];
};

/******************
 * GLOBAL VARIABLES
 ******************/

static struct malloc_state main_arena;

/************
 * PROTOTYPES
 ************/

static void malloc_bin_init(struct malloc_bin *bin);
static void bins_init(struct malloc_bin *bins, int num);

/*****************
 * IMPLEMENTATIONS
 *****************/

/* Consider current bin as a (fake) malloc_chunk */
static void malloc_bin_init(struct malloc_bin *bin)
{
    bin->head = (struct malloc_chunk *) (&bin->head - 2 * sizeof(size_t));
    bin->tail = bin->head;
}

static void bins_init(struct malloc_bin *bins, int num)
{
    int i;
    
    for (i = 0; i < num; i++)
        malloc_bin_init(&bins[i]);    
}

/* 
 * mm_init - initialize the malloc package.
 */
int mm_init(void)
{
    bins_init(main_arena.bins, NBINS);

    return 0;
}

/* 
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *     Always allocate a block whose size is a multiple of the alignment.
 */
void *mm_malloc(size_t size)
{
    int newsize = ALIGN(size + SIZE_T_SIZE);
    void *p = mem_sbrk(newsize);
    if (p == (void *)-1)
	return NULL;
    else {
        *(size_t *)p = size;
        return (void *)((char *)p + SIZE_T_SIZE);
    }
}

/*
 * mm_free - Freeing a block does nothing.
 */
void mm_free(void *ptr)
{
}

/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
 */
void *mm_realloc(void *ptr, size_t size)
{
    void *oldptr = ptr;
    void *newptr;
    size_t copySize;
    
    newptr = mm_malloc(size);
    if (newptr == NULL)
      return NULL;
    copySize = *(size_t *)((char *)oldptr - SIZE_T_SIZE);
    if (size < copySize)
      copySize = size;
    memcpy(newptr, oldptr, copySize);
    mm_free(oldptr);
    return newptr;
}














