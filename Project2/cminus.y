/****************************************************/
/* File: cminus.y                                     */
/* The C-MINUS Yacc/Bison specification file           */
/* Compiler Construction: Principles and Practice   */
/* Kenneth C. Louden                                */
/****************************************************/
%{
#define YYPARSER /* distinguishes Yacc output from other code files */

#include "globals.h"
#include "util.h"
#include "scan.h"
#include "parse.h"

#define YYSTYPE TreeNode *
static char * savedName; /* for use in assignments */
static int savedLineNo;  /* ditto */
static TreeNode * savedTree; /* stores syntax tree for later return */

static int yylex(void); /* lex와 error를 막기 위해 추가 */
%}

%token IF WHILE RETURN INT VOID
%nonassoc RPAREN
%nonassoc ELSE 
%token ID NUM EQ NE LT LE GT GE LPAREN LBRACE RBRACE LCURLY RCURLY SEMI COMMA ERROR 
%left PLUS MINUS 
%left TIMES OVER 
%right ASSIGN

%% /* Grammar for C-MINUS */

program : declaration_list { savedTree = $1; }
        ;  

declaration_list : declaration_list declaration 
                   { 
                    YYSTYPE t = $1;
                    if(t != NULL)
                    {
                      while(t->sibling != NULL)
                        t = t->sibling;
                      t->sibling = $2;
                      $$ = $1;
                    }
                    else
                      $$ = $2;
                   }
                 | declaration { $$ = $1; }
                 ;

declaration : var_declaration { $$ = $1; }
            | fun_declaration { $$ = $1; }                  
            ;

type_specifier : INT 
                 { 
                  $$ = newTreeNode(TypeK); 
                  $$->type = Int;
                 }
               | VOID 
                 {
                  $$ = newTreeNode(TypeK); 
                  $$->type = Void;
                 }
               ;

identifier : ID 
             {
              $$ = newTreeNode(VarExpK); 
              $$->lineno = lineno;
              $$->name = copyString(tokenString);
             }
           ;

number : NUM 
         {
          $$ = newTreeNode(ConstK);
          $$->lineno = lineno;
          $$->val = atoi(tokenString);
         }
       ;                                                 

var_declaration : type_specifier identifier SEMI
                  { 
                    $$ = newTreeNode(VarDeclK);
                    $$->type = $1->type;
                    $$->lineno = $2->lineno;                
                    $$->name = $2->name;                 
                  }
                 
                 | type_specifier identifier LBRACE number RBRACE SEMI
                   { 
                    $$ = newTreeNode(VarDeclK);
                    
                    if($1->type == Int)
                      $$->type = IntArray;
                    else if($1->type == Void)
                      $$->type = VoidArray;                         
                    
                    $$->lineno = $2->lineno;
                    $$->name = $2->name;
                    $$->child[0] = $4;
                   }
                 ;

fun_declaration : type_specifier identifier LPAREN params RPAREN compound_stmt
                  {
                    $$ = newTreeNode(FunDeclK);                    
                    $$->type = $1->type;
                    $$->lineno = $2->lineno;
                    $$->name = $2->name;
                    $$->child[0] = $4;
                    $$->child[1] = $6;
                  } 
                ;

params : param_list {$$ = $1;}
       | VOID
         {
          $$ = newTreeNode(ParamK);
          $$->lineno = lineno;
          $$->type = Void;
         }
       ;

param_list : param_list COMMA param
             {
              YYSTYPE t = $1;
              if(t != NULL)
              {
                while(t->sibling != NULL)
                  t = t->sibling;
                t->sibling = $3;
                $$ = $1;
              }
              else
                $$ = $3;
             }
            | param
              {$$ = $1;}
            ;

param : type_specifier identifier
        {
          $$ = newTreeNode(ParamK);
          $$->type = $1->type;
          $$->lineno = $2->lineno;
          $$->name = $2->name;
        }
      | type_specifier identifier LBRACE RBRACE
        {
          $$ = newTreeNode(ParamK);
          
          if($1->type == Int)
            $$->type = IntArray;
          else if($1->type == Void)
            $$->type = VoidArray;
          
          $$->lineno = $2->lineno;
          $$->name = $2->name;
        }                    
      ;

compound_stmt : LCURLY local_declarations statement_list RCURLY
                {
                  $$ = newTreeNode(CompStmtK);
                  $$->lineno = lineno;
                  $$->child[0] = $2;
                  $$->child[1] = $3;
                }
            ;

local_declarations : local_declarations var_declaration
                     {
                      YYSTYPE t = $1;
                      if(t != NULL)
                      {
                        while(t->sibling != NULL)
                          t = t->sibling;
                        t->sibling = $2;
                        $$ = $1;
                      }
                      else
                        $$ = $2;
                     }
                   | empty {$$ = NULL;}
                   ;

statement_list : statement_list statement
                 {
                  YYSTYPE t = $1;
                  if(t != NULL)
                  {
                    while(t->sibling != NULL)
                      t = t->sibling;
                    t->sibling = $2;
                    $$ = $1;
                  }
                  else
                    $$ = $2;
                 } 
               | empty {$$ = NULL;}
               ;

statement : expression_stmt {$$ = $1;}
          | compound_stmt {$$ = $1;}
          | selection_stmt {$$ = $1;}
          | iteration_stmt {$$ = $1;}
          | return_stmt {$$ = $1;}
          ;

expression_stmt : expression SEMI {$$ = $1;}
                | SEMI {$$ = NULL;}
                ;

selection_stmt : IF LPAREN expression RPAREN statement 
                 { 
                  $$ = newTreeNode(SelectStmtK);
                  $$->lineno = lineno; 
                  $$->child[0] = $3;
                  $$->child[1] = $5;
                 }
               | IF LPAREN expression RPAREN statement ELSE statement
                 { 
                  $$ = newTreeNode(SelectStmtK);
                  $$->lineno = lineno; 
                  $$->flag = TRUE;
                  $$->child[0] = $3;  
                  $$->child[1] = $5; 
                  $$->child[2] = $7;  
                 } 
               ;

iteration_stmt : WHILE LPAREN expression RPAREN statement
                 {
                  $$ = newTreeNode(IterStmtK);
                  $$->lineno = lineno;
                  $$->child[0] = $3;
                  $$->child[1] = $5;
                 }
                ;

return_stmt : RETURN SEMI 
              {
                $$ = newTreeNode(RetStmtK);
                $$->lineno = lineno;
                $$->flag = TRUE;
              }
            | RETURN expression SEMI
              {
                $$ = newTreeNode(RetStmtK);
                $$->lineno = lineno;
                $$->child[0] = $2;
              }

var : identifier
      {
        $$ = newTreeNode(VarExpK);
        $$->name = $1->name;
        $$->lineno = $1->lineno;
      }                  
    | identifier LBRACE expression RBRACE
      {
        $$ = newTreeNode(VarExpK);
        $$->name = $1->name;
        $$->lineno = $1->lineno;
        $$->child[0] = $3;
      }
    ;

expression : var ASSIGN expression
             {
              $$ = newTreeNode(AssignK);
              $$->lineno = lineno;
              $$->child[0] = $1;
              $$->child[1] = $3;
             }
           | simple_expression {$$ = $1;}
           ;

simple_expression : additive_expression relop additive_expression
                    {
                      $$ = newTreeNode(OpK);
                      $$->op = $2->op;
                      $$->lineno = lineno;
                      $$->child[0] = $1;
                      $$->child[1] = $3;
                    }
                  | additive_expression {$$ = $1;}
                  ;

relop : EQ 
        {
          $$ = newTreeNode(OpK); 
          $$->lineno = lineno;
          $$->op = EQ;
        }
      | NE
        {
          $$ = newTreeNode(OpK); 
          $$->lineno = lineno;
          $$->op = NE;
        }
      | LT
        {
          $$ = newTreeNode(OpK); 
          $$->lineno = lineno;
          $$->op = LT;
        }
      | LE
        {
          $$ = newTreeNode(OpK); 
          $$->lineno = lineno;
          $$->op = LE;
        }
      | GT
        {
          $$ = newTreeNode(OpK); 
          $$->lineno = lineno;
          $$->op = GT;
        }
      | GE
        {
          $$ = newTreeNode(OpK); 
          $$->lineno = lineno;
          $$->op = GE;
        }
      ;

additive_expression : additive_expression addop term
                      {
                        $$ = newTreeNode(OpK);
                        $$->lineno = lineno;
                        $$->op = $2->op;
                        $$->child[0] = $1;
                        $$->child[1] = $3;              
                      }
                    | term {$$ = $1;}
                    ;
addop : PLUS 
        {
          $$ = newTreeNode(OpK);
          $$->lineno = lineno;
          $$->op = PLUS;
        }
      | MINUS
        {
          $$ = newTreeNode(OpK);
          $$->lineno = lineno;
          $$->op = MINUS;
        }
      ;

term : term mulop factor
       {
        $$ = newTreeNode(OpK);
        $$->lineno = lineno;
        $$->op = $2->op;
        $$->child[0] = $1;
        $$->child[1] = $3;
       }
     | factor {$$ = $1;}
     ;

mulop : TIMES
        {
          $$ = newTreeNode(OpK);
          $$->lineno = lineno;
          $$->op = TIMES;
        }
      | OVER
        {
          $$ = newTreeNode(OpK);
          $$->lineno = lineno;
          $$->op = OVER;
        }
      ;

factor : LPAREN expression RPAREN {$$ = $2;}
       | var {$$ = $1;}
       | call {$$ = $1;}
       | number {$$ = $1;}
       ;

call : identifier LPAREN args RPAREN
       {
        $$ = newTreeNode(CallK);
        $$->name = $1->name;
        $$->lineno = $1->lineno;
        $$->child[0] = $3;
       }
      ;

args : arg_list {$$ = $1;}
     | empty {$$ = NULL;}
     ; 

arg_list : arg_list COMMA expression
           {
            YYSTYPE t = $1;
            if(t != NULL)
            {
              while(t->sibling != NULL)
                t = t->sibling;
              t->sibling = $3;
              $$ = $1;
            }
            else
              $$ = $3;
           }
         | expression {$$ = $1;}
         ;

empty : {$$ = NULL;}
      ;
    
%%

int yyerror(char * message)
{ fprintf(listing,"Syntax error at line %d: %s\n",lineno,message);
  fprintf(listing,"Current token: ");
  printToken(yychar,tokenString);
  Error = TRUE;
  return 0;
}

/* yylex calls getToken to make Yacc/Bison output
 * compatible with ealier versions of the C-MINUS scanner
 */
static int yylex(void)
{ return getToken(); }

TreeNode * parse(void)
{ yyparse();
  return savedTree;
}

