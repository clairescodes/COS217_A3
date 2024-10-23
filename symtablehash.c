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