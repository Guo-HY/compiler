#include "lexer.hpp"
#include <stdlib.h>

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
    } else if (!isBlank(ch)){
      return dealOtherTK(tokenInfo);
    }
  }
}

bool Lexer::dealIdent(TokenInfo* tokenInfo)
{
  tokenInfo->str.clear();
  if (!identifierNondigit(ch)) {
    /* program error */
    return false;
  }
  do {
    tokenInfo->str.push_back(ch);
    ch = fgetc(fp);
  } while (identifierNondigit(ch) || isDigit(ch));

  if (reserved.count(tokenInfo->str) > 0) {
    tokenInfo->tokenType = reserved.at(tokenInfo->str);
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
    return false;
  }

  do {
    tokenInfo->str.push_back(ch);
    ch = fgetc(fp);
  } while(isDigit(ch));

  if (tokenInfo->str.length() > 1 && tokenInfo->str[0] == '0') {
    /* error: IntConst has leading zeros*/
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
    return false;
  }
  do {
    formatChar = (ch == '%');
    backSlash = (ch == '\\');
    tokenInfo->str.push_back(ch);
    ch = fgetc(fp);
    if (formatChar && ch != 'd') {
      /* format error */
      return false;
    }
    if (backSlash && ch != 'n') {
      /* format error */
      return false;
    }
    formatChar = false;
    backSlash = false;
  } while(ch == 32 || ch == 33 || ch == 37 || (ch >= 40 && ch <= 126));

  if (ch != '"') {
    /* error : string is not terminate with " or string has illegal char*/
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
      return false;
    }
    tokenInfo->str.append(2, ch);
    ch = fgetc(fp);
    break;
  default:
    /* error : invalid identifier */
    return false;
  }
  tokenInfo->tokenType = reserved.at(tokenInfo->str);
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
  while (ch == ' ' || ch == '\t' || ch == '\n') {
    if (ch == '\n') {
      line++;
    }
    ch = fgetc(fp);
  }
}

bool Lexer::isBlank(char c)
{
  return c == ' ' || c == '\t' || c == '\n';
}

bool Lexer::dealSingleLineComment()
{
  if (ch != '/') { 
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
  return false;
}
