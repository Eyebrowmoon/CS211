#include <stdio.h>

extern int mm_init (void);
extern void *mm_malloc (size_t size);
extern void mm_free (void *ptr);
extern void *mm_realloc(void *ptr, size_t size);


/* 
 * Students work in teams of one or two.  Teams enter their team name, 
 * personal names and login IDs in a struct of this
 * type in their bits.c file.
 */
typedef struct {
    char *id; /* ID1+ID2 or ID1 */
    char *name;    /* full name of first member */
    char *email;      /* login ID of first member */
} student_t;

extern student_t student;
