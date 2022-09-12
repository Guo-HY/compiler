#ifndef _LEXER_H
#define _LEXER_H 1

#include <string>
#include <unordered_map>

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

std::unordered_map<std::string, TokenType> reserved {
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

  private:

  char ch;
  FILE* fp;
  int line;
  
  bool dealIdent(TokenInfo* tokenInfo);
  bool dealIntConst(TokenInfo* tokenInfo);
  bool dealFormatString(TokenInfo* tokenInfo);
  bool dealOtherTK(TokenInfo* tokenInfo);

  bool Lexer::identifierNondigit(char c);
  bool Lexer::isDigit(char c);
  void Lexer::skipBlank();
  bool Lexer::isBlank(char c);
  bool Lexer::dealSingleLineComment();
  bool Lexer::dealMultiLineComment();
};

#endif