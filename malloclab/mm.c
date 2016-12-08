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
#define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~(ALIGNMENT-1))

#define TOP_ALIGNMENT 8
#define TOP_ALIGN(size) (((size) + (TOP_ALIGNMENT-1)) & ~(TOP_ALIGNMENT-1))

#define NUMPREPARED 1

#define SIZE_T_SIZE (ALIGN(sizeof(size_t)))

#define UNSORTED 0

#define NBINS 121
#define NSMALLBINS 62

#define MIN_LARGE_SIZE ((NSMALLBINS + 2) << 3)

#define is_smallbin_range(size) (size < MIN_LARGE_SIZE)
#define smallbin_index(size) ((size >> 3) - 1)

#define VALID 1
#define PREV_INUSE 2

#define is_valid(p) (p->size & VALID)
#define prev_inuse(p) (p->size & PREV_INUSE)
#define flags(p) (p->size & (ALIGNMENT-1))

#define size2reqsize(size) (ALIGN(size - sizeof(size_t)))
#define size2chunksize(size) (ALIGN(size2reqsize(size) + 2 * sizeof(size_t)))
#define mem2chunk(p) ((struct malloc_chunk *) ((char *) p - 2 * sizeof(size_t)))
#define chunk2mem(p) ((void *)((char *) p + 2 * sizeof(size_t)))

#define is_empty_bin(bin) (bin->tail == mem2chunk(bin))

#define chunksize(p) ((p->size) & ~(ALIGNMENT-1))

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
     * 62 small bins with size 8 (size 16 ~ 511)
     * 58 large bins with
     *   - 32 bins with size 64 (size 528 ~ 2559)
     *   - 11 bins with size 512 (size 2560 ~ 8191)
     *   - 6 bins with size 4096 (size 8192 ~ 32767)
     *   - 7 bins with size 32768 (size 32768 ~ 262143)
     *   - 1 bins with size 262144 (size 262144 ~ 524287)
     *   - 1 bins for left (size 524288 ~ )
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

static int bin_index(size_t size);

static int extend_top_chunk(size_t size);
static void update_prev_info(struct malloc_chunk *chunk);

static struct malloc_chunk *find_chunk(size_t size);
static struct malloc_chunk *find_chunk_from_bin(struct malloc_bin *bin, size_t size);

static void append_chunk(struct malloc_bin *bin, struct malloc_chunk *chunk);
static void unlink_chunk(struct malloc_chunk *chunk);

static struct malloc_chunk *split_chunk(struct malloc_chunk *chunk, size_t size);
static struct malloc_chunk *split_top_chunk(size_t size);
static struct malloc_chunk *split_top_chunk_prepared(size_t size);
static void split_and_append(struct malloc_chunk *chunk, size_t size);

static struct malloc_chunk *coalesce_chunk(struct malloc_chunk *chunk);
static struct malloc_chunk *realloc_chunk(struct malloc_chunk *chunk, size_t size);

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

/* Calculate corresponding large bin index */
static int bin_index(size_t size)
{
    if (is_smallbin_range(size))
        return smallbin_index(size);
    else if (size < 2560)
        return 55 + (size >> 6);
    else if (size < 8192)
        return 90 + (size >> 9);
    else if (size < 32768)
        return 104 + (size >> 12);
    else if (size < 262144)
        return 110 + (size >> 15);
    else if (size < 524288)
        return 119;
    else
        return 120;
}

/* Extend top chunk as much as required */
static int extend_top_chunk(size_t size)
{
    struct malloc_chunk *top = arena.top;
    size_t required = TOP_ALIGN(size + 2 * sizeof(struct malloc_chunk) - chunksize(top));

    if ((void *)-1 == mem_sbrk(required)) return -1;
    else {
        top->size += required;
        return 0;
    }
}

/* Update prev size of next chunk (if next chunk is valid virtual memory space) */
static void update_prev_info(struct malloc_chunk *chunk)
{
    struct malloc_chunk *next_chunk = next_chunk(chunk);

    if ((void *)next_chunk < mem_heap_hi()) {
        next_chunk->prev_size = chunksize(chunk);
        next_chunk->size &= ~PREV_INUSE;
    }
}

/* Append chunk to bin */
static void append_chunk(struct malloc_bin *bin, struct malloc_chunk *chunk)
{
    struct malloc_chunk *last = mem2chunk(bin);
    struct malloc_chunk *cur_chunk;
    size_t chunk_size = chunksize(chunk);

    for (cur_chunk = bin->tail; cur_chunk != last; cur_chunk = cur_chunk->bk) {
        if (chunksize(cur_chunk) > chunk_size)
            break;
    }

    chunk->fd = cur_chunk->fd;
    chunk->bk = cur_chunk;

    cur_chunk->fd->bk = chunk;
    cur_chunk->fd = chunk;   
}

/* Find free chunk with size larger than given size */
static struct malloc_chunk *find_chunk(size_t size)
{
    struct malloc_chunk *last;
    struct malloc_chunk *cur_chunk, *next_chunk;
    int bin_idx = bin_index(size);
    
    for (; bin_idx < NBINS; bin_idx++) {
        cur_chunk = find_chunk_from_bin(&arena.bins[bin_idx], size);
        if (cur_chunk)
            return cur_chunk;
    }

    last = mem2chunk(&arena.bins[UNSORTED]);
    cur_chunk = arena.bins[UNSORTED].head;
    while (cur_chunk != last) {
        next_chunk = cur_chunk->fd;

        if (chunksize(cur_chunk) >= size)
            return cur_chunk;
        else {
            unlink_chunk(cur_chunk);
            bin_idx = bin_index(chunksize(cur_chunk));
            append_chunk(&arena.bins[bin_idx], cur_chunk);    
        }

        cur_chunk = next_chunk;
    }

    return NULL;
}

static struct malloc_chunk *find_chunk_from_bin(struct malloc_bin *bin, size_t size)
{
    struct malloc_chunk *cur_chunk, *last;
    size_t chunk_size;

    if (is_empty_bin(bin))
        return NULL;

    last = mem2chunk(bin);
    for (cur_chunk = bin->head; cur_chunk != last; cur_chunk = cur_chunk->fd) {
        chunk_size = chunksize(cur_chunk);
        if (chunk_size == size)
            return cur_chunk;
        else if (chunk_size < size)
            break;
    }

    if (cur_chunk->bk != last)
        return cur_chunk->bk;
    else
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

    chunk->size = size | flags(chunk);

    remainder = (struct malloc_chunk *) ((char *)chunk + size);
    remainder->prev_size = size;
    remainder->size = (chunk_size - size) & ~VALID;
    update_prev_info(remainder);

    return remainder;
}

/* Split top chunk and expand if needed */
static struct malloc_chunk *split_top_chunk(size_t size)
{
    struct malloc_chunk *top = arena.top;

    if (chunksize(top) < size + 2 * sizeof(struct malloc_chunk))
        if (extend_top_chunk(size) < 0)
            return NULL;
    
    arena.top = split_chunk(top, size);
    return top;
}

/* Split top chunk and expand if needed
 * and prepare some additional free chunk */
static struct malloc_chunk *split_top_chunk_prepared(size_t size)
{
    size_t prepared_size = 280;
    struct malloc_chunk *chunk, *top = arena.top;

    if (chunksize(top) < size + prepared_size + 2 * sizeof(struct malloc_chunk))
        if (extend_top_chunk(size + prepared_size) < 0)
            return NULL;

    chunk = split_chunk(top, prepared_size);
    chunk->size &= ~PREV_INUSE;
    append_chunk(&arena.bins[UNSORTED], top);

    arena.top = split_chunk(chunk, size);

    return chunk;
}

/* Split and append remainder to corresponding bin (if remainder is not NULL) */
static void split_and_append(struct malloc_chunk *chunk, size_t size)
{
    struct malloc_chunk *remainder;
    int bin_idx;

    remainder = split_chunk(chunk, size);
    if (remainder) {
        remainder->size |= PREV_INUSE;
        bin_idx = bin_index(chunksize(chunk));
        append_chunk(&arena.bins[bin_idx], remainder);
    }
}


/* Coalesce chunks with nearby chunks */
static struct malloc_chunk *coalesce_chunk(struct malloc_chunk *chunk)
{
    struct malloc_chunk *prev_chunk = prev_chunk(chunk);
    struct malloc_chunk *next_chunk = next_chunk(chunk);
    struct malloc_chunk *merged_chunk = chunk;

    if (!is_valid(next_chunk)) {
        if (next_chunk != arena.top)
            unlink_chunk(next_chunk);
        
        chunk->size += chunksize(next_chunk);
    }

    if (!prev_inuse(chunk)) {
        unlink_chunk(prev_chunk);

        prev_chunk->size += chunksize(chunk);
        merged_chunk = prev_chunk;
    }

    if (next_chunk == arena.top)
        arena.top = merged_chunk;

    return merged_chunk;
}

static struct malloc_chunk *realloc_chunk(struct malloc_chunk *chunk, size_t size)
{
    void *oldptr, *newptr;
    struct malloc_chunk *next_chunk = next_chunk(chunk);
    size_t required_size = size2chunksize(size) - chunksize(chunk);

    /* If the next chunk is top chunk */
    if (next_chunk == arena.top) {
        if (!split_top_chunk(required_size))
            return NULL;
    
        chunk->size += chunksize(next_chunk);

        next_chunk = next_chunk(chunk);
        next_chunk->prev_size = chunksize(chunk);
        next_chunk->size |= PREV_INUSE;

        return chunk2mem(chunk);
    }
    /* If there is space on next of chunk */
    else if (!is_valid(next_chunk) && chunksize(next_chunk) >= required_size) {
        split_and_append(next_chunk, required_size);

        chunk->size += chunksize(next_chunk);

        next_chunk = next_chunk(chunk);
        next_chunk->prev_size = chunksize(chunk);
        next_chunk->size |= PREV_INUSE;

        return chunk2mem(chunk);
    }
    else { /* If there is no space */
        oldptr = chunk2mem(chunk);
        newptr = mm_malloc(size);
        if (newptr == NULL)
            return NULL;
        
        memcpy(newptr, oldptr, size);
        
        mm_free(oldptr);
        
        return newptr;
    }
}

/* 
 * mm_init - initialize the malloc package.
 */
int mm_init(void)
{
    bins_init(arena.bins, NBINS);

    arena.top = mem_sbrk(TOP_ALIGNMENT);
    if (arena.top == (struct malloc_chunk *)-1) return -1;
    else
        arena.top->size = TOP_ALIGNMENT | PREV_INUSE;

    return 0;
}

/* 
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *     Always allocate a block whose size is a multiple of the alignment.
 */
void *mm_malloc(size_t size)
{
    size_t chunk_size = size2chunksize(size);
    struct malloc_chunk *p;

    /* Try to find from unsorted bin */
    p = find_chunk(chunk_size);
    if (p) {
        unlink_chunk(p);
        split_and_append(p, chunk_size);
    } else {
        p = split_top_chunk_prepared(chunk_size);
        if (!p) return NULL;
    }

    p->size |= VALID;
    next_chunk(p)->size |= PREV_INUSE;

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
    update_prev_info(merged_chunk);

    if (merged_chunk != arena.top)
        append_chunk(&arena.bins[UNSORTED], merged_chunk);
}

/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
 */
void *mm_realloc(void *ptr, size_t size)
{
    struct malloc_chunk *chunk = mem2chunk(ptr);
    size_t chunk_size = size2chunksize(size);

    if (!ptr)
        return mm_malloc(size);
    else if (!size) {
        mm_free(ptr);
        return NULL;
    } else if (chunksize(chunk) >= chunk_size) {
        split_and_append(chunk, chunk_size);
        return chunk2mem(chunk);
    } else
        return realloc_chunk(chunk, size);
}














