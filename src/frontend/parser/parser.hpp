#ifndef _PARSER_H
#define _PARSER_H 1

#include "syntax_tree.hpp"
#include "symbol_table.hpp"

extern std::vector<TokenInfo*> tokenInfoList;

class Parser {
  public:
  Parser() {
    this->nowTokenListPtr = 0;
    this->root = NULL;
  }

  SyntaxNode* syntaxAnalyse();
  void toString();

  private:
  /* 根节点 */
  CompUnitNode* root;
  /* 当前关注的token指针，处理完一个语法元素后，指针永远指向下一个 */
  int nowTokenListPtr;
  /* pop 会返回当前token的指针并将nowTokenListPtr加一 */
  TokenInfo* popToken();
  /* peek 会返回当前位置+num处的token，但不会改变nowTokenListPtr */
  TokenInfo* peekToken(int num);
  /* 从当前位置向后找，判断先遇到分号还是赋值等号，若先是等号则返回true  */
  bool isAssign();

  CompUnitNode* compUnitAnalyse();
  DeclNode* declAnalyse();
  ConstDeclNode* constDeclAnalyse();
  BTypeNode* bTypeAnalyse();
  ConstDefNode* constDefAnalyse(BTypeNode* bType);
  ConstInitValNode* constInitValAnalyse();
  VarDeclNode* varDeclAnalyse();
  VarDefNode* varDefAnalyse(BTypeNode* bType);
  InitValNode* initValAnalyse();
  FuncDefNode* funcDefAnalyse();
  MainFuncDefNode* mainFuncDefAnalyse();
  FuncTypeNode* funcTypeAnalyse();
  FuncFParamsNode* funcFParamsAnalyse();
  FuncFParamNode* FuncFParamAnalyse();
  /* newSymbolTable 为真表示需要新建符号表 */
  BlockNode* blockAnalyse(bool newSymbolTable);
  BlockItemNode* blockItemAnalyse();
  StmtNode* stmtAnalyse();
  ExpNode* expAnalyse(LValNode*);
  CondNode* condAnalyse();
  LValNode* lValAnalyse();
  PrimaryExpNode* primaryExpAnalyse(LValNode*);
  NumberNode* numberAnalyse();
  UnaryExpNode* unaryExpAnalyse(LValNode*);
  UnaryOpNode* unaryOpAnalyse();
  FuncRParamsNode* funcRParamsAnalyse();
  MulExpNode* mulExpAnalyse(LValNode*);
  AddExpNode* addExpAnalyse(LValNode*);
  RelExpNode* relExpAnalyse();
  EqExpNode* eqExpAnalyse();
  LAndExpNode* lAndExpAnalyse();
  LOrExpNode* lOrExpAnalyse();
  ConstExpNode* constExpAnalyse();

};

#endif