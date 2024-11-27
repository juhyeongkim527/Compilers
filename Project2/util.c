/****************************************************/
/* File: util.c                                     */
/* Utility function implementation                  */
/* for the C-MINUS compiler                            */
/* Compiler Construction: Principles and Practice   */
/* Kenneth C. Louden                                */
/****************************************************/

#include "globals.h"
#include "util.h"

/* Procedure printToken prints a token
 * and its lexeme to the listing file
 */
void printToken(TokenType token, const char *tokenString)
{
  switch (token)
  {
  /* reserved words */
  case IF:
  case ELSE:
  case WHILE:
  case RETURN:
  case INT:
  case VOID:
    fprintf(listing, "reserved word: %s\n", tokenString);
    break;

  /* multicharacter tokens */
  case NUM:
    fprintf(listing, "NUM, val= %s\n", tokenString);
    break;
  case ID:
    fprintf(listing, "ID, name= %s\n", tokenString);
    break;

  /* special symbols */
  case ASSIGN:
    fprintf(listing, "=\n");
    break;
  case EQ:
    fprintf(listing, "==\n");
    break;
  case NE:
    fprintf(listing, "!=\n");
    break;
  case LT:
    fprintf(listing, "<\n");
    break;
  case LE:
    fprintf(listing, "<=\n");
    break;
  case GT:
    fprintf(listing, ">\n");
    break;
  case GE:
    fprintf(listing, ">=\n");
    break;
  case PLUS:
    fprintf(listing, "+\n");
    break;
  case MINUS:
    fprintf(listing, "-\n");
    break;
  case TIMES:
    fprintf(listing, "*\n");
    break;
  case OVER:
    fprintf(listing, "/\n");
    break;
  case LPAREN:
    fprintf(listing, "(\n");
    break;
  case RPAREN:
    fprintf(listing, ")\n");
    break;
  case LBRACE:
    fprintf(listing, "[\n");
    break;
  case RBRACE:
    fprintf(listing, "]\n");
    break;
  case LCURLY:
    fprintf(listing, "{\n");
    break;
  case RCURLY:
    fprintf(listing, "}\n");
    break;
  case SEMI:
    fprintf(listing, ";\n");
    break;
  case COMMA:
    fprintf(listing, ",\n");
    break;

  case ENDFILE:
    fprintf(listing, "EOF\n");
    break;
  case ERROR:
    fprintf(listing, "ERROR: %s\n", tokenString);
    break;

  default: /* should never happen */
    fprintf(listing, "Unknown token: %d\n", token);
  }
}

/* Function newTreeNode creates a new tree node
 * for syntax tree construction
 * combine newStmtNode, newExpNode
 */
TreeNode *newTreeNode(NodeKind nodekind)
{
  TreeNode *t = (TreeNode *)malloc(sizeof(TreeNode));
  int i;
  if (t == NULL)
    fprintf(listing, "Out of memory error at line %d\n", lineno);
  else
  {
    for (i = 0; i < MAXCHILDREN; i++)
      t->child[i] = NULL;
    t->sibling = NULL;
    t->nodekind = nodekind; // parameter로 받은 nodeType에 따라 구분
    t->lineno = lineno;
    t->flag = FALSE;
  }
  return t;
}

/* Function copyString allocates and makes a new
 * copy of an existing string
 */
char *copyString(char *s)
{
  int n;
  char *t;
  if (s == NULL)
    return NULL;
  n = strlen(s) + 1;
  t = malloc(n);
  if (t == NULL)
    fprintf(listing, "Out of memory error at line %d\n", lineno);
  else
    strcpy(t, s);
  return t;
}

/* Variable indentno is used by printTree to
 * store current number of spaces to indent
 */
static indentno = 0;

/* macros to increase/decrease indentation */
#define INDENT indentno += 2
#define UNINDENT indentno -= 2

/* printSpaces indents by printing spaces */
static void printSpaces(void)
{
  int i;
  for (i = 0; i < indentno; i++)
    fprintf(listing, " ");
}

char *find_type(NodeType type)
{
  if (type == Int)
    return "int";
  if (type == Void)
    return "void";
  if (type == IntArray)
    return "int[]";
  if (type == VoidArray)
    return "void[]";
  else
    return "<Type Error>";
}

/* procedure printTree prints a syntax tree to the
 * listing file using indentation to indicate subtrees
 */
void printTree(TreeNode *tree)
{
  int i;
  INDENT;
  while (tree != NULL)
  {
    printSpaces();
    switch (tree->nodekind)
    {
    case VarDeclK:
      fprintf(listing, "Variable Declaration: name = %s, type = %s\n", tree->name, find_type(tree->type));
      break;

    case FunDeclK:
      fprintf(listing, "Function Declaration: name = %s, return type = %s\n", tree->name, find_type(tree->type));
      break;

    case ParamK:
      if (tree->type == Void)
        fprintf(listing, "Void Parameter\n");
      else
        fprintf(listing, "Parameter: name = %s, type = %s\n", tree->name, find_type(tree->type));
      break;

    case CompStmtK:
      fprintf(listing, "Compound Statement:\n");
      break;

    case SelectStmtK:
      if (tree->flag)
        fprintf(listing, "If-Else Statement:\n");
      else
        fprintf(listing, "If Statement:\n");
      break;

    case IterStmtK:
      fprintf(listing, "While Statement:\n");
      break;

    case RetStmtK:
      if (tree->flag)
        fprintf(listing, "Non-value Return Statement\n");
      else
        fprintf(listing, "Return Statement:\n");
      break;

    case OpK:
      fprintf(listing, "Op: ");
      printToken(tree->op, "\0");
      break;

    case AssignK:
      fprintf(listing, "Assign:\n");
      break;

    case VarExpK:
      fprintf(listing, "Variable: name = %s\n", tree->name);
      break;

    case ConstK:
      fprintf(listing, "Const: %d\n", tree->val);
      break;

    case CallK:
      fprintf(listing, "Call: function name = %s\n", tree->name);
      break;

    default:
      fprintf(listing, "Unknown Node kind\n");
      break;
    }
    for (i = 0; i < MAXCHILDREN; i++)
      printTree(tree->child[i]);
    tree = tree->sibling;
  }
  UNINDENT;
}
