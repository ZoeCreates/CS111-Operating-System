#include "hash-table-base.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <sys/queue.h>
#include <pthread.h>

// Structure representing an entry in the hash table
struct list_entry {
    const char *key;
    uint32_t value;
    SLIST_ENTRY(list_entry) pointers;  // Singly linked list pointers
};

SLIST_HEAD(list_head, list_entry);

// Structure representing a single hash table entry (a bucket)
struct hash_table_entry {
    struct list_head list_head;        // Linked list of entries for this hash bucket
    pthread_mutex_t lock;              // Mutex for protecting the list in this bucket
};

// Structure representing the entire hash table
struct hash_table_v2 {
    struct hash_table_entry entries[HASH_TABLE_CAPACITY];
};

// Function to create and initialize a new hash table
struct hash_table_v2 *hash_table_v2_create()
{
    struct hash_table_v2 *hash_table = calloc(1, sizeof(struct hash_table_v2));
    assert(hash_table != NULL);

    // Initialize each hash bucket and its mutex
    for (size_t i = 0; i < HASH_TABLE_CAPACITY; ++i) {
        struct hash_table_entry *entry = &hash_table->entries[i];
        SLIST_INIT(&entry->list_head);
        if (pthread_mutex_init(&entry->lock, NULL) != 0) {
            exit(EXIT_FAILURE);  // Exit if mutex initialization fails
        }
    }
    return hash_table;
}

// Function to retrieve the appropriate hash table entry (bucket) for a given key
static struct hash_table_entry *get_hash_table_entry(struct hash_table_v2 *hash_table,
                                                     const char *key)
{
    assert(key != NULL);
    uint32_t index = bernstein_hash(key) % HASH_TABLE_CAPACITY;
    struct hash_table_entry *entry = &hash_table->entries[index];
    return entry;
}

// Function to retrieve a specific list entry in a given hash table entry (bucket)
static struct list_entry *get_list_entry(struct hash_table_v2 *hash_table,
                                         const char *key,
                                         struct list_head *list_head)
{
    assert(key != NULL);

    struct list_entry *entry = NULL;

    // Iterate through the list to find the matching key
    SLIST_FOREACH(entry, list_head, pointers) {
        if (strcmp(entry->key, key) == 0) {
            return entry;
        }
    }
    return NULL;
}

// Function to check if the hash table contains a specific key
bool hash_table_v2_contains(struct hash_table_v2 *hash_table,
                            const char *key)
{
    struct hash_table_entry *hash_table_entry = get_hash_table_entry(hash_table, key);

    // Lock the bucket's mutex before accessing it
    if (pthread_mutex_lock(&hash_table_entry->lock) != 0) {
        exit(EXIT_FAILURE);
    }

    struct list_head *list_head = &hash_table_entry->list_head;
    struct list_entry *list_entry = get_list_entry(hash_table, key, list_head);

    // Unlock the mutex after access
    if (pthread_mutex_unlock(&hash_table_entry->lock) != 0) {
        exit(EXIT_FAILURE);
    }

    return list_entry != NULL;
}

// Function to add or update an entry in the hash table
void hash_table_v2_add_entry(struct hash_table_v2 *hash_table,
                             const char *key,
                             uint32_t value)
{
    struct hash_table_entry *hash_table_entry = get_hash_table_entry(hash_table, key);

    // Lock the bucket's mutex before modifying it
    if (pthread_mutex_lock(&hash_table_entry->lock) != 0) {
        exit(EXIT_FAILURE);
    }

    struct list_head *list_head = &hash_table_entry->list_head;
    struct list_entry *list_entry = get_list_entry(hash_table, key, list_head);

    // Update the value if the key already exists
    if (list_entry != NULL) {
        list_entry->value = value;

        // Unlock the mutex after the update
        if (pthread_mutex_unlock(&hash_table_entry->lock) != 0) {
            exit(EXIT_FAILURE);
        }
        return;
    }

    // If the key doesn't exist, create a new list entry
    list_entry = calloc(1, sizeof(struct list_entry));
    list_entry->key = key;
    list_entry->value = value;
    SLIST_INSERT_HEAD(list_head, list_entry, pointers);

    // Unlock the mutex after the insertion
    if (pthread_mutex_unlock(&hash_table_entry->lock) != 0) {
        exit(EXIT_FAILURE);
    }
}

// Function to retrieve the value associated with a specific key in the hash table
uint32_t hash_table_v2_get_value(struct hash_table_v2 *hash_table,
                                 const char *key)
{
    struct hash_table_entry *hash_table_entry = get_hash_table_entry(hash_table, key);

    // Lock the bucket's mutex before accessing it
    if (pthread_mutex_lock(&hash_table_entry->lock) != 0) {
        exit(EXIT_FAILURE);
    }

    struct list_head *list_head = &hash_table_entry->list_head;
    struct list_entry *list_entry = get_list_entry(hash_table, key, list_head);

    // Unlock the mutex after access
    if (pthread_mutex_unlock(&hash_table_entry->lock) != 0) {
        exit(EXIT_FAILURE);
    }

    assert(list_entry != NULL);
    return list_entry->value;
}

// Function to destroy the hash table and free all associated memory
void hash_table_v2_destroy(struct hash_table_v2 *hash_table)
{
    for (size_t i = 0; i < HASH_TABLE_CAPACITY; ++i) {
        struct hash_table_entry *entry = &hash_table->entries[i];
        struct list_head *list_head = &entry->list_head;
        struct list_entry *list_entry = NULL;
        while (!SLIST_EMPTY(list_head)) {
            list_entry = SLIST_FIRST(list_head);
            SLIST_REMOVE_HEAD(list_head, pointers);
            free(list_entry);
        }

        // Destroy the mutex for each bucket
        if (pthread_mutex_destroy(&entry->lock) != 0) {
            exit(EXIT_FAILURE);
        }
    }

    // Free the hash table structure itself
    free(hash_table);
}

