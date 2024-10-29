/* symbol table implementation that holds key-value 
   pairs.
   the key is a string and value is a void pointer
   that can store any datatype. 
   functionalities include: 
   - create & delete symbol table 
   - add & remove key-value pairs
   - retrieve key-value pairs
   - replace key-value pairs
   - check if key exists in symbol table 
   - apply user defined function to each entry 
   */
#ifndef SYMTABLE_INCLUDED
#define SYMTABLE_INCLUDED

#include <stddef.h>

/* abstract data type representing a symbol table */
typedef struct SymTable *SymTable_T;

/* creates a empty SymTable, allocates memory for it, 
   and returns it */
SymTable_T SymTable_new(void);

/* frees memory needed for symbol table oSymTable */
void SymTable_free(SymTable_T oSymTable);

/* returns number of bindings in oSymTable */
size_t SymTable_getLength(SymTable_T oSymTable);

/* adds new binding of pcKey, pvValue to symbol table (oSymTable) 
   if the key pcKey doesn't exist in oSymTable. 
   returns 1 if binding was added, returns 0 if memory allocation 
   fails or key already exists in oSymTable */
int SymTable_put(SymTable_T oSymTable,
    const char *pcKey, const void *pvValue);

/* replace old value (pvOldValue) of key pcKey in oSymTable 
    with new value pvValue and returns pvOldValue. 
    if the given pcKey doesn't exist within oSymTable, 
    returns NULL */
void *SymTable_replace(SymTable_T oSymTable,
    const char *pcKey, const void *pvValue);

/* checks if given key pcKey exists within oSymTable. 
    returns 1 if pcKey exists, if else returns 0 */
int SymTable_contains(SymTable_T oSymTable, const char *pcKey);

/* returns value associated with key pcKey if it exists 
    within oSymTable, returns NULL if can't find given pcKey
    in oSymTable */
void *SymTable_get(SymTable_T oSymTable, const char *pcKey);

/* removes binding associated with given pcKey from the
   symbol table (oSymTable) and frees memory.
   return value associated with removed key if key existed
   in oSymTable. return NULL otherwise. */
void *SymTable_remove(SymTable_T oSymTable, const char *pcKey);

/* to each binding in oSymTable, apply function (pfApply) given by
   the user. user is able to input additional parameter pvExtra
   if needed for the user defined function. */
void SymTable_map(SymTable_T oSymTable,
    void (*pfApply)(const char *pcKey, void *pvValue, void *pvExtra),
    const void *pvExtra);

#endif