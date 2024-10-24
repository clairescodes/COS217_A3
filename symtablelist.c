#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "symtable.h"

/* binding in symbol table */
struct SymTableNode {
    char *pcKey;                   /* key string owned by SymTable */
    void *pvValue;                 /* value for key */
    struct SymTableNode *psNext;   /* pointer to next node */
};

struct SymTable {
    struct SymTableNode *psFirst; 
    size_t numBindings; 
}; 

SymTable_T SymTable_new(void) {
    SymTable_T oSymTable;

    oSymTable = malloc(sizeof(struct SymTable));
    if (oSymTable == NULL)
        return NULL;

    oSymTable->psFirst = NULL;
    oSymTable->numBindings = 0;
    return oSymTable;
}

void SymTable_free(SymTable_T oSymTable) {
    struct SymTableNode *psCurrentNode;
    struct SymTableNode *psNextNode;

    assert(oSymTable != NULL);

    for (psCurrentNode = oSymTable->psFirst;
         psCurrentNode != NULL;
         psCurrentNode = psNextNode) {
        psNextNode = psCurrentNode->psNext;
        free(psCurrentNode->pcKey);  /* Free the key string */
        free(psCurrentNode);         /* Free the node */
    }

    free(oSymTable);
}; 

size_t SymTable_getLength(SymTable_T oSymTable) {
    assert(oSymTable != NULL);
    return oSymTable->numBindings;
}

int SymTable_put(SymTable_T oSymTable, const char *pcKey, const void *pvValue) {
    struct SymTableNode *psNewNode;
    struct SymTableNode *psCurrentNode;

    assert(oSymTable != NULL);
    assert(pcKey != NULL);

    /* Check for duplicate key */
    for (psCurrentNode = oSymTable->psFirst;
         psCurrentNode != NULL;
         psCurrentNode = psCurrentNode->psNext) {
        if (strcmp(psCurrentNode->pcKey, pcKey) == 0)
            return 0;  /* Key already exists */
    }

    /* Create new node */
    psNewNode = malloc(sizeof(struct SymTableNode));
    if (psNewNode == NULL)
        return 0;

    /* Make a defensive copy of the key */
    psNewNode->pcKey = malloc(strlen(pcKey) + 1);
    if (psNewNode->pcKey == NULL) {
        free(psNewNode);
        return 0;
    }
    strcpy(psNewNode->pcKey, pcKey);

    psNewNode->pvValue = (void *)pvValue;
    psNewNode->psNext = oSymTable->psFirst;
    oSymTable->psFirst = psNewNode;
    oSymTable->numBindings++;
    return 1;
}

void *SymTable_replace(SymTable_T oSymTable,
    const char *pcKey, const void *pvValue) {
    
    struct SymTableNode *psCurrentNode;
    void *pvOldValue; 
    assert(oSymTable != NULL);
    assert(pcKey != NULL); 

    for (psCurrentNode = oSymTable -> psFirst; 
    psCurrentNode != NULL; psCurrentNode = psCurrentNode->psNext) {

        if (strcmp(psCurrentNode -> pcKey, pcKey) == 0) {
            pvOldValue = psCurrentNode->pvValue;
            psCurrentNode->pvValue = (void *)pvValue; 
            return pvOldValue;
        }
    }
    return NULL; 
}

int SymTable_contains(SymTable_T oSymTable, const char *pcKey) {
    struct SymTableNode *psCurrentNode;
    assert(oSymTable != NULL);
    assert(pcKey != NULL);
    for (psCurrentNode = oSymTable->psFirst; 
        psCurrentNode != NULL; 
        psCurrentNode = psCurrentNode->psNext) {
            if (strcmp(psCurrentNode->pcKey, pcKey) == 0) {
                return 1; 
                }
    }
    return 0; 
}

void *SymTable_get(SymTable_T oSymTable, const char *pcKey) {
    struct SymTableNode *psCurrentNode;
    assert(oSymTable != NULL); 
    assert(pcKey != NULL); 
    for (psCurrentNode = oSymTable->psFirst;
        psCurrentNode != NULL; 
        psCurrentNode = psCurrentNode->psNext) {
            if (strcmp(psCurrentNode->pcKey, pcKey) == 0) {
                return psCurrentNode->pvValue; 
            }
    }
    return NULL; 
}

void *SymTable_remove(SymTable_T oSymTable, const char *pcKey) {
    struct SymTableNode *psCurrentNode;
    struct SymTableNode *psPreviousNode = NULL;
    void *pvValue;

    assert(oSymTable != NULL);
    assert(pcKey != NULL);

    for (psCurrentNode = oSymTable->psFirst; 
        psCurrentNode != NULL; 
        psCurrentNode = psCurrentNode->psNext) {
            /* remove node from list */
            if (strcmp(psCurrentNode->pcKey, pcKey) == 0) {
                pvValue = psCurrentNode->pvValue; 
                if (psPreviousNode == NULL) {
                    oSymTable->psFirst = psCurrentNode->psNext;
            } else {
                psPreviousNode->psNext = psCurrentNode->psNext;
            }
            free(psCurrentNode->pcKey); 
            free(psCurrentNode); 
            oSymTable->numBindings--; 
            return pvValue;

            }
            psPreviousNode = psCurrentNode; 
        }
        return NULL; 
}

void SymTable_map(SymTable_T oSymTable,
                    void (*pfApply)(const char *pcKey, void *pvValue, 
                    void *pvExtra), const void *pvExtra) {
    struct SymTableNode *psCurrentNode; 
    assert(oSymTable != NULL);
    assert(pfApply != NULL);
    for (psCurrentNode = oSymTable->psFirst; 
        psCurrentNode != NULL; 
        psCurrentNode = psCurrentNode->psNext) {
            (*pfApply)(psCurrentNode->pcKey, psCurrentNode->pvValue,
            (void *)pvExtra); 
        }

    }
