#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>
#include "../include/store.h"

// Initial size
#define K_INITIAL_SIZE 4 

// --- Helper Functions ---

// FNV-1a Hash Function (Standard, more popular than djb2 for production)
static uint64_t str_hash(const char *data) {
    uint64_t h = 0xcbf29ce484222325;
    const char *p = data;
    while (*p) {
        h = (h ^ (unsigned char)*p) * 0x100000001b3;
        p++;
    }
    return h;
}

// Table expansion function (The Core Logic)
static void hmap_resize(HMap *hmap) {
    // 1. Calculate new size (double it)
    size_t new_size = hmap->size ? hmap->size * 2 : K_INITIAL_SIZE;
    
    // 2. Allocate new table (Pointer Array)
    HNode **new_tab = calloc(new_size, sizeof(HNode *));
    if (!new_tab) return; // Out of memory handling (should be handled better in production)

    size_t new_mask = new_size - 1;

    // 3. Move old data to new home (Rehashing)
    // Here we don't malloc/free Nodes, we just "move Pointers"
    for (size_t i = 0; i < hmap->size; ++i) {
        HNode *node = hmap->tab[i];
        while (node) {
            HNode *next = node->next; // Remember next node
            
            // Calculate new position: (hcode & new_mask)
            // Tech tip: Using & mask is much faster than % size (but size must be 2^N)
            size_t pos = node->hcode & new_mask;
            
            // Insert at head of new table
            node->next = new_tab[pos];
            new_tab[pos] = node;
            
            node = next; // Move to next node
        }
    }

    // 4. Discard old table and switch to new one
    free(hmap->tab);
    hmap->tab = new_tab;
    hmap->size = new_size;
    hmap->mask = new_mask;
}

// --- API Implementation ---

void hmap_init(HMap *hmap) {
    hmap->tab = NULL;
    hmap->mask = 0;
    hmap->size = 0;
    hmap->used = 0;
}

// Lookup (GET)
HNode *hmap_lookup(HMap *hmap, const char *key) {
    if (!hmap->tab) return NULL;

    uint64_t h = str_hash(key);
    size_t pos = h & hmap->mask;
    
    HNode *node = hmap->tab[pos];
    while (node) {
        if (node->hcode == h && strcmp(node->key, key) == 0) {
            return node;
        }
        node = node->next;
    }
    return NULL;
}

// Insert (SET)
void hmap_insert(HMap *hmap, const char *key, const char *value) {
    // Check Load Factor: If full, expand
    if (!hmap->tab || hmap->used >= hmap->size) {
        hmap_resize(hmap);
    }

    // 1. Try to find if Key exists (Update)
    HNode *node = hmap_lookup(hmap, key);
    if (node) {
        free(node->value);
        node->value = strdup(value);
        return;
    }

    // 2. If not found, create new Node (Insert)
    uint64_t h = str_hash(key);
    node = malloc(sizeof(HNode));
    node->next = NULL;
    node->hcode = h;
    node->key = strdup(key);
    node->value = strdup(value);

    // Insert into table
    size_t pos = h & hmap->mask;
    node->next = hmap->tab[pos];
    hmap->tab[pos] = node;
    hmap->used++;
}

// Delete (DEL) - return 1 if deleted, 0 if not found
int hmap_delete(HMap *hmap, const char *key) {
    if (!hmap->tab) return 0;

    uint64_t h = str_hash(key);
    size_t pos = h & hmap->mask;

    HNode **from = &hmap->tab[pos]; // Pointer to Pointer (Secret Technique)
    
    // Loop to find Node
    while (*from) {
        HNode *node = *from;
        if (node->hcode == h && strcmp(node->key, key) == 0) {
            // Found! Cut from Linked List
            *from = node->next; // Point previous to next
            
            free(node->key);
            free(node->value);
            free(node);
            hmap->used--;
            return 1;
        }
        from = &node->next;
    }
    return 0;
}

void hmap_destroy(HMap *hmap) {
    for (size_t i = 0; i < hmap->size; ++i) {
        HNode *node = hmap->tab[i];
        while (node) {
            HNode *next = node->next;
            free(node->key);
            free(node->value);
            free(node);
            node = next;
        }
    }
    free(hmap->tab);
    hmap_init(hmap); // Reset struct
}

// Global Store
static HMap g_db;

void store_init(void) {
    hmap_init(&g_db);
}

HMap *store_get_db(void) {
    return &g_db;
}