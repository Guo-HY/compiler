#ifndef _LEXER_H
#define _LEXER_H 1

#include <string>
#include <unordered_map>
#include <vector>

enum TokenType {
  IDENFR,
  INTCON,
  STRCON,
  MAINTK,
  CONSTTK,
  INTTK,
  BREAKTK,
  CONTINUETK,
  IFTK,
  ELSETK,
  NOT,
  AND,
  OR,
  WHILETK,
  GETINTTK,
  PRINTFTK,
  RETURNTK,
  PLUS,
  MINU,
  VOIDTK,
  MULT,
  DIV,
  MOD,
  LSS,
  LEQ,
  GRE,
  GEQ,
  EQL,
  NEQ,
  ASSIGN,
  SEMICN,
  COMMA,
  LPARENT,
  RPARENT,
  LBRACK,
  RBRACK,
  LBRACE, 
  RBRACE,
  END,
};
/* TODO : 改成类 */
struct TokenInfo {
  TokenType tokenType;
  std::string str;
  int value;
};

class Lexer {
  public:

  Lexer(FILE *fp) {
    if (fp == NULL) {
      /* TODO : error handle*/
      this->ch = EOF;
    }
    this->fp = fp;
    this->ch = fgetc(fp);
    this->line = 1;
  }

  bool getToken(TokenInfo* tokenInfo);
  bool getAllToken();
  private:

  char ch;
  FILE* fp;
  int line;
  
  bool dealIdent(TokenInfo* tokenInfo);
  bool dealIntConst(TokenInfo* tokenInfo);
  bool dealFormatString(TokenInfo* tokenInfo);
  bool dealOtherTK(TokenInfo* tokenInfo);

  bool identifierNondigit(char c);
  bool isDigit(char c);
  void skipBlank();
  bool isBlank(char c);
  bool dealSingleLineComment();
  bool dealMultiLineComment();
};

#endif