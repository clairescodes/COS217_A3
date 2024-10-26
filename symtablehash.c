#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "symtable.h"
#define INITIAL_BUCKET_COUNT 509

/* key value pair node */
struct SymTableNode {
    char *pcKey;
    void *pvValue;
    struct SymTableNode *psNext;  /* next node in the bucket */
};

/* symbol table structure */
struct SymTable {
    struct SymTableNode **buckets;
    size_t numBuckets;
    size_t numBindings;
};

static size_t SymTable_hash(const char *pcKey, size_t numBuckets) {
    const size_t HASH_MULTIPLIER = 65599;
    size_t u;
    size_t uHash = 0;

    assert(pcKey != NULL);

    for (u = 0; pcKey[u] != '\0'; u++)
        uHash = uHash * HASH_MULTIPLIER + (size_t)pcKey[u];

    return uHash % numBuckets;
}

SymTable_T SymTable_new(void) {
    SymTable_T oSymTable;
    size_t i; 
    
    oSymTable = malloc(sizeof(struct SymTable));
    if (oSymTable == NULL)
        return NULL;
    
    oSymTable->buckets = malloc(sizeof(struct SymTableNode *) * INITIAL_BUCKET_COUNT); 
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

size_t SymTable_getLength(SymTable_T oSymTable) {
    assert(oSymTable != NULL);
    return oSymTable->numBindings;
}

int SymTable_put(SymTable_T oSymTable,
    const char *pcKey, const void *pvValue) {
        struct SymTableNode *psNewNode; 
        struct SymTableNode *psCurrentNode; 
        size_t hashIndex; 
        
        assert(oSymTable != NULL); 
        assert(pcKey != NULL); 
        
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
