#include <stdio.h>

#include "frontend/lexer/lexer.hpp"
#include "frontend/parser/parser.hpp"
#include "frontend/parser/symbol_table.hpp"
#include "utils/error_handle.hpp"
#include <error.hpp>

extern char tokenName[][20];
extern std::vector<TokenInfo*> tokenInfoList;
extern SymbolTable* currentSymbolTable;
extern ErrorList errorList;

void getTokenTest(Lexer* lexer);
void getAllTokenTest(Lexer* lexer);
extern void symbolTableInit();
extern void allSymbolTableToString();

int main(int argc, char **argv)
{
  FILE* fp = fopen("testfile.c", "r");
  freopen("output.txt", "w", stdout);
  bool ret;
  if (fp == NULL) {
    panic("can't open source file");
  }

  Lexer*lexer = new Lexer(fp);
  ret = lexer->getAllToken();
  if (ret == false) {
    Log("lexer has something error\n");
    exit(1);
  }

  Parser* parser = new Parser();
  parser->toString();
  Log("after parser");
  freopen("table.txt", "w", stdout);
  currentSymbolTable->toString(0);

  freopen("error.txt", "w", stdout);
  errorList.toString();

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
    printf("%s\t%s\n", tokenName[tokenInfo.tokenType], tokenInfo.str.c_str());
  }
}

/**
 * @brief 通过调用getAllToken输出所有的词法元素以及行号
 */
void getAllTokenTest(Lexer* lexer)
{

  int size = tokenInfoList.size();
  for (int i = 0;i < size - 1; i++) {
    printf("tokenName=%s\ttokenValue=%s\ttokenLine=%d\n", tokenName[tokenInfoList[i]->tokenType], 
    tokenInfoList[i]->str.c_str(), tokenInfoList[i]->line);
  }
}