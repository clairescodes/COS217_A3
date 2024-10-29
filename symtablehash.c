/* 
 * symtablehash.c
 *
 * Symbol table module implementation 
 * via hash table & separate chaining. 
 * functionalities include:
 * - creating and deleting a symbol table 
 * - adding and removing key-value pairs
 * - retrieving, replacing, checking existence of keys
 * - applying a user-defined function to each entry 
 */

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "symtable.h"
#define INITIAL_BUCKET_COUNT 509
#define RESIZE_FACTOR 0.5

/* array of available bucket sizes, used for hash table resizing */
static const size_t availBucketSize[] = {509, 1021, 2039, 4093, 
    8191, 16381, 32749, 65521};
/* number of elements */
static size_t numBucketSizes = sizeof(availBucketSize) 
    / sizeof(availBucketSize[0]);

/* key value pair node structure */
struct SymTableNode {
    /* key string */
    char *pcKey;
    /* value for key */
    void *pvValue;
    /* pointer to next node in the bucket */
    struct SymTableNode *psNext;  
};

/* symbol table structure */
struct SymTable {
    /* array of bucket pointers */
    struct SymTableNode **buckets;
    /* number of buckets */
    size_t numBuckets;
    /* number of bindings */
    size_t numBindings;
};

/*
 * Compute hash code for given key string pcKey
 * using the number of buckets provided as uNumBuckets.
 * Return size_t hash index. 
 */
static size_t SymTable_hash(const char *pcKey, size_t uNumBuckets) {
    const size_t HASH_MULTIPLIER = 65599;
    size_t u;
    size_t uHash = 0;

    assert(pcKey != NULL);

    for (u = 0; pcKey[u] != '\0'; u++)
        uHash = uHash * HASH_MULTIPLIER + (size_t)pcKey[u];

    return uHash % uNumBuckets;
}

/* 
 * Helper function that resizes symbol table oSymTable by 
 * increasing number of buckets. Then all bindings that already
 * exist are rehashed into new buckets. Returns 1 if resizing is
 * successful, and 0 otherwise. 
 */
static int SymTable_resize(SymTable_T oSymTable) {
    size_t newBucketSize; 
    size_t oldBucketSize = oSymTable->numBuckets; 
    size_t i; 
    struct SymTableNode **newBuckets; 

    size_t currentIndex = 0;
    while (currentIndex < numBucketSizes 
            && availBucketSize[currentIndex] <= oldBucketSize) {
        currentIndex++;
    }
    /* reached maximum bucket size */
    if (currentIndex >= numBucketSizes) return 1; 

    newBucketSize = availBucketSize[currentIndex];
    newBuckets = calloc(newBucketSize, sizeof(struct SymTableNode *));
    if (!newBuckets) return 0;

    for (i = 0; i < oldBucketSize; i++) {
        struct SymTableNode *node = oSymTable->buckets[i];
        while (node) {
            struct SymTableNode *nextNode = node->psNext;
            size_t newIndex = SymTable_hash(node->pcKey, 
                                            newBucketSize);
            node->psNext = newBuckets[newIndex];
            newBuckets[newIndex] = node;
            node = nextNode;
        }
    }

    free(oSymTable->buckets);
    oSymTable->buckets = newBuckets;
    oSymTable->numBuckets = newBucketSize;

    return 1;


}

/*
 * Creates and returns a empty SymTable_T symbol table, 
 * and allocate memory for the symbol table structure & buckets.
 * Returns NULL if memory allocation is unsuccessful.
 */
SymTable_T SymTable_new(void) {
    SymTable_T oSymTable;
    size_t i; 
    
    oSymTable = malloc(sizeof(struct SymTable));
    if (oSymTable == NULL)
        return NULL;
    
    oSymTable->buckets = malloc(sizeof(struct SymTableNode *) 
                                    * INITIAL_BUCKET_COUNT); 
    if (oSymTable->buckets == NULL) {
        free(oSymTable); 
        return NULL;
    }
    
    for (i = 0; i < INITIAL_BUCKET_COUNT; i++)
        oSymTable->buckets[i] = NULL; 
    
    oSymTable->numBuckets = INITIAL_BUCKET_COUNT; 
    oSymTable->numBindings = 0; 

    return oSymTable;   
}

/*
 * Free all memory associated with symbol table oSymTable.
 * Thus key-value bindings as well as the symbol table itself 
 * are freed. If oSymTable is NULL, nothing is freed. 
 */
void SymTable_free(SymTable_T oSymTable) {
    struct SymTableNode *psCurrentNode; 
    struct SymTableNode *psNextNode; 
    size_t i; 

    assert(oSymTable != NULL); 

    for (i = 0; i < oSymTable->numBuckets; i++){
        psCurrentNode = oSymTable->buckets[i]; 
        while (psCurrentNode != NULL) {
            psNextNode = psCurrentNode->psNext; 
            free(psCurrentNode->pcKey); 
            free(psCurrentNode); 
            psCurrentNode = psNextNode; 
        }
    }

    free(oSymTable->buckets);
    free(oSymTable); 
}

/* Returns number of bindings in oSymTable. */
size_t SymTable_getLength(SymTable_T oSymTable) {
    assert(oSymTable != NULL);
    return oSymTable->numBindings;
}

/* 
 * Adds new binding of pcKey, pvValue to symbol table (oSymTable) 
 * if the key pcKey doesn't exist in oSymTable. 
 * returns 1 if binding was added, returns 0 if memory allocation 
 * fails or key already exists in oSymTable 
 */
int SymTable_put(SymTable_T oSymTable,
    const char *pcKey, const void *pvValue) {
        struct SymTableNode *psNewNode; 
        struct SymTableNode *psCurrentNode; 
        size_t hashIndex; 
        
        assert(oSymTable != NULL); 
        assert(pcKey != NULL); 

        /* check if need resizing */
        if ((double)oSymTable->numBindings / oSymTable->numBuckets 
                                                    > RESIZE_FACTOR) {
        if (!SymTable_resize(oSymTable)) {
            /* case when resizing fails */
            return 0; 
            }
        }
        
        hashIndex = SymTable_hash(pcKey, oSymTable->numBuckets); 

        /* does not insert key if it already exists */
        for (psCurrentNode = oSymTable->buckets[hashIndex]; 
            psCurrentNode != NULL; 
            psCurrentNode = psCurrentNode-> psNext) {
                if (strcmp(psCurrentNode->pcKey, pcKey) == 0)
                    return 0; 
            }
        
        /* make a new node & check memory is allocated corectly */
        psNewNode = malloc(sizeof(struct SymTableNode)); 
        if (psNewNode == NULL)
            return 0; 

        /* defensive copy */
        psNewNode->pcKey = malloc(strlen(pcKey) + 1);
        if (psNewNode->pcKey == NULL) {
            free(psNewNode);
            return 0;
        }
        strcpy(psNewNode->pcKey, pcKey);
        psNewNode->pvValue = (void *)pvValue;
        psNewNode->psNext = oSymTable->buckets[hashIndex];
        oSymTable->buckets[hashIndex] = psNewNode;
        oSymTable->numBindings++; 
        return 1;
}

/* 
 * replace old value (pvOldValue) of key pcKey in oSymTable
 * with new value pvValue and returns pvOldValue. 
 * if the given pcKey doesn't exist within oSymTable, 
 * returns NULL 
 */
void *SymTable_replace(SymTable_T oSymTable,
    const char *pcKey, const void *pvValue) {
        struct SymTableNode *psCurrentNode; 
        size_t hashIndex; 
        void *pvOldValue; 

        assert(oSymTable != NULL); 
        assert(pcKey != NULL); 

        hashIndex = SymTable_hash(pcKey, oSymTable->numBuckets); 

        /* find key in bucket and replace */
        for (psCurrentNode = oSymTable->buckets[hashIndex]; 
            psCurrentNode != NULL; 
            psCurrentNode = psCurrentNode->psNext) {
                if (strcmp(psCurrentNode->pcKey, pcKey) == 0) {
                    pvOldValue = psCurrentNode->pvValue; 
                    psCurrentNode->pvValue = (void *)pvValue; 
                    return pvOldValue; 
                }
            }

        return NULL; 
    }


/* 
 * Checks if given key pcKey exists within oSymTable. 
 * Returns 1 if pcKey exists, if else returns 0 
 */
int SymTable_contains(SymTable_T oSymTable, const char *pcKey) {
    struct SymTableNode *psCurrentNode; 
    size_t hashIndex; 

    assert(oSymTable != NULL);
    assert(pcKey != NULL);

    hashIndex = SymTable_hash(pcKey, oSymTable->numBuckets); 

    for (psCurrentNode = oSymTable->buckets[hashIndex]; 
        psCurrentNode != NULL; 
        psCurrentNode = psCurrentNode->psNext) {
            if (strcmp(psCurrentNode->pcKey, pcKey) == 0)
                return 1; 
        }
    return 0; 
}

/* 
 * Returns value associated with key pcKey if it exists 
 * within oSymTable, returns NULL if can't find given pcKey
 * in oSymTable 
 */
void *SymTable_get(SymTable_T oSymTable, const char *pcKey) {
    struct SymTableNode *psCurrentNode;
    size_t hashIndex; 

    assert(oSymTable != NULL);
    assert(pcKey != NULL);

    hashIndex = SymTable_hash(pcKey, oSymTable->numBuckets);

    /* find key by traversing bucket */
    for (psCurrentNode = oSymTable->buckets[hashIndex];
         psCurrentNode != NULL;
         psCurrentNode = psCurrentNode->psNext) {
        if (strcmp(psCurrentNode->pcKey, pcKey) == 0)
            return psCurrentNode->pvValue;
    }
    return NULL; 
}

/* 
 * Removes binding associated with given pcKey from the
 * symbol table (oSymTable) and frees memory.
 * Return the value associated with removed key if key existed
 * in oSymTable. return NULL otherwise. 
 */
void *SymTable_remove(SymTable_T oSymTable, const char *pcKey) {
    struct SymTableNode *psCurrentNode;
    struct SymTableNode *psPreviousNode = NULL;
    size_t hashIndex; 
    void *pvValue; 

    assert (oSymTable != NULL); 
    assert (pcKey != NULL);
    hashIndex = SymTable_hash(pcKey, oSymTable->numBuckets); 

    psCurrentNode = oSymTable->buckets[hashIndex];
    while (psCurrentNode != NULL) {
        if (strcmp(psCurrentNode->pcKey, pcKey) == 0) {
            pvValue = psCurrentNode->pvValue;
            
            if (psPreviousNode == NULL) { /* case of a head node */
                oSymTable->buckets[hashIndex] = psCurrentNode->psNext;
            } else {
                psPreviousNode->psNext = psCurrentNode->psNext;
            }

            free(psCurrentNode->pcKey); 
            free(psCurrentNode); 

            oSymTable->numBindings--; 
            return pvValue; 
        }
        psPreviousNode = psCurrentNode;
        psCurrentNode = psCurrentNode->psNext; 
    }
    return NULL; 
}

/* 
 * To each binding in oSymTable, apply function (pfApply) given by
 * the user. user is able to input additional parameter pvExtra
 * if needed for the user defined function. 
 */
void SymTable_map(SymTable_T oSymTable,
    void (*pfApply)(const char *pcKey, void *pvValue, void *pvExtra),
    const void *pvExtra) {
    struct SymTableNode *psCurrentNode;
    size_t i;

    assert(oSymTable != NULL);
    assert(pfApply != NULL);

    for (i = 0; i < oSymTable->numBuckets; i++) {
        psCurrentNode = oSymTable->buckets[i];
        while (psCurrentNode != NULL) {
            (*pfApply)(psCurrentNode->pcKey, psCurrentNode->pvValue, (void *)pvExtra);
            psCurrentNode = psCurrentNode->psNext;
        }

    }

}
