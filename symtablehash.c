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
static const size_t auAvailBucketSize[] = {509, 1021, 2039, 4093, 
    8191, 16381, 32749, 65521};
/* number of elements */
static size_t uNumBucketSizes = sizeof(auAvailBucketSize) 
    / sizeof(auAvailBucketSize[0]);

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
    struct SymTableNode **ppsBuckets;
    /* number of buckets */
    size_t uNumBuckets;
    /* number of bindings */
    size_t uNumBindings;
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
    size_t uNewBucketSize; 
    size_t uOldBucketSize = oSymTable->uNumBuckets; 
    size_t i; 
    struct SymTableNode **ppsNewBuckets; 

    size_t uCurrentIndex = 0;
    /* find the next bucket size that's available */
    while (uCurrentIndex < uNumBucketSizes 
            && auAvailBucketSize[uCurrentIndex] 
            <= uOldBucketSize) {
        uCurrentIndex++;
    }
    /* if reach maximum bucket size, stop resizing */
    if (uCurrentIndex >= uNumBucketSizes) return 1; 

    /* set new bucket size and allocate memory for the new array */
    uNewBucketSize = auAvailBucketSize[uCurrentIndex];
    ppsNewBuckets = calloc(uNewBucketSize, 
                           sizeof(struct SymTableNode *));
    /* case when memory allocation fails */
    if (!ppsNewBuckets) return 0; 

    /* rehash each node from old to new bucket */
    for (i = 0; i < uOldBucketSize; i++) {
    struct SymTableNode *psNode = oSymTable->ppsBuckets[i];
    while (psNode) {
        struct SymTableNode *psNextNode = psNode->psNext;
        size_t newIndex 
               = SymTable_hash(psNode->pcKey, uNewBucketSize);
        /* insert node to new bucket at the index calculated */
        psNode->psNext = ppsNewBuckets[newIndex];
        ppsNewBuckets[newIndex] = psNode;

        psNode = psNextNode;
        }
    }
    free(oSymTable->ppsBuckets);
    oSymTable->ppsBuckets = ppsNewBuckets;
    oSymTable->uNumBuckets = uNewBucketSize;

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
    
    /* memory allocation for SymTable */
    oSymTable = malloc(sizeof(struct SymTable));
    if (oSymTable == NULL)
        return NULL;
    
    /* memory allocation for bucket pointer array */
    oSymTable->ppsBuckets = malloc(sizeof(struct SymTableNode *) 
                                    * INITIAL_BUCKET_COUNT); 
    if (oSymTable->ppsBuckets == NULL) {
        /* have to free oSymTable 
           if bucket memory allocation fails */
        free(oSymTable); 
        return NULL;
    }
    
    /* initialize bucket pointers */
    for (i = 0; i < INITIAL_BUCKET_COUNT; i++)
        oSymTable->ppsBuckets[i] = NULL; 
    
    oSymTable->uNumBuckets = INITIAL_BUCKET_COUNT; 
    oSymTable->uNumBindings = 0; 

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

    for (i = 0; i < oSymTable->uNumBuckets; i++){
        /* start at the first node of each bucket's chain */
        psCurrentNode = oSymTable->ppsBuckets[i]; 
        while (psCurrentNode != NULL) {
            /* have to save next node in chain before freeing */
            psNextNode = psCurrentNode->psNext; 
            /* free memory for both key and current node */
            free(psCurrentNode->pcKey); 
            free(psCurrentNode); 
            psCurrentNode = psNextNode; 
        }
    }

    /* free memory for both array of bucket pointers & symbol table */
    free(oSymTable->ppsBuckets);
    free(oSymTable); 
}

/* Returns number of bindings in oSymTable. */
size_t SymTable_getLength(SymTable_T oSymTable) {
    assert(oSymTable != NULL);
    return oSymTable->uNumBindings;
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
        size_t uHashIndex; 
        
        assert(oSymTable != NULL); 
        assert(pcKey != NULL); 

        /* determine if resizing is needed. 
           comment out the if statement to disable resizing */
        if ((double)oSymTable->uNumBindings / oSymTable->uNumBuckets 
                                                    > RESIZE_FACTOR) {
            /* case when resizing fails */
            if (!SymTable_resize(oSymTable)) return 0; 
        }
        
        uHashIndex = SymTable_hash(pcKey, oSymTable->uNumBuckets); 

        /* does not insert key if it already exists */
        for (psCurrentNode = oSymTable->ppsBuckets[uHashIndex]; 
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
        /* copy key into the new memory */
        strcpy(psNewNode->pcKey, pcKey);

        /* link new node to the chain's front */
        psNewNode->pvValue = (void *)pvValue;
        psNewNode->psNext = oSymTable->ppsBuckets[uHashIndex];
        oSymTable->ppsBuckets[uHashIndex] = psNewNode;
        oSymTable->uNumBindings++; 
        
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
        size_t uHashIndex; 
        void *pvOldValue; 

        assert(oSymTable != NULL); 
        assert(pcKey != NULL); 
        
        /* determine which bucket key is located in */
        uHashIndex = SymTable_hash(pcKey, oSymTable->uNumBuckets); 

        /* traverse chain of nodes to find & replace node 
           with matching key */
        for (psCurrentNode = oSymTable->ppsBuckets[uHashIndex]; 
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
    size_t uHashIndex; 

    assert(oSymTable != NULL);
    assert(pcKey != NULL);
    
    /* determine which bucket key is located in */
    uHashIndex = SymTable_hash(pcKey, oSymTable->uNumBuckets); 

    /* traverse chain of nodes to find node with matching key */
    for (psCurrentNode = oSymTable->ppsBuckets[uHashIndex]; 
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
    size_t uHashIndex; 

    assert(oSymTable != NULL);
    assert(pcKey != NULL);

    /* determine which bucket key is located in */
    uHashIndex = SymTable_hash(pcKey, oSymTable->uNumBuckets);

    /* find key by traversing bucket, return value */
    for (psCurrentNode = oSymTable->ppsBuckets[uHashIndex];
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
    size_t uHashIndex; 
    void *pvValue; 

    assert (oSymTable != NULL); 
    assert (pcKey != NULL);

    /* determine which bucket key is located in */
    uHashIndex = SymTable_hash(pcKey, oSymTable->uNumBuckets); 

    psCurrentNode = oSymTable->ppsBuckets[uHashIndex];
    while (psCurrentNode != NULL) {
        if (strcmp(psCurrentNode->pcKey, pcKey) == 0) {
            /* store value so that it can be returned after removal */
            pvValue = psCurrentNode->pvValue;
            
            if (psPreviousNode == NULL) { /* head node case */
                oSymTable->ppsBuckets[uHashIndex] 
                = psCurrentNode->psNext;
            } else {
                psPreviousNode->psNext = psCurrentNode->psNext;
            }

            /* free memory for both key and node */
            free(psCurrentNode->pcKey); 
            free(psCurrentNode); 

            oSymTable->uNumBindings--; 
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

    for (i = 0; i < oSymTable->uNumBuckets; i++) {
        psCurrentNode = oSymTable->ppsBuckets[i]; 
        /* traverse each key value pair and apply pfApply with  */
        while (psCurrentNode != NULL) {
            (*pfApply)(psCurrentNode->pcKey, 
                       psCurrentNode->pvValue, (void *)pvExtra);
            psCurrentNode = psCurrentNode->psNext;
        }

    }

}
