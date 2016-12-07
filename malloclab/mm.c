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

#define TOP_ALIGNMENT 4096
#define TOP_ALIGN(size) (((size) + (TOP_ALIGNMENT-1)) & ~4095)

#define SIZE_T_SIZE (ALIGN(sizeof(size_t)))

#define UNSORTED 0

#define NBINS 128
#define NSMALLBINS 64

#define VALID 1
#define is_valid(p) (p->size & VALID)

#define size2chunksize(size) (ALIGN(size + 2 * sizeof(size_t)))
#define mem2chunk(p) ((struct malloc_chunk *) ((char *) p - 2 * sizeof(size_t)))
#define chunk2mem(p) ((void *)((char *) p + 2 * sizeof(size_t)))

#define is_empty_bin(bin) (bin->tail == mem2chunk(bin))

#define chunksize(p) ((p->size) & ~0x7)

#define next_chunk(p) ((struct malloc_chunk *) ((char *) p + chunksize(p)))
#define prev_chunk(p) ((struct malloc_chunk *) ((char *) p - p->prev_size))

/*******************
 * TYPES AND STRUCTS
 *******************/

struct malloc_chunk {
    size_t prev_size;   /* Alwats used */
    size_t size;

    struct malloc_chunk *fd;    /* Used only if free */
    struct malloc_chunk *bk;
};

struct malloc_bin {
    struct malloc_chunk *head;
    struct malloc_chunk *tail;
};

struct malloc_state {

    struct malloc_chunk *top;

    /* 
     * Components of bins are similar to libc
     * 
     * There is no fast bin
     * Single unsorted bin
     * 64 small bins with size 8
     * 63 large bins with
     *   - 32 bins with size 64
     *   - 16 bins with size 512
     *   - 8 bins with size 4096
     *   - 4 bins with size 32768
     *   - 2 bins with size 262144
     *   - 1 bins for left
     * */
    struct malloc_bin bins[NBINS];
};

/******************
 * GLOBAL VARIABLES
 ******************/

static struct malloc_state arena;

/************
 * PROTOTYPES
 ************/

static void malloc_bin_init(struct malloc_bin *bin);
static void bins_init(struct malloc_bin *bins, int num);

static int extend_top_chunk(size_t size);

static void update_prev_size(struct malloc_chunk *chunk);

static void append_chunk(struct malloc_bin *bin, struct malloc_chunk *chunk);
static struct malloc_chunk *find_chunk(struct malloc_bin *bin, size_t size);
static void unlink_chunk(struct malloc_chunk *chunk);
static struct malloc_chunk *split_chunk(struct malloc_chunk *chunk, size_t size);
static struct malloc_chunk *coalesce_chunk(struct malloc_chunk *chunk);

/*****************
 * IMPLEMENTATIONS
 *****************/

/* Consider current bin as a (fake) malloc_chunk */
static void malloc_bin_init(struct malloc_bin *bin)
{
    bin->head = mem2chunk(bin);
    bin->tail = bin->head;
}

/* Initialize array of bins with given length num */
static void bins_init(struct malloc_bin *bins, int num)
{
    int i;
    
    for (i = 0; i < num; i++)
        malloc_bin_init(&bins[i]);    
}

/* Extend top chunk as much as required */
static int extend_top_chunk(size_t size)
{
    struct malloc_chunk *top = arena.top;
    size_t required = TOP_ALIGN(size + 2 * sizeof(struct malloc_chunk) - chunksize(top));

    if ((void *)-1 == mem_sbrk(required))
        return -1;
    else {
        top->size += required;
        return 0;
    }
}

/* Update prev size of next chunk (if next chunk is valid virtual memory space) */
static void update_prev_size(struct malloc_chunk *chunk)
{
    next_chunk(chunk)->prev_size = chunksize(chunk);
}

/* Append chunk to bin */
static void append_chunk(struct malloc_bin *bin, struct malloc_chunk *chunk)
{
    chunk->fd = bin->head;
    chunk->bk = mem2chunk(bin);

    bin->head->bk = chunk;
    bin->head = chunk;   
}

/* Find free chunk with size larger than given size */
static struct malloc_chunk *find_chunk(struct malloc_bin *bin, size_t size)
{
    struct malloc_chunk *last = mem2chunk(bin);
    struct malloc_chunk *cur_chunk;
   
    for (cur_chunk = bin->head; cur_chunk != last; cur_chunk = cur_chunk->fd) {
        if (chunksize(cur_chunk) >= size)
            return cur_chunk;
    }

    return NULL;
}

/* 
 * Unlink given malloc chunk from bin.
 * Does not check any consistency.
 */
static void unlink_chunk(struct malloc_chunk *chunk)
{
    struct malloc_chunk *fd = chunk->fd;
    struct malloc_chunk *bk = chunk->bk;

    fd->bk = bk;
    bk->fd = fd;
}

/* 
 * Split given chunk into two chunks (chunks of given size and remainder).
 * Return the remainder chunk.
 * If there is no remainder chunk, it returns NULL.
 * */
static struct malloc_chunk *split_chunk(struct malloc_chunk *chunk, size_t size)
{
    struct malloc_chunk *remainder;
    size_t chunk_size = chunksize(chunk);

    if (chunk_size <= size + sizeof (struct malloc_chunk))
        return NULL;

    remainder = (struct malloc_chunk *) ((char *)chunk + size);
    remainder->prev_size = size;
    remainder->size = chunk_size - size;
    remainder->size &= ~VALID;

    update_prev_size(remainder);

    chunk->size = size;

    return remainder;
}

/* Coalesce chunks with nearby chunks */
static struct malloc_chunk *coalesce_chunk(struct malloc_chunk *chunk)
{
    struct malloc_chunk *prev_chunk = prev_chunk(chunk);
    struct malloc_chunk *next_chunk = next_chunk(chunk);
    struct malloc_chunk *merged_chunk = chunk;

    if (next_chunk != arena.top && !is_valid(next_chunk)) {
        unlink_chunk(next_chunk);
        
        chunk->size += chunksize(next_chunk);
    }

    if (mem_heap_lo() < (void *)prev_chunk && !is_valid(prev_chunk)) {
        unlink_chunk(prev_chunk);

        prev_chunk->size += chunksize(chunk);
        merged_chunk = prev_chunk;
    }

    return merged_chunk;
}

/* 
 * mm_init - initialize the malloc package.
 */
int mm_init(void)
{
    bins_init(arena.bins, NBINS);

    arena.top = mem_sbrk(TOP_ALIGNMENT);
    if (arena.top == (struct malloc_chunk *)-1)
        return -1;
    else
        arena.top->size = TOP_ALIGNMENT;

    return 0;
}

/* 
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *     Always allocate a block whose size is a multiple of the alignment.
 */
void *mm_malloc(size_t size)
{
    size_t chunksize = size2chunksize(size);
    struct malloc_chunk *p, *remainder;
    struct malloc_chunk *top = arena.top;

    /* Try to find from unsorted bin */
    p = find_chunk(&arena.bins[UNSORTED], chunksize);
    if (p) {
        unlink_chunk(p);
        remainder = split_chunk(p, chunksize);
        if (remainder) {
            append_chunk(&arena.bins[UNSORTED], remainder);
        }

        p->size |= VALID;
        update_prev_size(p);

        return chunk2mem(p);
    }

    if (top->size < chunksize + 2 * sizeof(struct malloc_chunk))
        if (extend_top_chunk(chunksize) < 0)
            return NULL;

    p = top;
    arena.top = split_chunk(p, chunksize);

    p->size |= VALID;

    return chunk2mem(p);
}

/*
 * mm_free - Freeing a block does nothing.
 */
void mm_free(void *ptr)
{
    struct malloc_chunk *chunk = mem2chunk(ptr);
    struct malloc_chunk *merged_chunk = coalesce_chunk(chunk);

    merged_chunk->size &= ~VALID;
    update_prev_size(merged_chunk);

    append_chunk(&arena.bins[UNSORTED], merged_chunk);
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














