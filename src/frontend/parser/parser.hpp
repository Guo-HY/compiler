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
  CompUnitNode* getRoot() { return this->root; }

  private:
  /**
   * @brief 根节点
   */
  CompUnitNode* root;
  /**
   * @brief 当前关注的token指针，处理完一个语法元素后，指针永远指向下一个
   */
  int nowTokenListPtr;
  /**
   * @brief pop 会返回当前token的指针并将nowTokenListPtr加一
   * @return TokenInfo* 
   */
  TokenInfo* popToken();
  /**
   * @brief peek 会返回当前位置+num处的token，但不会改变nowTokenListPtr
   * @param num 
   * @return TokenInfo* 
   */
  TokenInfo* peekToken(int num);

  CompUnitNode* compUnitAnalyse();
  DeclNode* declAnalyse();
  ConstDeclNode* constDeclAnalyse();
  BTypeNode* bTypeAnalyse();
  /**
   * @brief 
   * @param bType 变量定义类型
   * @return ConstDefNode* 
   */
  ConstDefNode* constDefAnalyse(BTypeNode* bType);
  ConstInitValNode* constInitValAnalyse(std::string* name);
  VarDeclNode* varDeclAnalyse();
  /**
   * @brief 
   * @param bType 变量定义类型
   * @return VarDefNode* 
   */
  VarDefNode* varDefAnalyse(BTypeNode* bType);
  InitValNode* initValAnalyse();
  FuncDefNode* funcDefAnalyse();
  MainFuncDefNode* mainFuncDefAnalyse();
  FuncTypeNode* funcTypeAnalyse();
  FuncFParamsNode* funcFParamsAnalyse();
  FuncFParamNode* FuncFParamAnalyse();
  /* newSymbolTable 为真表示需要新建符号表 */
  /**
   * @brief 
   * @param newSymbolTable 为真表示需要新建符号表
   * @param funcName block所在函数名
   * @param needRE 该block是否需要return语句
   * @return BlockNode* 
   */
  BlockNode* blockAnalyse(bool newSymbolTable, std::string* funcName, bool needRE);
  /**
   * @brief 
   * @param funcName 所在函数名
   * @param hasRE 返回该blockItem是否存在return语句
   * @return BlockItemNode* 
   */
  BlockItemNode* blockItemAnalyse(std::string* funcName, bool* hasRE);
  /**
   * @brief 
   * 
   * @param funcName 所在函数名
   * @param hasRE 返回该stmt是否存在return语句 
   * @return StmtNode* 
   */
  StmtNode* stmtAnalyse(std::string* funcName, bool* hasRE);
  /**
   * @brief 
   * @param lval lval不为NULL代表exp中的lval已经被解析
   * @param addToNFCP  是否要添加到funcCParams中
   * @param funcCParams 函数实参表
   * @return ExpNode* 
   */
  ExpNode* expAnalyse(LValNode* lval, bool addToNFCP, std::vector<ObjectSymbolItem*>* funcCParams);
  CondNode* condAnalyse();
  LValNode* lValAnalyse();
  /**
   * @brief 
   * @param lval lval不为NULL代表exp中的lval已经被解析
   * @param addToNFCP 是否要添加到funcCParams中
   * @param funcCParams 函数实参表
   * @return PrimaryExpNode* 
   */
  PrimaryExpNode* primaryExpAnalyse(LValNode* lval, bool addToNFCP, std::vector<ObjectSymbolItem*>* funcCParams);
  NumberNode* numberAnalyse();
  /**
   * @brief 
   * @param lval lval不为NULL代表exp中的lval已经被解析
   * @param addToNFCP 是否要添加到funcCParams中
   * @param funcCParams 函数实参表
   * @return UnaryExpNode* 
   */
  UnaryExpNode* unaryExpAnalyse(LValNode* lval, bool addToNFCP, std::vector<ObjectSymbolItem*>* funcCParams);
  UnaryOpNode* unaryOpAnalyse();
  /**
   * @brief 
   * @param funcCParams 函数实参表
   * @return FuncRParamsNode* 
   */
  FuncRParamsNode* funcRParamsAnalyse(std::vector<ObjectSymbolItem*>* funcCParams);
  /**
   * @brief 
   * @param lval lval不为NULL代表exp中的lval已经被解析
   * @param addToNFCP 是否要添加到funcCParams中
   * @param funcCParams 函数实参表 
   * @return MulExpNode* 
   */
  MulExpNode* mulExpAnalyse(LValNode* lval, bool addToNFCP, std::vector<ObjectSymbolItem*>* funcCParams);
  /**
   * @brief 
   * @param lval lval不为NULL代表exp中的lval已经被解析
   * @param addToNFCP 是否要添加到funcCParams中
   * @param funcCParams 函数实参表 
   * @return AddExpNode* 
   */
  AddExpNode* addExpAnalyse(LValNode* lval, bool addToNFCP, std::vector<ObjectSymbolItem*>* funcCParams);
  RelExpNode* relExpAnalyse();
  EqExpNode* eqExpAnalyse();
  LAndExpNode* lAndExpAnalyse();
  LOrExpNode* lOrExpAnalyse();
  ConstExpNode* constExpAnalyse();

  /**
   * @brief 检查类型为tokenType的符号是否缺失，
   * 如果缺失就将错误信息加入errorList，否则popToken
   * @param tokenType 
   * @return true 
   * @return false 
   */
  bool tokenLackHandler(TokenType tokenType);
  /**
   * @brief 检查格式字符串是否含有非法字符，参数个数是否匹配
   * 
   * @param pLine printf所在行号
   * @param formatNum 参数个数
   * @param token 格式字符串
   * @return true 
   * @return false 
   */
  bool formatErrorHandler(int pLine, int formatNum, TokenInfo* token);
  /**
   * @brief 处理m类错误
   * 
   * @param line 
   * @return true 
   * @return false 
   */
  bool errorMHandler(int line);
  /**
   * @brief 下一个元素是否为Exp
   * 
   * @return true 
   * @return false 
   */
  bool isExpFirst();

};

#endif