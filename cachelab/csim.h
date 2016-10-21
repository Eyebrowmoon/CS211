#ifndef _CSIM_H
#define _CSIM_H

#include <inttypes.h>

#define HIT 0
#define MISS

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

  int hit_count;
  int miss_count;
  int eviction_count;

  struct cache_set **cache;
};

struct cache_set *new_cset(int E);
struct cache_sim *new_csim(void);
void alloc_cache(struct cache_sim *csim);
void free_cset(struct cache_set *cset);
void free_csim(struct cache_sim *csim);

void add_line(struct cache_set *cset);
void unlink_line(struct cache_line *cline);
void prepend_line(struct cache_set *cset, struct cache_line *cline);
struct cache_line *find_line(struct cache_set *cset, uint64_t tag);
struct cache_line *find_invalid(struct cache_set *cset);

void print_help(void);

struct cache_sim *parse_args(int argc, char *argv[]);
int is_valid_input(struct cache_sim *csim);

void access_cache(struct cache_sim *csim, uint64_t addr, int size); 
void simulate(struct cache_sim *csim);

#endif
