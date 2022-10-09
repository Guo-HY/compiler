#ifndef _PARSER_H
#define _PARSER_H 1

#include "syntax_tree.hpp"
#include "symbol_table.hpp"
#include "../../utils/error_handle.hpp"
extern std::vector<TokenInfo*> tokenInfoList;

class Parser {
  public:
  Parser(ErrorList* list) {
    this->nowTokenListPtr = 0;
    this->root = NULL;
    this->errorList = list;
  }

  SyntaxNode* syntaxAnalyse();
  void toString();

  private:
  /* 根节点 */
  CompUnitNode* root;
  /* 当前关注的token指针，处理完一个语法元素后，指针永远指向下一个 */
  int nowTokenListPtr;
  /* 错误列表指针 */
  ErrorList* errorList;
  /* pop 会返回当前token的指针并将nowTokenListPtr加一 */
  TokenInfo* popToken();
    /* peek 会返回当前位置+num处的token，但不会改变nowTokenListPtr */
  TokenInfo* peekToken(int num);

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

  /* 检查类型为tokenType的符号是否缺失，
    如果缺失就将错误信息加入errorList，否则popToken */
  bool tokenLackHandler(TokenType tokenType);

  bool Parser::isExpFirst();

};

#endif