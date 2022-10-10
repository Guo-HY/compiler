#ifndef _PARSER_H
#define _PARSER_H 1

#include "syntax_tree.hpp"
#include "symbol_table.hpp"
#include "../../utils/error_handle.hpp"
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
  BlockNode* blockAnalyse(bool newSymbolTable, std::string* funcName, bool needRE);
  BlockItemNode* blockItemAnalyse(std::string* funcName, bool* hasRE);
  StmtNode* stmtAnalyse(std::string* funcName, bool* hasRE);
  ExpNode* expAnalyse(LValNode* lval, bool addToNFCP, std::vector<ObjectSymbolItem*>* funcCParams);
  CondNode* condAnalyse();
  LValNode* lValAnalyse();
  PrimaryExpNode* primaryExpAnalyse(LValNode* lval, bool addToNFCP, std::vector<ObjectSymbolItem*>* funcCParams);
  NumberNode* numberAnalyse();
  UnaryExpNode* unaryExpAnalyse(LValNode* lval, bool addToNFCP, std::vector<ObjectSymbolItem*>* funcCParams);
  UnaryOpNode* unaryOpAnalyse();
  FuncRParamsNode* funcRParamsAnalyse(std::vector<ObjectSymbolItem*>* funcCParams);
  MulExpNode* mulExpAnalyse(LValNode* lval, bool addToNFCP, std::vector<ObjectSymbolItem*>* funcCParams);
  AddExpNode* addExpAnalyse(LValNode* lval, bool addToNFCP, std::vector<ObjectSymbolItem*>* funcCParams);
  RelExpNode* relExpAnalyse();
  EqExpNode* eqExpAnalyse();
  LAndExpNode* lAndExpAnalyse();
  LOrExpNode* lOrExpAnalyse();
  ConstExpNode* constExpAnalyse();

  /* 检查类型为tokenType的符号是否缺失，
    如果缺失就将错误信息加入errorList，否则popToken */
  bool tokenLackHandler(TokenType tokenType);
  /* 检查格式字符串是否含有非法字符，参数个数是否匹配 */
  bool formatErrorHandler(int pLine, int formatNum, TokenInfo* token);
  bool errorMHandler(int line);
  
  bool isExpFirst();

};

#endif