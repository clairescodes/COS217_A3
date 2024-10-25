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
    size_t bucketCount;
    size_t numBindings;
};

static size_t SymTable_hash(const char *pcKey, size_t uBucketCount) {
    const size_t HASH_MULTIPLIER = 65599;
    size_t u;
    size_t uHash = 0;

    assert(pcKey != NULL);

    for (u = 0; pcKey[u] != '\0'; u++)
        uHash = uHash * HASH_MULTIPLIER + (size_t)pcKey[u];

    return uHash % uBucketCount;
}

SymTable_T SymTable_new(void) {
   SymTable_T oSymTable;
   size_t i; 


   oSymTable = malloc(sizeof(struct SymTable));
   if (oSymTable == NULL)
    return NULL;
   
   oSymTable->buckets = 


    
}

void SymTable_free(SymTable_T oSymTable);

size_t SymTable_getLength(SymTable_T oSymTable);

int SymTable_put(SymTable_T oSymTable,
    const char *pcKey, const void *pvValue);

void *SymTable_replace(SymTable_T oSymTable,
    const char *pcKey, const void *pvValue);

int SymTable_contains(SymTable_T oSymTable, const char *pcKey);

void *SymTable_get(SymTable_T oSymTable, const char *pcKey);

void *SymTable_remove(SymTable_T oSymTable, const char *pcKey);

void SymTable_map(SymTable_T oSymTable,
    void (*pfApply)(const char *pcKey, void *pvValue, void *pvExtra),
    const void *pvExtra);
