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

#define TOP_ALIGNMENT 256
#define TOP_ALIGN(size) (((size) + (TOP_ALIGNMENT-1)) & ~(TOP_ALIGNMENT-1))

#define PREPARED_SIZE 280

#define SIZE_T_SIZE (ALIGN(sizeof(size_t)))

#define UNSORTED 0
#define SMALL 1
#define MIDDLE 2
#define LARGE 3
#define HUGE 4

#define MIN_MIDDLE_SIZE 256
#define MIN_LARGE_SIZE 1024
#define MIN_HUGE_SIZE 8192

#define VALID 1
#define PREV_INUSE 2

#define is_valid(p) (p->size & VALID)
#define prev_inuse(p) (p->size & PREV_INUSE)
#define flags(p) (p->size & (ALIGNMENT-1))

#define size2reqsize(size) (ALIGN(size - sizeof(size_t)))
#define size2chunksize(size) (ALIGN(size2reqsize(size) + 2 * sizeof(size_t)))
#define mem2chunk(p) ((struct malloc_chunk *) ((char *) p - 2 * sizeof(size_t)))
#define chunk2mem(p) ((void *)((char *) p + 2 * sizeof(size_t)))

#define chunksize(p) ((p->size) & ~(ALIGNMENT-1))

#define next_chunk(p) ((struct malloc_chunk *) ((char *) p + chunksize(p)))
#define prev_chunk(p) ((struct malloc_chunk *) ((char *) p - p->prev_size))

/*******************
 * TYPES AND STRUCTS
 *******************/

struct malloc_chunk {
    size_t prev_size;   /* Used only if previous block is free */
    size_t size;        /* Always used */

    struct malloc_chunk *fd;    /* Used only if free */
    struct malloc_chunk *bk;
};

/******************
 * GLOBAL VARIABLES
 ******************/

static struct malloc_chunk *top;

static struct malloc_chunk *unsorted_bin;
static struct malloc_chunk *small_bin;
static struct malloc_chunk *middle_bin;
static struct malloc_chunk *large_bin;
static struct malloc_chunk *huge_bin;

/************
 * PROTOTYPES
 ************/

static int calculate_bin(size_t size);
static struct malloc_chunk **get_bin(int bin);

static int extend_top_chunk(size_t size);
static void update_prev_info(struct malloc_chunk *chunk);

static struct malloc_chunk *find_chunk(size_t size);
static struct malloc_chunk *find_chunk_from_bin(int bin, size_t size);
static struct malloc_chunk *find_chunk_from_unsorted(size_t size);

static void append_chunk(int bin, struct malloc_chunk *chunk);
static void append_chunk_unsorted(struct malloc_chunk *chunk);
static void unlink_chunk(struct malloc_chunk *chunk);

static struct malloc_chunk *split_chunk(struct malloc_chunk *chunk, size_t size);
static struct malloc_chunk *split_top_chunk(size_t size);
static struct malloc_chunk *split_top_chunk_prepared(size_t size);
static void split_and_append(struct malloc_chunk *chunk, size_t size);

static struct malloc_chunk *coalesce_chunk(struct malloc_chunk *chunk);
static struct malloc_chunk *realloc_chunk(struct malloc_chunk *chunk, size_t size);

int mm_check(void);
int check_bin(int bin);

/*****************
 * IMPLEMENTATIONS
 *****************/

/* Calculate corresponding large bin index */
static int calculate_bin(size_t size)
{
    if (size < MIN_MIDDLE_SIZE)
        return SMALL;
    else if (size < MIN_LARGE_SIZE)
        return MIDDLE;
    else if (size < MIN_HUGE_SIZE)
        return LARGE;
    else
        return HUGE;
}

/* Calculate corresponding large bin index */
static struct malloc_chunk **get_bin(int bin)
{
    switch(bin) {
        case UNSORTED:
            return &unsorted_bin;
        case SMALL:
            return &small_bin;
        case MIDDLE:
            return &middle_bin;
        case LARGE:
            return &large_bin;
        case HUGE:
            return &huge_bin;
        default:
            return NULL;
    }
}

/* Extend top chunk as much as required */
static int extend_top_chunk(size_t size)
{
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
static void append_chunk(int bin, struct malloc_chunk *chunk)
{
    struct malloc_chunk **bin_ptr = get_bin(bin);
    struct malloc_chunk *cur_chunk, *next_chunk;
    size_t chunk_size = chunksize(chunk);

    /* Find place to inserted */
    cur_chunk = mem2chunk(bin_ptr);
    next_chunk = cur_chunk->fd;
    while (next_chunk) {
        if (chunksize(next_chunk) >= chunk_size)
            break;
        cur_chunk = next_chunk;
        next_chunk = cur_chunk->fd;
    }

    /* Insert chunk */
    chunk->fd = next_chunk;
    chunk->bk = cur_chunk;

    cur_chunk->fd = chunk;
    if (next_chunk)
        next_chunk->bk = chunk;
}

/* append to unsorted bin */
static void append_chunk_unsorted(struct malloc_chunk *chunk)
{
    chunk->fd = unsorted_bin;
    chunk->bk = mem2chunk(&unsorted_bin);

    if (unsorted_bin) unsorted_bin->bk = chunk;
    unsorted_bin = chunk;   
}

/* 
 * Unlink given malloc chunk from bin.
 * Does not check any consistency.
 */
static void unlink_chunk(struct malloc_chunk *chunk)
{
    struct malloc_chunk *fd = chunk->fd;
    struct malloc_chunk *bk = chunk->bk;

    bk->fd = fd;
    if (fd) fd->bk = bk;
}

/* Find free chunk with size larger than given size */
static struct malloc_chunk *find_chunk(size_t size)
{
    struct malloc_chunk *chunk;
    int bin = calculate_bin(size);

    switch (bin) {
        case SMALL:
            chunk = find_chunk_from_bin(SMALL, size);
            if (chunk) return chunk;
        case MIDDLE:
            chunk = find_chunk_from_bin(MIDDLE, size);
            if (chunk) return chunk;
        case LARGE:
            chunk = find_chunk_from_bin(LARGE, size);
            if (chunk) return chunk;
        case HUGE:
            chunk = find_chunk_from_bin(HUGE, size);
            if (chunk) return chunk;
        default:
            return find_chunk_from_unsorted(size);
    }
}

/* Find chunk from sorted bin */
static struct malloc_chunk *find_chunk_from_bin(int bin, size_t size)
{
    struct malloc_chunk **bin_ptr = get_bin(bin);
    struct malloc_chunk *cur_chunk;

    if (!bin_ptr)
        return NULL;

    for (cur_chunk = *bin_ptr; cur_chunk; cur_chunk = cur_chunk->fd) {
        if (chunksize(cur_chunk) >= size)
            return cur_chunk;
    }

    return NULL;
}

/* Find chunk from unsorted bin */
static struct malloc_chunk *find_chunk_from_unsorted(size_t size)
{
    struct malloc_chunk *cur_chunk, *next_chunk;
    int bin;

    cur_chunk = unsorted_bin;
    while (cur_chunk) {
        next_chunk = cur_chunk->fd;
        
        if (chunksize(cur_chunk) >= size)
            return cur_chunk;
        else {
            unlink_chunk(cur_chunk);
            bin = calculate_bin(chunksize(cur_chunk));
            append_chunk(bin, cur_chunk);
        }

        cur_chunk = next_chunk;
    }

    return NULL;
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
    struct malloc_chunk *top_chunk = top;

    if (chunksize(top_chunk) < size + 2 * sizeof(struct malloc_chunk))
        if (extend_top_chunk(size) < 0)
            return NULL;
    
    top = split_chunk(top_chunk, size);
    return top_chunk;
}

/* Split top chunk and expand if needed
 * and prepare some additional free chunk */
static struct malloc_chunk *split_top_chunk_prepared(size_t size)
{
    struct malloc_chunk *chunk, *top_chunk = top;

    if (chunksize(top_chunk) < size + PREPARED_SIZE + 2 * sizeof(struct malloc_chunk))
        if (extend_top_chunk(size + PREPARED_SIZE) < 0)
            return NULL;

    chunk = split_chunk(top_chunk, PREPARED_SIZE);
    chunk->size &= ~PREV_INUSE;
    append_chunk_unsorted(top_chunk);

    top = split_chunk(chunk, size);

    return chunk;
}

/* Split and append remainder to corresponding bin (if remainder is not NULL) */
static void split_and_append(struct malloc_chunk *chunk, size_t size)
{
    struct malloc_chunk *remainder;
    int bin;

    remainder = split_chunk(chunk, size);
    if (remainder) {
        remainder->size |= PREV_INUSE;
        bin = calculate_bin(chunksize(chunk));
        append_chunk(bin, remainder);
    }
}


/* Coalesce chunks with nearby chunks */
static struct malloc_chunk *coalesce_chunk(struct malloc_chunk *chunk)
{
    struct malloc_chunk *prev_chunk = prev_chunk(chunk);
    struct malloc_chunk *next_chunk = next_chunk(chunk);
    struct malloc_chunk *merged_chunk = chunk;

    /* Merge with next chunk */
    if (!is_valid(next_chunk)) {
        if (next_chunk != top)
            unlink_chunk(next_chunk);
        
        chunk->size += chunksize(next_chunk);
    }

    /* Merge with prev chunk */
    if (!prev_inuse(chunk)) {
        unlink_chunk(prev_chunk);

        prev_chunk->size += chunksize(chunk);
        merged_chunk = prev_chunk;
    }

    if (next_chunk == top)
        top = merged_chunk;

    return merged_chunk;
}

static struct malloc_chunk *realloc_chunk(struct malloc_chunk *chunk, size_t size)
{
    void *oldptr, *newptr;
    struct malloc_chunk *next_chunk = next_chunk(chunk);
    size_t required_size = size2chunksize(size) - chunksize(chunk);

    /* If the next chunk is top chunk */
    if (next_chunk == top) {
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
    unsorted_bin = NULL;
    small_bin = NULL;
    middle_bin = NULL;
    large_bin = NULL;
    huge_bin = NULL;

    top = mem_sbrk(TOP_ALIGNMENT);
    if (top == (struct malloc_chunk *)-1) return -1;
    else {
        top->prev_size = 0;
        top->size = TOP_ALIGNMENT | PREV_INUSE;
    }

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
    struct malloc_chunk *merged_chunk;
    
    merged_chunk = coalesce_chunk(chunk);

    merged_chunk->size &= ~VALID;
    update_prev_info(merged_chunk);

    if (merged_chunk != top)
        append_chunk_unsorted(merged_chunk);
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

/* Check whether single chunk is invalid */
int check_chunk(struct malloc_chunk *chunk)
{
    /* Chunk is not in heap area */
    if ((void *) next_chunk(chunk) > mem_heap_hi() || (void *) chunk < mem_heap_lo()) {
        printf("Not in heap area: %p\n", chunk);
        return -1;
    }

    /* Freed chunk is marked as valid */
    if (is_valid(chunk)) {
        printf("Valid free chunk: %p\n", chunk);
        return -1;
    }

    /* Not merged with adjacent free chunk */
    if (!prev_inuse(chunk) || !is_valid(next_chunk(chunk))) {
        printf("Not merged free chunk: %p\n", chunk);
        return -1;
    }

    /* Wrong prev_size */
    if (next_chunk(chunk)->prev_size != chunksize(chunk)) {
        printf("Wrong prev_size in next chunk: %p\n", chunk);
        return -1;    
    }

    return 0;    
}

/* Chunk whether given bin includes invalid chunk */
int check_bin(int bin)
{
    struct malloc_chunk **bin_ptr = get_bin(bin);
    struct malloc_chunk *chunk;
    
    for (chunk = *bin_ptr; chunk; chunk = chunk->fd) {
        if (check_chunk(chunk) < 0)
            return -1;    
    }

    return 0;
}

/*
 * Check consistency of heap memory.
 * 
 * Check can be enabled by adding following code to start of mm_malloc, mm_free, mm_realloc:
 *  - if (mm_check() < 0) exit(0);
 * 
 * This checks
 *  - whether freed chunk is in heap area
 *  - whether freed chunk is marked as invalid
 *  - whether freed chunk is merged with adjacent free chunks
 *  - whether prev_size of next chunk is equal to size of given chunk
 */
int mm_check(void)
{
    if (check_bin(UNSORTED) < 0) return -1;
    if (check_bin(SMALL) < 0) return -1;
    if (check_bin(MIDDLE) < 0) return -1;
    if (check_bin(LARGE) < 0) return -1;
    if (check_bin(HUGE) < 0) return -1;
    
    return 0;  
}

