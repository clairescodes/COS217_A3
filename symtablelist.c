#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "symtable.h"

/* binding that stores pcKey, pvValue, and psNext */
struct SymTableNode {
    /* key string */
    char *pcKey;
    /* value for key */
    void *pvValue;
    /* pointer to next node */
    struct SymTableNode *psNext;
};

/* symbol table that consists of binding nodes */
struct SymTable {
    /* pointer to first node */
    struct SymTableNode *psFirst;
    /* stores symbol table's number of bindings */
    size_t numBindings; 
}; 

/* creates a empty SymTable, allocates memory for it, 
   and returns it */
SymTable_T SymTable_new(void) {
    SymTable_T oSymTable; 

    oSymTable = malloc(sizeof(struct SymTable));
    if (oSymTable == NULL)
        return NULL;

    oSymTable->psFirst = NULL;
    oSymTable->numBindings = 0;
    return oSymTable;
}

/* frees memory needed for the SymTable */
void SymTable_free(SymTable_T oSymTable) {
    struct SymTableNode *psCurrentNode;
    struct SymTableNode *psNextNode;

    assert(oSymTable != NULL);

    for (psCurrentNode = oSymTable->psFirst;
         psCurrentNode != NULL;
         psCurrentNode = psNextNode) {
        psNextNode = psCurrentNode->psNext;
        free(psCurrentNode->pcKey);
        free(psCurrentNode);
    }

    free(oSymTable);
}

/* returns number of bindings in the symbol table (oSymTable) */
size_t SymTable_getLength(SymTable_T oSymTable) {
    assert(oSymTable != NULL);
    return oSymTable->numBindings;
}

/* adds new binding of pcKey, pvValue to symbol table (oSymTable) 
   if the key pcKey doesn't exist in oSymTable. 
   returns 1 if binding was added, returns 0 if memory allocation 
   fails or key already exists in oSymTable */
int SymTable_put(SymTable_T oSymTable, 
    const char *pcKey, const void *pvValue) {
    struct SymTableNode *psNewNode;
    struct SymTableNode *psCurrentNode;

    assert(oSymTable != NULL);
    assert(pcKey != NULL);
    assert(pvValue != NULL);


    /* check if there exists a duplicate key */
    for (psCurrentNode = oSymTable->psFirst; psCurrentNode != NULL;
         psCurrentNode = psCurrentNode->psNext) {
        if (strcmp(psCurrentNode->pcKey, pcKey) == 0)
            return 0; 
    }

    /* create new node */
    psNewNode = malloc(sizeof(struct SymTableNode));
    if (psNewNode == NULL)
        return 0;

    /* defensive copy of the key */
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

/* replace old value (pvOldValue) of key pcKey in oSymTable 
    with new value pvValue and returns pvOldValue. 
    if the given pcKey doesn't exist within oSymTable, 
    returns NULL */
void *SymTable_replace(SymTable_T oSymTable,
    const char *pcKey, const void *pvValue) {
    
    struct SymTableNode *psCurrentNode;
    void *pvOldValue; 
    assert(oSymTable != NULL);
    assert(pcKey != NULL); 
    assert(pvValue != NULL); 

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

/* checks if given key pcKey exists within oSymTable. 
    returns 1 if pcKey exists, if else returns 0 */
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

/* returns value associated with key pcKey if it exists 
    within oSymTable, returns NULL if can't find given pcKey
    in oSymTable */
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

/* removes binding associated with given pcKey from the
   symbol table (oSymTable) and frees memory.
   return value associated with removed key if key existed
   in oSymTable. return NULL otherwise. */
void *SymTable_remove(SymTable_T oSymTable, const char *pcKey) {
    struct SymTableNode *psCurrentNode;
    struct SymTableNode *psPreviousNode = NULL;
    void *pvValue;

    assert(oSymTable != NULL);
    assert(pcKey != NULL);

    for (psCurrentNode = oSymTable->psFirst; 
        psCurrentNode != NULL; 
        psCurrentNode = psCurrentNode->psNext) {
            /* traverse and remove node from list */
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

/* to each binding in oSymTable, apply function (pfApply) given by
   the user. */
void SymTable_map(SymTable_T oSymTable,
                    void (*pfApply)(const char *pcKey, void *pvValue, 
                    void *pvExtra), const void *pvExtra) {
    struct SymTableNode *psCurrentNode; 
    assert(oSymTable != NULL);
    assert(pfApply != NULL);
    assert(pvExtra != NULL);
    for (psCurrentNode = oSymTable->psFirst; 
        psCurrentNode != NULL; 
        psCurrentNode = psCurrentNode->psNext) {
            (*pfApply)(psCurrentNode->pcKey, psCurrentNode->pvValue,
            (void *)pvExtra); 
        }

    }
