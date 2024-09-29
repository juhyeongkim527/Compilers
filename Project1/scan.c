/****************************************************/
/* File: scan.c                                     */
/* The scanner implementation for the TINY compiler */
/* Compiler Construction: Principles and Practice   */
/* Kenneth C. Louden                                */
/****************************************************/

#include "globals.h"
#include "util.h"
#include "scan.h"

/* states in scanner DFA */
typedef enum
{
  START,
  INNUM,
  INID,
  DONE,
  // INASSIGN // INASSIGN은 Tiny가 아닌 C-Minus에는 Assign token으로 사용하지 않음

  // 추가한 부분
  INEQ,
  INLT,
  INGT,
  INNE,
  INOVER,    // '/'를 입력 받은 후 '*'를 기다리며 OVER인지 COMMENT인지 검증하는 State
  INCOMMENT, // '/*' 까지 입력 받고 COMMENT 안에 들어와서 '*'를 기다리는 State
  INCOMMENT_ // '/* ... *' 까지 입력 받고 '/'를 기다리는 State
} StateType;

/* lexeme of identifier or reserved word */
char tokenString[MAXTOKENLEN + 1];

/* BUFLEN = length of the input buffer for
   source code lines */
#define BUFLEN 256

static char lineBuf[BUFLEN]; /* holds the current line */
static int linepos = 0;      /* current position in LineBuf */
static int bufsize = 0;      /* current size of buffer string */
static int EOF_flag = FALSE; /* corrects ungetNextChar behavior on EOF */

/* getNextChar fetches the next non-blank character
   from lineBuf, reading in a new line if lineBuf is
   exhausted */
static int getNextChar(void)
{
  if (!(linepos < bufsize))
  {
    lineno++;
    if (fgets(lineBuf, BUFLEN - 1, source))
    {
      if (EchoSource)
        fprintf(listing, "%4d: %s", lineno, lineBuf);
      bufsize = strlen(lineBuf);
      linepos = 0;
      return lineBuf[linepos++];
    }
    else
    {
      EOF_flag = TRUE;
      return EOF;
    }
  }
  else
    return lineBuf[linepos++];
}

/* ungetNextChar backtracks one character
   in lineBuf */
static void ungetNextChar(void)
{
  if (!EOF_flag)
    linepos--;
}

/* lookup table of reserved words */
// reserved words(keywords) 수정
static struct
{
  char *str;
  TokenType tok;
} reservedWords[MAXRESERVED] = {{"if", IF}, {"else", ELSE}, {"while", WHILE}, {"return", RETURN}, {"int", INT}, {"void", VOID}};

/* lookup an identifier to see if it is a reserved word */
/* uses linear search */
static TokenType reservedLookup(char *s)
{
  int i;
  for (i = 0; i < MAXRESERVED; i++)
    if (!strcmp(s, reservedWords[i].str))
      return reservedWords[i].tok;
  return ID;
}

/****************************************/
/* the primary function of the scanner  */
/****************************************/
/* function getToken returns the
 * next token in source file
 */
TokenType getToken(void)
{ /* index for storing into tokenString */
  int tokenStringIndex = 0;
  /* holds current token to be returned */
  TokenType currentToken;
  /* current state - always begins at START */
  StateType state = START;
  /* flag to indicate save to tokenString */
  int save;
  while (state != DONE)
  {
    int c = getNextChar();
    save = TRUE;

    switch (state)
    {
    case START:
      if (isdigit(c))
        state = INNUM;

      else if (isalpha(c))
        state = INID;

      else if ((c == ' ') || (c == '\t') || (c == '\n'))
        save = FALSE;

      else if (c == '=')
      {
        save = FALSE;
        state = INEQ;
      }

      else if (c == '<')
      {
        save = FALSE;
        state = INLT;
      }

      else if (c == '>')
      {
        save = FALSE;
        state = INGT;
      }

      else if (c == '!')
      {
        // save = FALSE;
        // '!' 뒤에 '='가 나오지 않는 경우 ERROR로 처리해야 하기 때문에, save = FALSE;를 해주면 안됨
        state = INNE;
      }
      else if (c == '/')
      {
        save = FALSE;
        state = INOVER;
      }

      else
      {
        state = DONE;
        switch (c)
        {
        case EOF:
          save = FALSE;
          currentToken = ENDFILE;
          break;
        case '+':
          currentToken = PLUS;
          break;
        case '-':
          currentToken = MINUS;
          break;
        case '*':
          currentToken = TIMES;
          break;
        case '(':
          currentToken = LPAREN;
          break;
        case ')':
          currentToken = RPAREN;
          break;
        case '[':
          currentToken = LBRACE;
          break;
        case ']':
          currentToken = RBRACE;
          break;
        case '{':
          currentToken = LCURLY;
          break;
        case '}':
          currentToken = RCURLY;
          break;
        case ';':
          currentToken = SEMI;
          break;
        case ',':
          currentToken = COMMA;
          break;
        default:
          currentToken = ERROR;
          break;
        }
      }
      break; // end case START

    case INEQ:
      // save = FALSE;는 딱히 해줄 필요 없긴 할듯
      // 어쩌피 ID, NUM, reserved words이외에는 tokenString 자체를 출력하지 않기 때문
      save = FALSE;
      if (c == '=')
      {
        state = DONE;
        currentToken = EQ;
      }
      else
      {
        ungetNextChar();
        state = DONE;
        currentToken = ASSIGN;
      }
      break;

    case INLT:
      save = FALSE;
      if (c == '=')
      {
        state = DONE;
        currentToken = LE;
      }
      else
      {
        ungetNextChar();
        state = DONE;
        currentToken = LT;
      }
      break;

    case INGT:
      save = FALSE;
      if (c == '=')
      {

        state = DONE;
        currentToken = GE;
      }
      else
      {
        ungetNextChar();
        state = DONE;
        currentToken = GT;
      }
      break;

    case INNE:
      save = FALSE; // '='가 오지 않았을 때, ERROR에 c가 들어가지 않게 하기 위해 FALSE로 설정해줘야함
      if (c == '=')
      {
        state = DONE;
        currentToken = NE;
      }
      else
      {
        ungetNextChar();
        state = DONE;
        currentToken = ERROR;
      }
      break;

    case INOVER: // '/'를 입력 받은 후 '*'를 기다리며 OVER인지 COMMENT인지 검증하는 State
      save = FALSE;
      if (c == '*') // 주석이 시작되면 INCOMMENT_ 상태로 이동
        state = INCOMMENT;
      else if (c == EOF) // 주석 중에 파일이 끝나면 ERROR가 아닌 ENDFILE로 끝냄
      {
        state = DONE;
        currentToken = ENDFILE;
      }
      else // 주석이 시작되지 않았다면 OVER로 인식
      {
        ungetNextChar();
        state = DONE;
        currentToken = OVER;
      }
      break;

    case INCOMMENT: // '/*' 까지 입력 받고 COMMENT 안에 들어와서 '*'를 기다리는 State
      save = FALSE;
      if (c == '*') // 주석을 닫는 '*'가 입력된 경우 INCOMMENT__로 이동
        state = INCOMMENT_;
      else if (c == EOF) // 주석 중에 파일이 끝나면 ERROR가 아닌 ENDFILE로 끝냄
      {
        state = DONE;
        currentToken = ENDFILE;
      }
      break;

    case INCOMMENT_: // '/* ... *' 까지 입력 받고 '/'를 기다리는 State
      save = FALSE;
      if (c == '/') // 주석을 끝내고 다시 START 상태로 돌아가기
        state = START;
      else if (c == EOF)
      {
        state = DONE;
        currentToken = ENDFILE;
      }
      else // '*' 뒤에 바로 '/'가 오지 않으면 다시 INCOMMENT_ 상태로 돌아가서 '*'를 받아야 함
        state = INCOMMENT;
      break;

    case INNUM:
      if (!isdigit(c))
      { /* backup in the input */
        ungetNextChar();
        save = FALSE;
        state = DONE;
        currentToken = NUM;
      }
      break;

    case INID:
      if (!isalpha(c) && !isdigit(c))
      { /* backup in the input */
        ungetNextChar();
        save = FALSE;
        state = DONE;
        currentToken = ID;
      }
      break;

    case DONE:
    default: /* should never happen */
      fprintf(listing, "Scanner Bug: state= %d\n", state);
      state = DONE;
      currentToken = ERROR;
      break;
    } // end switch

    if ((save) && (tokenStringIndex <= MAXTOKENLEN))
      tokenString[tokenStringIndex++] = (char)c;

    if (state == DONE)
    {
      tokenString[tokenStringIndex] = '\0';
      if (currentToken == ID)
        currentToken = reservedLookup(tokenString); // ID가 reserved words였는지 확인
    }
  } // end while

  if (TraceScan) // main.c에서 설정한 TraceScan이 TRUE일 때(Scanner)만 출력
  {
    fprintf(listing, "\t%d: ", lineno);
    printToken(currentToken, tokenString); // currentToken에 따라 listing(stdout)에 tokenString 출력
  }
  return currentToken; // main.c에서 currentToken이 ENDFILE이면 종료
} /* end getToken */
