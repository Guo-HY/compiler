#ifndef _TEST_H
#define _TEST_H 1
#include "../frontend/lexer/lexer.hpp"
#include "../include/error.hpp"
extern char tokenName[][20];
extern std::vector<TokenInfo*> tokenInfoList;

void getTokenTest(Lexer* lexer);
void getAllTokenTest();

#endif