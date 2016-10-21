#include "cachelab.h"
#include "csim.h"
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>

int main(int argc, char *argv[])
{
  struct cache_sim *csim;

  csim = parse_args(argc, argv);

  if (is_valid_input(csim))
    alloc_cache(csim);
  else {
    print_help();
    exit(0);
  }

  printSummary(0, 0, 0);
  return 0;
}

/* Return new cache set with E lines.
 * It does not check validity of E, so E must be larger than 0. */
struct cache_set *new_cset(int E)
{
  int i;
  struct cache_set *cset = malloc(sizeof(struct cache_set)); 
  struct cache_line *head = malloc(sizeof(struct cache_line));
  struct cache_line *tail = malloc(sizeof(struct cache_line));

  head->prev = NULL;
  head->next = tail;

  tail->prev = head;
  tail->next = NULL;

  cset->E = E;
  cset->head = head;
  cset->tail = tail;

  for (i = 0; i < E; i++)
    add_line(cset);

  return cset; 
}

/* Return new cache simulator.
 * calloc used instead of malloc,
 * since 0 is the default value for all members.*/
struct cache_sim *new_csim(void)
{
  struct cache_sim *csim = calloc(sizeof(struct cache_sim), 1); 

  return csim;
}

void alloc_cache(struct cache_sim *csim)
{
  int i;
  int S = 1 << csim->s;
  int E = csim->E;

  struct cache_set **cache = malloc(sizeof(struct cache_set *) * S);

  for (i = 0; i < S; i++)
    cache[i] = new_cset(E);

  csim->cache = cache;
}

void free_cset(struct cache_set *cset)
{
  struct cache_line *cur_line = cset->head;
  struct cache_line *next_line;

  while (cur_line) {
    next_line = cur_line->next;
    free(cur_line);
    cur_line = next_line;
  } 
}

void free_csim(struct cache_sim *csim)
{
  int i;
  int S = 1 << csim->s;
  char *trace_name = csim->trace_name;
  struct cache_set **cache = csim->cache;

  if (trace_name)
    free(trace_name);
    
  if (cache) {
    for (i = 0; i < S; i++)
      free_cset(cache[i]);
    
    free(cache);
  }

  free(csim);
}

void add_line(struct cache_set *cset)
{
  struct cache_line *cline = malloc(sizeof(struct cache_line));
  struct cache_line *head = cset->head;
  struct cache_line *head_next = head->next;

  cline->valid = 0;
  
  cline->prev = head;
  cline->next = head_next;

  head->next = cline;
  head_next->prev = cline;
}

/* Parse arguments and store to csim, and return it. */
struct cache_sim *parse_args(int argc, char *argv[])
{
  struct cache_sim *csim = new_csim();

  int opt = 0;
  char *trace_name;

  while (-1 != (opt = getopt(argc, argv, "hvsEbt:"))) {
    switch(opt) {
      case 'h':
        print_help();
        exit(0);
      case 'v':
        csim->verbose = 1;
        break;
      case 's':
        csim->s = atoi(optarg);
        break;
      case 'E':
        csim->E = atoi(optarg);
        break;
      case 'b':
        csim->b = atoi(optarg);
        break;
      case 't':
        trace_name = malloc(strlen(optarg) + 1); 
        strcpy(trace_name, optarg);
               
        csim->trace_name = trace_name;
    }
  }

  return csim;
}

int is_valid_input(struct cache_sim *csim)
{  
  return (csim->s > 0 && csim->E > 0 && csim->b > 0 && csim->trace_name);
}

void print_help(void)
{
  printf("Usage: ./csim-ref [-hv] -s <num> -E <num> -b <num> -t <file>\n");
  printf("Options:\n");
  printf("  -h         Print this help message.\n");
  printf("  -v         Optional verbose flag.\n");
  printf("  -s <num>   Number of set index bits.\n");
  printf("  -E <num>   Number of lines per set.\n");
  printf("  -b <num>   Number of block offset bits.\n");
  printf("  -t <file>  Trace file.\n\n");
      
  printf("Examples:\n");
  printf("  linux>  ./csim-ref -s 4 -E 1 -b 4 -t traces/yi.trace\n");
  printf("  linux>  ./csim-ref -v -s 8 -E 2 -b 4 -t traces/yi.trace\n");
}

