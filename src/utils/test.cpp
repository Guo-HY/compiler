#include "test.hpp"

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