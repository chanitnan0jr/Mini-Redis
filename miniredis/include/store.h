#ifndef STORE_H
#define STORE_H

#include <stddef.h> // size_t

// Node Structure (Linked List)
typedef struct HNode {
    struct HNode *next;
    uint64_t hcode; // Hash code stored for resizing
    char *key;
    char *value;
} HNode;

// Table Structure (Dictionary)
typedef struct HMap {
    HNode **tab;   // Array of pointers (Main table)
    size_t mask;   // Table size - 1 (Used for index masking)
    size_t size;   // Total slots (Must be Power of 2)
    size_t used;   // Number of actual data items
} HMap;

// API
void hmap_init(HMap *hmap);
void hmap_destroy(HMap *hmap);
HNode *hmap_lookup(HMap *hmap, const char *key);
void hmap_insert(HMap *hmap, const char *key, const char *value);
int hmap_delete(HMap *hmap, const char *key);

// Global Store API
void store_init(void);
HMap *store_get_db(void);

#endif
