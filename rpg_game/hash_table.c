/*
 Student Name:  Karn Watcharasupat
 Date:          11 March 2020

=======================
ECE 2035 Project 2-1:
=======================
This file provides definition for the structs and functions declared in the
header file. It also contains helper functions that are not accessible from
outside of the file.

FOR FULL CREDIT, BE SURE TO TRY MULTIPLE TEST CASES and DOCUMENT YOUR CODE.

===================================
Naming conventions in this file:
===================================
1. All struct names use camel case where the first letter is capitalized.
  e.g. "HashTable", or "HashTableEntry"

2. Variable names with a preceding underscore "_" will not be called directly.
  e.g. "_HashTable", "_HashTableEntry"

  Recall that in C, we have to type "struct" together with the name of the struct
  in order to initialize a new variable. To avoid this, in hash_table.h
  we use typedef to provide new "nicknames" for "struct _HashTable" and
  "struct _HashTableEntry". As a result, we can create new struct variables
  by just using:
    - "HashTable myNewTable;"
     or
    - "HashTableEntry myNewHashTableEntry;"

  The preceding underscore "_" simply provides a distinction between the names
  of the actual struct defition and the "nicknames" that we use to initialize
  new structs.
  [See Hidden Definitions section for more information.]

3. Functions, their local variables and arguments are named with camel case, where
  the first letter is lower-case.
  e.g. "createHashTable" is a function. One of its arguments is "numBuckets".
       It also has a local variable called "newTable".

4. The name of a struct member is divided by using underscores "_". This serves
  as a distinction between function local variables and struct members.
  e.g. "num_buckets" is a member of "HashTable".

*/

/****************************************************************************
* Include the Public Interface
*
* By including the public interface at the top of the file, the compiler can
* enforce that the function declarations in the the header are not in
* conflict with the definitions in the file. This is not a guarantee of
* correctness, but it is better than nothing!
***************************************************************************/
#include "hash_table.h"

/****************************************************************************
* Include other private dependencies
*
* These other modules are used in the implementation of the hash table module,
* but are not required by users of the hash table.
***************************************************************************/
#include <stdio.h>   // For printf
#include <stdlib.h>  // For malloc and free

/****************************************************************************
* Hidden Definitions
*
* These definitions are not available outside of this file. However, because
* the are forward declared in hash_table.h, the type names are
* available everywhere and user code can hold pointers to these structs.
***************************************************************************/
/**
 * This structure represents an a hash table.
 * Use "HashTable" instead when you are creating a new variable. [See top comments]
 */
struct _HashTable {
    /** The array of pointers to the head of a singly linked list, whose nodes
      are HashTableEntry objects */
    HashTableEntry** buckets;

    /** The hash function pointer */
    HashFunction hash;

    /** The number of buckets in the hash table */
    unsigned int num_buckets;
};

/**
 * This structure represents a hash table entry.
 * Use "HashTableEntry" instead when you are creating a new variable. [See top comments]
 */
struct _HashTableEntry {
    /** The key for the hash table entry */
    unsigned int key;

    /** The value associated with this hash table entry */
    void* value;

    /**
  * A pointer pointing to the next hash table entry
  * NULL means there is no next entry (i.e. this is the tail)
  */
    HashTableEntry* next;
};

/****************************************************************************
* Private Functions
*
* These functions are not available outside of this file, since they are not
* declared in hash_table.h.
***************************************************************************/
/**
* createHashTableEntry
*
* Helper function that creates a hash table entry by allocating memory for it on
* the heap. It initializes the entry with key and value, initialize pointer to
* the next entry as NULL, and return the pointer to this hash table entry.
*
* @param key The key corresponds to the hash table entry
* @param value The value stored in the hash table entry
* @return The pointer to the hash table entry
*/
static HashTableEntry* createHashTableEntry(unsigned int key, void* value) {
    // Allocate memory for the HashTableEntry
    HashTableEntry* entry = (HashTableEntry*)malloc(sizeof(HashTableEntry));

    // Initialize struct members
    entry->key = key;
    entry->value = value;
    entry->next = NULL;

    // return the pointer to the HashTableEntry
    return entry;
}

/**
* findItemInBucket
*
* Helper function that checks whether there exists the hash table entry that
* contains a specific key inside a given bucket.
*
* @param bucketEntries The pointer to the head of the bucket.
* @param key The key corresponds to the hash table entry
* @return The pointer to the hash table entry, or NULL if key does not exist
*/
static HashTableEntry* findItemInBucket(HashTableEntry* bucketEntries, unsigned int key) {
    // if the head has the key

    if (!bucketEntries) {
        return NULL;
    }

    if (bucketEntries->key == key) {
        // return the pointer to the entry itself
        return bucketEntries;
    }

    HashTableEntry* thisNode = bucketEntries->next;
    while (thisNode) {
        if (thisNode->key == key) {
            return thisNode;
        }

        thisNode = thisNode->next;
    }

    return NULL;
}

/**
* findItem
*
* Helper function that checks whether there exists the hash table entry that
* contains a specific key.
*
* @param hashTable The pointer to the hash table.
* @param key The key corresponds to the hash table entry
* @return The pointer to the hash table entry, or NULL if key does not exist
*/
static HashTableEntry* findItem(HashTable* hashTable, unsigned int key) {
    unsigned int bucket = hashTable->hash(key);

    HashTableEntry* bucketEntries = hashTable->buckets[bucket];
    HashTableEntry* entry = findItemInBucket(bucketEntries, key);

    return entry;
}

/****************************************************************************
* Public Interface Functions
*
* These functions implement the public interface as specified in the header
* file, and make use of the private functions and hidden definitions in the
* above sections.
****************************************************************************/
// The createHashTable is provided for you as a starting point.
HashTable* createHashTable(HashFunction hashFunction, unsigned int numBuckets) {
    // The hash table has to contain at least one bucket. Exit gracefully if
    // this condition is not met.
    if (numBuckets == 0) {
        printf("Hash table has to contain at least 1 bucket...\n");
        exit(1);
    }

    // Allocate memory for the new HashTable struct on heap.
    HashTable* newTable = (HashTable*)malloc(sizeof(HashTable));

    // Initialize the components of the new HashTable struct.
    newTable->hash = hashFunction;
    newTable->num_buckets = numBuckets;
    newTable->buckets = (HashTableEntry**)malloc(numBuckets * sizeof(HashTableEntry*));

    // As the new buckets contain indeterminant values, init each bucket as NULL.
    unsigned int i;
    for (i = 0; i < numBuckets; ++i) {
        newTable->buckets[i] = NULL;
    }

    // Return the new HashTable struct.
    return newTable;
}

/**
* destroyBucket
*
* Helper function that free all memory allocated for a bucket.
*
* @param buckets The pointer to head of a singly linked list
*/
void destroyBucket(HashTableEntry* bucket) {
    if (!bucket) {
        // if the list is empty, return.
        return;
    }

    HashTableEntry* thisNode = bucket;
    HashTableEntry* nextNode;
    void* value;

    while (thisNode) {  // while this node is not NULL
        nextNode = thisNode->next;
        value = thisNode->value;
        if (value) {
            free(thisNode->value);
        }
        free(thisNode);  // free the memory for this node
        thisNode = nextNode;
    }

    return;
}

/**
 * destroyHashTable
 *
 * Destroy the hash table. The nodes (HashTableEntry objects) of singly linked
 * list, the values stored on the linked list, the buckets, and the hashtable
 * itself are freed from the heap. In other words, free all the allocated memory
 * on heap that is associated with heap, including the values that users store in
 * the hash table.
 *
 * @param myHashTable The pointer to the hash table.
 *
 */
void destroyHashTable(HashTable* hashTable) {
    unsigned int numBuckets = hashTable->num_buckets;

    for (unsigned int i = 0; i < numBuckets; i++) {
        if (hashTable->buckets[i] == NULL) {
            continue;  // there's no memory to free here.
        }

        destroyBucket(hashTable->buckets[i]);
    }
    free(hashTable->buckets);
    free(hashTable);
}

/**
 * insertItem
 *
 * Insert the value into the hash table based on the key.
 * In other words, create a new hash table entry and add it to a specific bucket.
 *
 * @param hashTable The pointer to the hash table.
 * @param key The key that corresponds to the value.
 * @param value The value to be stored in the hash table.
 * @return old value if it is overwritten, or NULL if not replaced
 */
void* insertItem(HashTable* hashTable, unsigned int key, void* value) {
    HashTableEntry* entry = findItem(hashTable, key);

    if (entry) {                        // if an entry already exists
        void* oldValue = entry->value;  // keep track of the old value
        entry->value = value;           // replace the entry's value by the new value
        return oldValue;
    } else {
        unsigned int bucket = hashTable->hash(key);                  // get the bucket id
        HashTableEntry* bucketEntries = hashTable->buckets[bucket];  // get pointer to the bucket
        HashTableEntry* newEntry = createHashTableEntry(key, value);
        hashTable->buckets[bucket] = newEntry;  // add the new entry at the start of the bucket

        if (bucketEntries) {                 // if the bucket is not empty
            newEntry->next = bucketEntries;  // append the original list to the new head
        }                                    // else, do nothing

        return NULL;
    }
}

/**
 * getItem
 *
 * Get the value that corresponds to the key in the hash table.
 *
 * @param myHashTable The pointer to the hash table.
 * @param key The key that corresponds to the item.
 * @return the value corresponding to the key, or NULL if the key is not present
 */
void* getItem(HashTable* hashTable, unsigned int key) {
    HashTableEntry* entry = findItem(hashTable, key);

    if (entry) {
        // printf("Entry is not null");
        return entry->value;
    }

    return NULL;
}

/** removeEntryFromBucket
 * 
 * Remove an entry from a bucket and free its memory
 * 
 *
 * @param hashTable The pointer to the hash table.
 * @param key The key that corresponds to the item.
 */

void removeEntryFromBucket(HashTable* hashTable, unsigned int key) {
    unsigned int bucket = hashTable->hash(key);

    HashTableEntry* thisNode = hashTable->buckets[bucket];

    if (!thisNode) {
        return;
    }

    void* value;

    // if the wanted node is the first node
    if (thisNode->key == key) {
        hashTable->buckets[bucket] = thisNode->next;
        value = thisNode->value;
        if (value) {
            free(value);
        }
        free(thisNode);
        return;
    }

    // if the wanted node is not the first node
    while (thisNode) {
        // printf("middle remove\n");
        if (thisNode->next->key == key) {
            // the next node is to be removed
            HashTableEntry* tmp = thisNode->next->next;
            value = thisNode->next->value;
            if (value) {
                free(value);
            }
            free(thisNode->next);
            thisNode->next = tmp;
            return;
        }
        thisNode = thisNode->next;
    }
}

/**
 * removeItem
 *
 * Remove the item in hash table based on the key and return the value stored in it.
 * In other words, return the value and free the hash table entry from heap.
 *
 * @param hashTable The pointer to the hash table.
 * @param key The key that corresponds to the item.
 * @return the pointer of the value corresponding to the key, or NULL if the key is not present
 */
void* removeItem(HashTable* hashTable, unsigned int key) {
    HashTableEntry* entry = findItem(hashTable, key);

    if (entry) {
        void* value = entry->value;
        removeEntryFromBucket(hashTable, key);
        return value;
    }

    return NULL;
}

/**
 * deleteItem
 *
 * Delete the item in the hash table based on the key. In other words, free the
 * value stored in the hash table entry and the hash table entry itself from
 * the heap.
 *
 * @param hashTable The pointer to the hash table.
 * @param key The key that corresponds to the item.
 *
 */
void deleteItem(HashTable* hashTable, unsigned int key) {
    removeItem(hashTable, key);
}
