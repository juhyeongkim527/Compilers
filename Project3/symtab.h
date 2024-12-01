/****************************************************/
/* File: symtab.h                                   */
/* Symbol table interface for the TINY compiler     */
/* (allows only one symbol table)                   */
/* Compiler Construction: Principles and Practice   */
/* Kenneth C. Louden                                */
/****************************************************/

#ifndef _SYMTAB_H_
#define _SYMTAB_H_

#include "globals.h"
/* SIZE is the size of the hash table */
#define SIZE 211

/* the list of line numbers of the source
 * code in which a variable is referenced
 */
typedef struct LineListRec
{
    int lineno;
    struct LineListRec *next;
} *LineList;

/* The record in the bucket lists for
 * each variable, including name,
 * assigned memory location, and
 * the list of line numbers in which
 * it appears in the source code
 */
typedef struct BucketListRec
{
    char *name;
    LineList lines;
    int memloc; /* memory location for variable */
    struct BucketListRec *next;
    TreeNode *treeNode;
} *BucketList;

typedef struct ScopeListRec
{
    char *name;
    BucketList hashTable[SIZE];
    struct ScopeListRec *parent;
} *ScopeList;

int addLocation();
void push_scope(ScopeList scope);
void pop_scope();
ScopeList get_top_scope();
ScopeList create_scope(char *name);

/* Procedure st_insert inserts line numbers and
 * memory locations into the symbol table
 * loc = memory location is inserted only the
 * first time, otherwise ignored
 */
void st_insert(char *name, int lineno, int loc, TreeNode *treeNode);

void st_insert_lineno(char *name, int lineno);

/* Function st_lookup returns the memory
 * location of a variable or -1 if not found
 */
int st_lookup(char *name);

BucketList st_lookup_return_bucket(char *name);

int st_lookup_current_scope(char *name);

/* Procedure printSymTab prints a formatted
 * listing of the symbol table contents
 * to the listing file
 */
void printSymTab(FILE *listing);

void printFunc(FILE *listing);

#endif
