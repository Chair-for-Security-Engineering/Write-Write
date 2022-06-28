//#define USE_LIBTEA
#define BENCH
#define TRY_UNTIL_SUCCESS
#define VERIFY
#if defined USE_LIBTEA || defined VERIFY
#include "libtea.h"

libtea_instance* instance;
void setup_libtea();
#else
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <time.h>
#endif // USE_LIBTEA
#include "math.h"


#define RUNS 10
#define CACHE_MISS_THRESHOLD 130 // 200 for XEON E-2224G
#define OUTLIER_THRESHOLD 1400 // 1400 for XEON E-2224G
#define MEM_SIZE 100000000
#define CACHE_ASSOC 16

struct eviction_set_t{
  uint64_t *address;
  struct eviction_set_t *next;
}ev_set_t;


struct eviction_set_t* get_evset(uint64_t* addr_space, uint64_t* victim, uint64_t addr_space_size);

void calibrate(uint64_t* target);

// Functions to minimize and test the eviction set

bool reduce_evset(uint64_t* victim, struct eviction_set_t** ev_set_ptr);

int get_evset_len(struct eviction_set_t* ev_set);

void append_evset_address(struct eviction_set_t* ev_set, struct eviction_set_t* e);

void merge_evsets(struct eviction_set_t** a, struct eviction_set_t** b);

bool test_evset(uint64_t *victim, struct eviction_set_t *ev_set);

// Output functions
void print_evset(struct eviction_set_t* ev_set, uint64_t* victim);