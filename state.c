/* state.c - Global state definitions for the crawler */
#include "crawler.h"

/* Global queue for URLs to be crawled */
queue_t q;

/* Global hash table for URL deduplication */
node_t *hash_table[MAX_HASH];

/* Domain name of first URL */
char root[400];

/* Struct used for stat function */
struct stat stats;

/* File paths */
char waiting_list[100], root_list[100], found_list[100];

/* Signal handling flag */
volatile sig_atomic_t should_checkpoint = 0;