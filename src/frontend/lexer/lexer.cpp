#include "lexer.hpp"
#include <stdlib.h>
#include "../include/error.hpp"

std::unordered_map<std::string, TokenType> reservedTable {
  {"main",    MAINTK},
  {"const",   CONSTTK},
  {"int",     INTTK},
  {"break",   BREAKTK},
  {"continue",CONTINUETK},
  {"if",      IFTK},
  {"else",    ELSETK},
  {"while",   WHILETK},
  {"getint",  GETINTTK},
  {"printf",  PRINTFTK},
  {"return",  RETURNTK},
  {"void",    VOIDTK},
  {"!",       NOT},
  {"&&",      AND},
  {"||",      OR},
  {"+",       PLUS}, 
  {"-",       MINU},
  {"*",       MULT}, 
  {"/",       DIV},
  {"%",       MOD},
  {"<",       LSS},
  {"<=",      LEQ},
  {">",       GRE},
  {">=",      GEQ},
  {"==",      EQL},
  {"!=",      NEQ},
  {"=",       ASSIGN},
  {";",       SEMICN},
  {",",       COMMA},
  {"(",       LPARENT},
  {")",       RPARENT},
  {"[",       LBRACK},
  {"]",       RBRACK},
  {"{",       LBRACE},
  {"}",       RBRACE},
};

bool Lexer::getToken(TokenInfo* tokenInfo) 
{
  while (true) {
    skipBlank();
    if (ch == EOF) {
      /* end of file*/
      tokenInfo->tokenType = END;
      return true;
    }

    /* deal comment */
    if (ch == '/') {
      ch = fgetc(fp);
      if (ch == '/') {
        dealSingleLineComment();
      } else if (ch == '*') {
        if (!dealMultiLineComment()) {
          return false;
        }
      } else {
        ch = '/';
        fseek(fp, -1L, 1);
      }
    }

    if (identifierNondigit(ch)) {
      return dealIdent(tokenInfo);
    } else if (isDigit(ch)) {
      return dealIntConst(tokenInfo);
    } else if (ch == '"') {
      return dealFormatString(tokenInfo);
    } else if (!isBlank(ch) && ch != EOF){
      return dealOtherTK(tokenInfo);
    }
  }
}

bool Lexer::dealIdent(TokenInfo* tokenInfo)
{
  tokenInfo->str.clear();
  if (!identifierNondigit(ch)) {
    /* program error */
    Log("program error\n");
    return false;
  }
  do {
    tokenInfo->str.push_back(ch);
    ch = fgetc(fp);
  } while (identifierNondigit(ch) || isDigit(ch));

  if (reservedTable.count(tokenInfo->str) > 0) {
    tokenInfo->tokenType = reservedTable.at(tokenInfo->str);
  } else {
    tokenInfo->tokenType = IDENFR;
  }
  return true;
}

bool Lexer::dealIntConst(TokenInfo* tokenInfo) 
{
  tokenInfo->str.clear();
  if (!isDigit(ch)) {
    /* program error */
    Log("program error\n");
    return false;
  }

  do {
    tokenInfo->str.push_back(ch);
    ch = fgetc(fp);
  } while(isDigit(ch));

  if (tokenInfo->str.length() > 1 && tokenInfo->str[0] == '0') {
    /* error: IntConst has leading zeros*/
    Log("IntConst has leading zeros\n");
    return false;
  }
  tokenInfo->tokenType = INTCON;
  tokenInfo->value = atoi(tokenInfo->str.c_str());
  return true;
}

bool Lexer::dealFormatString(TokenInfo* tokenInfo)
{
  bool formatChar = false;
  bool backSlash = false;

  tokenInfo->str.clear();
  if (ch != '"') {
    /* program error*/
    Log("program error\n");
    return false;
  }
  do {
    formatChar = (ch == '%');
    backSlash = (ch == '\\');
    tokenInfo->str.push_back(ch);
    ch = fgetc(fp);
    if (formatChar && ch != 'd') {
      /* format error */
      Log("format error : %% doesn't follow with d\n");
      return false;
    }
    if (backSlash && ch != 'n') {
      /* format error */
      Log("format error : \\ doesn't follow with n\n");
      return false;
    }
    formatChar = false;
    backSlash = false;
  } while(ch == 32 || ch == 33 || ch == 37 || (ch >= 40 && ch <= 126));

  if (ch != '"') {
    /* error : string is not terminate with " or string has illegal char*/
    Log("format string is not terminate with \" or string has illegal char\n");
    return false;
  }
  tokenInfo->str.push_back(ch);
  ch = fgetc(fp);
  tokenInfo->tokenType = STRCON;
  return true;
}

bool Lexer::dealOtherTK(TokenInfo* tokenInfo)
{
  char tmp;
  tokenInfo->str.clear();
  switch (ch) {
  case '+':
  case '-':
  case '*':
  case '%':
  case ';':
  case ',':
  case '(':
  case ')':
  case '[':
  case ']':
  case '{':
  case '}':
  case '/':
    tokenInfo->str.push_back(ch);
    ch = fgetc(fp);
    break;
  case '<':
  case '>':
  case '=':
  case '!':
    tokenInfo->str.push_back(ch);
    ch = fgetc(fp);
    if (ch == '=') {
      tokenInfo->str.push_back(ch);
      ch = fgetc(fp);
    }
    break;
  case '&':
  case '|':
    tmp = fgetc(fp);
    if (ch != tmp) {
      /* error : need double but has single char */
      Log("need double but has single char(& or |)\n");
      return false;
    }
    tokenInfo->str.append(2, ch);
    ch = fgetc(fp);
    break;
  default:
    /* error : invalid identifier */
    Log("invalid identifier : %c\n", ch);
    return false;
  }
  tokenInfo->tokenType = reservedTable.at(tokenInfo->str);
  return true;
}

bool Lexer::identifierNondigit(char c)
{
  return c == '_' || (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

bool Lexer::isDigit(char c) 
{
  return (c >= '0' && c <= '9');
}

void Lexer::skipBlank() 
{
  while (ch == ' ' || ch == '\t' || ch == '\n' || ch == '\r') {
    if (ch == '\n') {
      line++;
    }
    ch = fgetc(fp);
  }
}

bool Lexer::isBlank(char c)
{
  return c == ' ' || c == '\t' || c == '\n' || c == '\r';
}

bool Lexer::dealSingleLineComment()
{
  if (ch != '/') { 
    Log("program error\n");
    return false; 
  }
  while (ch != '\n' && ch != EOF) {
    ch = fgetc(fp);
  }
  return true;
}

bool Lexer::dealMultiLineComment()
{
  bool isAsterisk = false;
  if (ch != '*') {
    Log("program error\n");
    return false;
  }
  do {
    ch = fgetc(fp);
    if (isAsterisk && ch == '/') {
      ch = fgetc(fp);
      return true;
    }
    isAsterisk = false;
    if (ch == '*') {
      isAsterisk = true;
    }
    if (ch == '\n') {
      line++;
    }
  } while (ch != EOF);
  /* error : multi-line Comment dosen't have \*\/ */
  Log("multi-line Comment dosen't have */\n");
  return false;
}
