#include <stdio.h>
#include "frontend/lexer/lexer.hpp"
#include "frontend/parser/parser.hpp"
#include <error.hpp>

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

void getTokenTest(Lexer* lexer);
extern std::vector<TokenInfo*> tokenInfoList;

int main(int argc, char **argv)
{
  FILE *fp = fopen("testfile.txt", "r");
  freopen("output.txt", "w", stdout);
  if (fp == NULL) {
    panic("can't open source file");
  }

  Lexer lexer(fp);
  bool ret;
  // getTokenTest(&lexer);

  ret = lexer.getAllToken();

  if (ret == false) {
    Log("lexer has something error\n");
    exit(1);
  }
  // int size = tokenInfoList.size();
  // for (int i = 0;i < size - 1; i++) {
  //   printf("%s %s\n", tokenName[tokenInfoList[i]->tokenType], tokenInfoList[i]->str.c_str());
  // }

  Parser parser;
  parser.toString();

  return 0;
}

/**
 * @brief 通过调用getToken识别所有的词法元素
 * 
 */
void getTokenTest(Lexer* lexer)
{
  bool ret;
  TokenInfo tokenInfo;

  while (true) {
    ret = lexer->getToken(&tokenInfo);
    if (ret == false) {
      Log("lexer has something error\n");
      exit(1);
    }
    if (tokenInfo.tokenType == END) {
      break;
    }
    printf("%s %s\n", tokenName[tokenInfo.tokenType], tokenInfo.str.c_str());
  }
}