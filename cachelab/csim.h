#ifndef _CSIM_H
#define _CSIM_H

#include <stdint.h>

struct cache_line {
  int valid;
  uint64_t tag; 

  struct cache_line *prev;
  struct cache_line *next;
};

struct cache_set {
  int E;
  struct cache_line *head;
  struct cache_line *tail;  
};

struct cache_sim {
  int verbose;
  int s;
  int E;
  int b;
  char *trace_name;

  struct cache_set **cache;
};

struct cache_set *new_cset(int E);
struct cache_sim *new_csim(void);
void alloc_cache(struct cache_sim *csim);
void free_cset(struct cache_set *cset);
void free_csim(struct cache_sim *csim);

void add_line(struct cache_set *cset);
//struct cache_line *exist_line(uint64_t tag);

struct cache_sim *parse_args(int argc, char *argv[]);
int is_valid_input(struct cache_sim *csim);

void print_help(void);

#endif
