/****************************************************/
/* File: symtab.c                                   */
/* Symbol table implementation for the TINY compiler*/
/* (allows only one symbol table)                   */
/* Symbol table is implemented as a chained         */
/* hash table                                       */
/* Compiler Construction: Principles and Practice   */
/* Kenneth C. Louden                                */
/****************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "symtab.h"

/* SHIFT is the power of two used as multiplier
   in hash function  */
#define SHIFT 4

/* the hash function */
static int hash(char *key)
{
  int temp = 0;
  int i = 0;
  while (key[i] != '\0')
  {
    temp = ((temp << SHIFT) + key[i]) % SIZE;
    ++i;
  }
  return temp;
}

/* the hash table */
// static BucketList hashTable[SIZE];

ScopeList scopeList[SIZE];
int sizeOfScopeList = 0;
ScopeList scopeStack[SIZE];
int sizeOfScopeStack = 0;
int location[SIZE];

int addLocation()
{
  return location[sizeOfScopeStack - 1]++;
}

ScopeList create_scope(char *name)
{
  ScopeList scope = (ScopeList)malloc(sizeof(struct ScopeListRec));
  scope->name = name;
  for (int i = 0; i < SIZE; i++)
  {
    scope->hashTable[i] = NULL;
  }
  scopeList[sizeOfScopeList++] = scope;
  scope->parent = scopeStack[sizeOfScopeStack - 1];
  return scope;
}

void push_scope(ScopeList scope)
{
  location[sizeOfScopeStack] = 0;
  scopeStack[sizeOfScopeStack++] = scope;
}

void pop_scope()
{
  sizeOfScopeStack--;
}

ScopeList get_top_scope()
{
  return scopeStack[sizeOfScopeStack - 1];
}

/* Procedure st_insert inserts line numbers and
 * memory locations into the symbol table
 * loc = memory location is inserted only the
 * first time, otherwise ignored
 */
void st_insert(char *name, int lineno, int loc, TreeNode *treeNode)
{
  int h = hash(name);
  ScopeList scope = get_top_scope();
  BucketList l = scope->hashTable[h];

  while ((l != NULL) && (strcmp(name, l->name) != 0))
    l = l->next;

  if (l == NULL) /* variable not yet in table */
  {
    l = (BucketList)malloc(sizeof(struct BucketListRec));
    l->name = name;
    l->lines = (LineList)malloc(sizeof(struct LineListRec));
    l->lines->lineno = lineno;
    l->memloc = loc;
    l->lines->next = NULL;
    l->next = scope->hashTable[h];
    scope->hashTable[h] = l;
    l->treeNode = treeNode;
  }
  // else /* found in table, so just add line number */
  // {
  //   LineList t = l->lines;
  //   while (t->next != NULL)
  //     t = t->next;
  //   t->next = (LineList)malloc(sizeof(struct LineListRec));
  //   t->next->lineno = lineno;
  //   t->next->next = NULL;
  // }
} /* st_insert */

void st_insert_lineno(char *name, int lineno)
{
  int h = hash(name);
  ScopeList scope = get_top_scope();
  BucketList bucket = scope->hashTable[h];

  while ((bucket != NULL) && (strcmp(name, bucket->name) != 0))
    bucket = bucket->next;

  if (bucket != NULL) /* variable not yet in table */
  {
    LineList line = bucket->lines;

    while (line->next != NULL)
      line = line->next;

    line->next = (LineList)malloc(sizeof(struct LineListRec));
    line->next->lineno = lineno;
    line->next->next = NULL;
  }
}

/* Function st_lookup returns the memory
 * location of a variable or -1 if not found
 */
int st_lookup(char *name)
{
  BucketList bucket = st_lookup_return_bucket(name);
  if (bucket == NULL)
    return -1;
  else
    return bucket->memloc;
}

BucketList st_lookup_return_bucket(char *name)
{
  int h = hash(name);
  ScopeList scope = get_top_scope();

  while (scope != NULL)
  {
    BucketList bucket = scope->hashTable[h];
    while ((bucket != NULL) && (strcmp(name, bucket->name) != 0))
      bucket = bucket->next;
    if (bucket != NULL)
      return bucket;
    scope = scope->parent;
  }
  return NULL;
}

int st_lookup_current_scope(char *name)
{
  int h = hash(name);
  ScopeList scope = get_top_scope();
  BucketList bucket = scope->hashTable[h];

  while ((bucket != NULL) && (strcmp(name, bucket->name) != 0))
    bucket = bucket->next;

  if (bucket == NULL)
    return -1;
  else
    return bucket->memloc;
}

/* Procedure printSymTab prints a formatted
 * listing of the symbol table contents
 * to the listing file
 */
void printSymTab(FILE *listing)
{
  int i;
  fprintf(listing, "< Symbol Table >\n");
  fprintf(listing, " Symbol Name   Symbol Kind   Symbol Type    Scope Name   Location  Line Numbers\n");
  fprintf(listing, "-------------  -----------  -------------  ------------  --------  ------------\n");
  for (i = 0; i < sizeOfScopeList; ++i)
  {
    ScopeList scope = scopeList[i];
    BucketList *hashTable = scope->hashTable;

    for (int j = 0; j < SIZE; ++j)
    {
      if (hashTable[j] == NULL)
        continue;

      BucketList l = hashTable[j];
      TreeNode *treeNode = l->treeNode;

      while (l != NULL)
      {
        fprintf(listing, "%-13s  ", l->name);

        switch (treeNode->nodekind)
        {
        case VarDeclK:
          fprintf(listing, "%-11s  ", "Variable");
          switch (treeNode->type)
          {
          case Int:
            fprintf(listing, "%-13s  ", "int");
            break;
          case IntArray:
            fprintf(listing, "%-13s  ", "int[]");
            break;
          default:
            break;
          }
          break;

        case FunDeclK:
          fprintf(listing, "%-11s  ", "Function");
          switch (treeNode->type)
          {
          case Int:
            fprintf(listing, "%-13s  ", "int");
            break;
          case Void:
            fprintf(listing, "%-13s  ", "void");
            break;
          default:
            break;
          }
          break;

        case ParamK:
          fprintf(listing, "%-11s  ", "Variable");
          switch (treeNode->type)
          {
          case Int:
            fprintf(listing, "%-13s  ", "int");
            break;
          case IntArray:
            fprintf(listing, "%-13s  ", "int[]");
            break;
          default:
            break;
          }
          break;

        default:
          break;
        }

        fprintf(listing, "%-13s  ", scope->name);
        fprintf(listing, "%-8d  ", l->memloc);

        LineList t = l->lines;
        while (t != NULL)
        {
          fprintf(listing, "%3d ", t->lineno);
          t = t->next;
        }
        fprintf(listing, "\n");
        l = l->next;
      }
    }
  }
} /* printSymTab */