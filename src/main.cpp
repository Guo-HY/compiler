#include <stdio.h>
#include "frontend/lexer/lexer.hpp"
#include "frontend/parser/parser.hpp"
#include <error.hpp>

extern char tokenName[][20];

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

  Parser parser;
  parser.toString();

  // freopen("token.txt", "w", stdout);
  // int size = tokenInfoList.size();
  // for (int i = 0;i < size - 1; i++) {
  //   printf("%s %s\n", tokenName[tokenInfoList[i]->tokenType], tokenInfoList[i]->str.c_str());
  // }

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