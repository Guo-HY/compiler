#include <stdio.h>
#include "frontend/lexer/lexer.hpp"

char tokenName[][20] = {
  "IDENFR",
  "INTCON",
  "STRCON",
  "MAINTK",
  "CONSTTK",
  "INTTK",
  "BREAKTK",
  "CONTINUETK",
  "IFTK",
  "ELSETK",
  "NOT",
  "AND",
  "OR",
  "WHILETK",
  "GETINTTK",
  "PRINTFTK",
  "RETURNTK",
  "PLUS",
  "MINU",
  "VOIDTK",
  "MULT",
  "DIV",
  "MOD",
  "LSS",
  "LEQ",
  "GRE",
  "GEQ",
  "EQL",
  "NEQ",
  "ASSIGN",
  "SEMICN",
  "COMMA",
  "LPARENT",
  "RPARENT",
  "LBRACK",
  "RBRACK",
  "LBRACE", 
  "RBRACE",
  "END",
};


int main(int argc, char **argv)
{
  FILE *fp = fopen(argv[1], "r");
  if (fp == NULL) {
    printf("error : fp == NULL\n");
    exit(1);
  }
  Lexer lexer(fp);
  bool ret;
  TokenInfo tokenInfo;

  while (true) {
    ret = lexer.getToken(&tokenInfo);
    if (ret == false) {
      printf("lexer has something error\n");
      exit(1);
    }
    if (tokenInfo.tokenType == END) {
      break;
    }
    printf("%s %s\n", tokenName[tokenInfo.tokenType], tokenInfo.str.c_str());
  }
  return 0;
}