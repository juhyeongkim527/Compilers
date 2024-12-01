/****************************************************/
/* File: analyze.c                                  */
/* Semantic analyzer implementation                 */
/* for the TINY compiler                            */
/* Compiler Construction: Principles and Practice   */
/* Kenneth C. Louden                                */
/****************************************************/

#include "globals.h"
#include "symtab.h"
#include "analyze.h"
#include "util.h"

/* counter for variable memory locations */
static int location = 0;

ScopeList globalScope = NULL;
char *curFuncName = NULL;
int isFuncScopeCreated = FALSE;

void undeclaredFunctionError(TreeNode *treeNode)
{
  fprintf(listing, "Error: undeclared function \"%s\" is called at line %d\n", treeNode->name, treeNode->lineno);
  Error = TRUE;
}

void undeclaredVariableError(TreeNode *treeNode)
{
  fprintf(listing, "Error: undeclared variable \"%s\" is used at line %d\n", treeNode->name, treeNode->lineno);
  Error = TRUE;
}

void voidTypeError(TreeNode *treeNode)
{
  fprintf(listing, "Error: The void-type variable is declared at line %d (name : \"%s\")\n", treeNode->lineno, treeNode->name);
  Error = TRUE;
}

void invalidArrayIndexingIntError(TreeNode *treeNode)
{
  fprintf(listing, "Error: Invalid array indexing at line %d (name : \"%s\"). indicies should be integer\n", treeNode->lineno, treeNode->name);
  Error = TRUE;
}

void invalidArrayIndexingNotArrayError(TreeNode *treeNode)
{
  fprintf(listing, "Error: Invalid array indexing at line %d (name : \"%s\"). indexing can only allowed for int[] variables\n", treeNode->lineno, treeNode->name);
  Error = TRUE;
}

void invalidFunctionCallError(TreeNode *treeNode)
{
  fprintf(listing, "Error: Invalid function call at line %d (name : \"%s\")\n", treeNode->lineno, treeNode->name);
  Error = TRUE;
}

void invalidReturnError(TreeNode *treeNode)
{
  fprintf(listing, "Error: Invalid return at line %d\n", treeNode->lineno);
  Error = TRUE;
}

void invalidAssignmentError(TreeNode *treeNode)
{
  fprintf(listing, "Error: invalid assignment at line %d\n", treeNode->lineno);
  Error = TRUE;
}

void invalidOperationError(TreeNode *treeNode)
{
  fprintf(listing, "Error: invalid operation at line %d\n", treeNode->lineno);
  Error = TRUE;
}

void invalidConditionError(TreeNode *treeNode)
{
  fprintf(listing, "Error: invalid condition at line %d\n", treeNode->lineno);
  Error = TRUE;
}

void redefinedSymbolError(TreeNode *treeNode, BucketList l)
{
  fprintf(listing, "Error: Symbol \"%s\" is redefined at line %d (already defined at line ", treeNode->name, treeNode->lineno);
  LineList t = l->lines;
  while (t != NULL)
  {
    fprintf(listing, "%d ", t->lineno);
    t = t->next;
  }
  fprintf(listing, ")\n");
  Error = TRUE;
}

void push_built_in_functions()
{
  TreeNode *inputNode = newTreeNode(FunDeclK);
  inputNode->lineno = 0;
  inputNode->type = Int;
  inputNode->name = "input";
  inputNode->child[0] = newTreeNode(ParamK);
  inputNode->child[0]->type = Void;
  st_insert("input", 0, addLocation(), inputNode);

  TreeNode *outputNode = newTreeNode(FunDeclK);
  outputNode->lineno = 0;
  outputNode->type = Void;
  outputNode->name = "output";
  TreeNode *param = newTreeNode(ParamK);
  param->type = Int;
  param->name = "value";
  outputNode->child[0] = param;
  st_insert("output", 0, addLocation(), outputNode);
  ScopeList scope = create_scope("output");
  push_scope(scope);
  st_insert("value", 0, addLocation(), param);
  pop_scope();
}

/* Procedure traverse is a generic recursive
 * syntax tree traversal routine:
 * it applies preProc in preorder and postProc
 * in postorder to tree pointed to by t
 */
static void
traverse(TreeNode *t,
         void (*preProc)(TreeNode *),
         void (*postProc)(TreeNode *))
{
  if (t != NULL)
  {
    // printf("nodekind: %d\n", t->nodekind);
    // printf("name: %s\n", t->name);
    // printf("type: %d\n", t->type);
    // printf("lineno: %d\n\n", t->lineno);
    preProc(t);
    {
      int i;
      for (i = 0; i < MAXCHILDREN; i++)
        traverse(t->child[i], preProc, postProc);
    }
    postProc(t);
    traverse(t->sibling, preProc, postProc);
  }
}

/* nullProc is a do-nothing procedure to
 * generate preorder-only or postorder-only
 * traversals from traverse
 */
static void nullProc(TreeNode *t)
{
  if (t == NULL)
    return;
  else
    return;
}

void postProcInsertNode(TreeNode *treeNode)
{
  if (treeNode->nodekind == CompStmtK)
    pop_scope();
}

/* Procedure insertNode inserts
 * identifiers stored in t into
 * the symbol table
 */
static void insertNode(TreeNode *t)
{
  switch (t->nodekind)
  {
  case VarDeclK:
    if (t->type == Void || t->type == VoidArray)
      voidTypeError(t);
    if (t->child[0] != NULL)
      t->type = IntArray;
    if (st_lookup_current_scope(t->name) != -1)
    {
      BucketList l = st_lookup_return_bucket(t->name);
      redefinedSymbolError(t, l);
      st_insert_lineno(t->name, t->lineno);
    }
    else
    {
      st_insert(t->name, t->lineno, addLocation(), t);
    }
    break;

  case FunDeclK:
    curFuncName = t->name;
    if (st_lookup_current_scope(t->name) != -1)
    {
      BucketList l = st_lookup_return_bucket(t->name);
      redefinedSymbolError(t, l);
    }
    else
    {
      st_insert(t->name, t->lineno, addLocation(), t);
      ScopeList scope = create_scope(t->name);
      push_scope(scope);
      isFuncScopeCreated = TRUE;
    }
    break;

  case ParamK:
    if (t->type == Void)
      break;
    if (t->type == VoidArray)
      voidTypeError(t);
    if (st_lookup_current_scope(t->name) != -1)
    {
      BucketList l = st_lookup_return_bucket(t->name);
      redefinedSymbolError(t, l);
    }
    else
    {
      st_insert(t->name, t->lineno, addLocation(), t);
      if (t->child[0] != NULL)
        t->type = IntArray;
    }
    break;

  case CompStmtK:
    if (isFuncScopeCreated)
      isFuncScopeCreated = FALSE;
    else
    {
      ScopeList scope = create_scope(curFuncName);
      push_scope(scope);
    }
    t->scope = get_top_scope();
    break;

  case CallK:
    if (st_lookup(t->name) == -1)
    {
      TreeNode *newUndeclaredNode = newTreeNode(FunDeclK);
      newUndeclaredNode->lineno = t->lineno;
      newUndeclaredNode->name = t->name;
      newUndeclaredNode->type = Undetermined;
      newUndeclaredNode->child[0] = newTreeNode(ParamK);
      newUndeclaredNode->child[0]->type = Undetermined;
      st_insert(t->name, t->lineno, addLocation(), newUndeclaredNode);
      undeclaredFunctionError(t);
    }
    else
      st_insert_lineno(t->name, t->lineno);
    break;

  case VarExpK:
    if (st_lookup(t->name) == -1)
    {
      TreeNode *newUndeclaredNode = newTreeNode(VarDeclK);
      newUndeclaredNode->lineno = t->lineno;
      newUndeclaredNode->name = t->name;
      newUndeclaredNode->type = Undetermined;
      st_insert(t->name, t->lineno, addLocation(), newUndeclaredNode);
      undeclaredVariableError(t);
    }
    else
      st_insert_lineno(t->name, t->lineno);
    break;

  default:
    break;
  }
}

/* Function buildSymtab constructs the symbol
 * table by preorder traversal of the syntax tree
 */
void buildSymtab(TreeNode *syntaxTree)
{
  globalScope = create_scope("global");
  push_scope(globalScope);
  push_built_in_functions();
  traverse(syntaxTree, insertNode, postProcInsertNode);
  pop_scope();

  if (TraceAnalyze)
  {
    printSymTab(listing);
    // printFunc(listing);
  }
}

static void typeError(TreeNode *t, char *message)
{
  fprintf(listing, "Type error at line %d: %s\n", t->lineno, message);
  Error = TRUE;
}

void preProcCheckNode(TreeNode *treeNode)
{
  if (treeNode->nodekind == CompStmtK)
    push_scope(treeNode->scope);

  else if (treeNode->nodekind == FunDeclK)
    curFuncName = treeNode->name;
}

/* Procedure checkNode performs
 * type checking at a single tree node
 */
static void checkNode(TreeNode *t)
{
  switch (t->nodekind)
  {
  case CompStmtK:
    pop_scope();
    break;
  case SelectStmtK:
  case IterStmtK:
    if (t->child[0]->type != Int)
      invalidConditionError(t);
    break;

  case RetStmtK:
  {
    TreeNode *funcNode = st_lookup_return_bucket(curFuncName)->treeNode;
    if (funcNode->type == Void && t->child[0] != NULL)
      invalidReturnError(t);
    else if (funcNode->type != Void && t->child[0] == NULL)
      invalidReturnError(t);
    else if (funcNode->type != Void && t->child[0]->type != funcNode->type)
      invalidReturnError(t);
    break;
  }

  case AssignK:
    if (t->child[0] == NULL || t->child[1] == NULL || t->child[0]->type == Undetermined || t->child[1]->type == Undetermined)
      invalidAssignmentError(t);
    else if (t->child[0]->type == Void || t->child[1]->type == Void)
      invalidAssignmentError(t);
    else if (t->child[0]->type != t->child[1]->type)
      invalidAssignmentError(t);
    else
      t->type = t->child[0]->type;
    break;

  case OpK:
    if (t->child[0] == NULL || t->child[1] == NULL)
      invalidOperationError(t);
    else if (t->child[0]->type != Int || t->child[1]->type != Int)
    {
      invalidOperationError(t);
      t->type = Undetermined;
    }
    else
      t->type = Int;

    break;

  case CallK:
  {
    BucketList l = st_lookup_return_bucket(t->name);
    TreeNode *funcNode = l->treeNode;
    TreeNode *param = funcNode->child[0];
    TreeNode *arg = t->child[0];
    int errorFlag = FALSE;

    if (param->type == Void)
    {
      if (arg != NULL)
        invalidFunctionCallError(t);
    }
    else
    {
      while (param != NULL && arg != NULL)
      {
        if (param->type != arg->type)
        {
          errorFlag = TRUE;
          invalidFunctionCallError(t);
          break;
        }
        param = param->sibling;
        arg = arg->sibling;
      }
      if ((param != NULL || arg != NULL) && !errorFlag)
      {
        invalidFunctionCallError(t);
      }
    }
    t->type = funcNode->type;
    break;
  }

  case VarExpK:
  {
    if (st_lookup(t->name) == -1)
      undeclaredVariableError(t);

    BucketList l = st_lookup_return_bucket(t->name);
    TreeNode *varNode = l->treeNode;

    if (varNode->type == Void || varNode->type == VoidArray)
      voidTypeError(t);

    if (t->child[0] != NULL)
    {
      if (t->child[0]->type != Int)
        invalidArrayIndexingIntError(t);
      if (varNode->type != IntArray)
        invalidArrayIndexingNotArrayError(t);
      t->type = Int;
    }
    else
      t->type = varNode->type;
    break;
  }

  case ConstK:
    t->type = Int;
    break;

  default:
    break;
  }
}

/* Procedure typeCheck performs type checking
 * by a postorder syntax tree traversal
 */
void typeCheck(TreeNode *syntaxTree)
{
  push_scope(globalScope);
  traverse(syntaxTree, preProcCheckNode, checkNode);
  pop_scope();
}
