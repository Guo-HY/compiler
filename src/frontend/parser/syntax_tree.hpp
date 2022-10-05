#ifndef _SYNTAX_TREE_H
#define _SYNTAX_TREE_H 1

#include <string>
#include <vector>
#include "../lexer/lexer.hpp"
#include "../../include/error.hpp"

class SyntaxNode;
class CompUnitNode;
class DeclNode;
class ConstDeclNode;
class BTypeNode;
class ConstDefNode;
class ConstInitValNode;
class VarDeclNode;
class VarDefNode;
class InitValNode;
class FuncDefNode;
class MainFuncDefNode;
class FuncTypeNode;
class FuncFParamsNode;
class FuncFParamNode;
class BlockNode;
class BlockItemNode;
class StmtNode;
class ExpNode;
class CondNode;
class LValNode;
class PrimaryExpNode;
class NumberNode;
class UnaryExpNode;
class UnaryOpNode;
class FuncRParamsNode;
class MulExpNode;
class AddExpNode;
class RelExpNode;
class EqExpNode;
class LAndExpNode;
class LOrExpNode;
class ConstExpNode;

enum DeclNodeType {
  DECL_NONE,
  DECL_CONST, /* ConstDecl */
  DECL_VAR,   /* VarDecl */
};

enum BlockItemType {
  BLOCKITEM_NONE,
  BLOCKITEM_DECL,
  BLOCKITEM_STMT,
};

enum PrimaryExpType {
  PRIMARY_NONE,
  PRIMARY_EXP,
  PRIMARY_LVAL,
  PRIMARY_NUMBER,
};

enum UnaryExpType {
  UNARY_NONE,
  UNARY_PRIMARYEXP,
  UNARY_FUNCCALL,
  UNARY_PREFIX,
};

enum StmtType {
  STMT_NONE,
  STMT_ASSIGN,
  STMT_EXP,
  STMT_BLOCK,
  STMT_IF,
  STMT_WHILE,
  STMT_BREAK,
  STMT_CONTINUE,
  STMT_RETURN,
  STMT_PRINTF,
  STMT_GETINT,
};

class SyntaxNode {
  public:

  virtual std::string toString() = 0;

};

class CompUnitNode : public SyntaxNode {
  public:
  std::vector<DeclNode*> declNodes;
  std::vector<FuncDefNode*> funcDefNodes;
  MainFuncDefNode* mainFuncDefNode;

  public:
  CompUnitNode(): mainFuncDefNode(NULL) {}
  std::string toString() override;
};

class DeclNode : public SyntaxNode {
  public:
  DeclNodeType declNodeType;
  ConstDeclNode* constDeclNode;
  VarDeclNode* varDeclNode;

  public:
  DeclNode(): declNodeType(DECL_NONE), constDeclNode(NULL), 
  varDeclNode(NULL) {}
  std::string toString() override;

};

class ConstDeclNode : public SyntaxNode {
  public:
  BTypeNode* bTypeNode;
  std::vector<ConstDefNode*> constDefNodes;

  public:
  ConstDeclNode(): bTypeNode(NULL) {}
  std::string toString() override;
};

class BTypeNode : public SyntaxNode {

  public:
  std::string toString() override;
};

class ConstDefNode : public SyntaxNode {
  public:
  BTypeNode* bTypeNode;
  TokenInfo* ident;
  int arrayDimension;
  std::vector<ConstExpNode*> constExpNodes;
  ConstInitValNode* constInitValNode;
  
  public:
  ConstDefNode(): bTypeNode(NULL), ident(NULL), arrayDimension(0), 
  constInitValNode(NULL) {}
  std::string toString() override;
};

class ConstInitValNode : public SyntaxNode {
  public:
  bool initArray;  /* false : ConstInitVal ::=  ConstExp */
  ConstExpNode* constExpNode;
  std::vector<ConstInitValNode*> constInitValNodes;

  public:
  ConstInitValNode(): initArray(false), constExpNode(NULL) {}
  std::string toString() override;

};

class VarDeclNode : public SyntaxNode {
  public:
  BTypeNode* bTypeNode;
  std::vector<VarDefNode*> varDefNodes;

  public:
  VarDeclNode(): bTypeNode(NULL) {}
  std::string toString() override;
};

class VarDefNode : public SyntaxNode {
  public:
  TokenInfo* ident;
  BTypeNode* bTypeNode;
  std::vector<ConstExpNode*> constExpNodes;
  bool hasInitVal;  
  InitValNode* initValNode;
  
  public:
  VarDefNode(): ident(NULL), bTypeNode(NULL), hasInitVal(false), initValNode(NULL) {}
  std::string toString() override;
};

class InitValNode : public SyntaxNode {
  public:
  bool initArray; /* true : InitVal ::= '{' [ InitVal { ',' InitVal } ] '}' */
  ExpNode* expNode;
  std::vector<InitValNode*> initValNodes;

  public:
  InitValNode(): initArray(false), expNode(NULL) {}
  std::string toString() override;
};

class FuncDefNode : public SyntaxNode {
  public:
  FuncTypeNode* funcTypeNode;
  TokenInfo* ident;
  bool hasFuncFParams;
  FuncFParamsNode* funcFParamsNode;
  BlockNode* blockNode;
  
  public:
  FuncDefNode(): funcTypeNode(NULL), ident(NULL), 
  hasFuncFParams(false), funcFParamsNode(NULL), blockNode(NULL) {} 
  std::string toString() override;
};

class MainFuncDefNode : public SyntaxNode {
  public:
  BlockNode* blockNode;
  
  public:
  MainFuncDefNode(): blockNode(NULL) {}
  std::string toString() override;
};

class FuncTypeNode : public SyntaxNode {
  public:
  TokenInfo* funcType;
  
  public:
  FuncTypeNode(): funcType(NULL) {}
  std::string toString() override;
};

class FuncFParamsNode : public SyntaxNode {
  public:
  std::vector<FuncFParamNode*> funcFParamNodes;
  
  public:
  std::string toString() override;
};

class FuncFParamNode : public SyntaxNode {
  public:
  BTypeNode* bTypeNode;
  TokenInfo* ident;
  int arrayDimension;
  std::vector<ConstExpNode*> constExpNodes;
  
  public:
  FuncFParamNode(): bTypeNode(NULL), ident(NULL), 
  arrayDimension(0) {}
  std::string toString() override;
};

class BlockNode : public SyntaxNode {
  public:
  std::vector<BlockItemNode*> blockItemNodes;
  
  public:
  std::string toString() override;
};

class BlockItemNode : public SyntaxNode {
  public:
  BlockItemType blockItemType;
  DeclNode* declNode;
  StmtNode* stmtNode;
  
  public:
  BlockItemNode(): blockItemType(BLOCKITEM_NONE), declNode(NULL), 
  stmtNode(NULL) {}
  std::string toString() override;
};

class StmtNode : public SyntaxNode {
  public:
  StmtType stmtType;
  LValNode* lValNode;
  std::vector<ExpNode*> expNodes;
  BlockNode* blockNode;
  CondNode* condNode;
  std::vector<StmtNode*> stmtNodes;
  TokenInfo* formatString;
  bool hasElse;
  public:
  StmtNode(): stmtType(STMT_NONE), lValNode(NULL),
  blockNode(NULL), condNode(NULL), formatString(NULL),
  hasElse(false) {}
  std::string toString() override;
};

class ExpNode : public SyntaxNode {
  public:
  AddExpNode* addExpNode;
  
  public:
  ExpNode(): addExpNode(NULL) {}
  std::string toString() override;
};

class CondNode : public SyntaxNode {
  public:
  LOrExpNode* lOrExpNode;
  
  public:
  CondNode(): lOrExpNode(NULL) {}
  std::string toString() override;
};

class LValNode : public SyntaxNode {
  public:
  TokenInfo* ident;
  std::vector<ExpNode*> expNodes;
  
  public:
  LValNode(): ident(NULL) {}
  std::string toString() override;
};

class PrimaryExpNode : public SyntaxNode {
  public:
  PrimaryExpType primaryExpType;
  ExpNode* expNode;
  LValNode* lValNode;
  NumberNode* numberNode;
  
  public:
  PrimaryExpNode(): primaryExpType(PRIMARY_NONE), expNode(NULL), 
  lValNode(NULL), numberNode(NULL) {}
  std::string toString() override;
};

class NumberNode : public SyntaxNode {
  public:
  TokenInfo* intConst;
  
  public:
  NumberNode(): intConst(NULL) {}
  std::string toString() override;
};

class UnaryExpNode : public SyntaxNode {
  public:
  UnaryExpType unaryExpType;
  PrimaryExpNode* primaryExpNode;
  TokenInfo* ident;
  bool hasFuncRParams;
  FuncRParamsNode* funcRParamsNode;
  UnaryOpNode* unaryOpNode;
  UnaryExpNode* unaryExpNode;

  public:
  UnaryExpNode(): unaryExpType(UNARY_NONE), primaryExpNode(NULL), 
  ident(NULL), hasFuncRParams(false), funcRParamsNode(NULL), 
  unaryOpNode(NULL), unaryExpNode(NULL) {}
  std::string toString() override;
};

class UnaryOpNode : public SyntaxNode {
  public:
  TokenInfo* unaryOp;
  
  public:
  UnaryOpNode(): unaryOp(NULL) {}
  std::string toString() override;
};

class FuncRParamsNode : public SyntaxNode {
  public:
  std::vector<ExpNode*> expNodes;
  
  public:
  std::string toString() override;
};

class MulExpNode : public SyntaxNode {
  public:
  std::vector<UnaryExpNode*> unaryExpNodes;
  std::vector<TokenInfo*> ops;
  public:
  std::string toString() override;
};

class AddExpNode : public SyntaxNode {
  public:
  // bool isLeaf;
  // MulExpNode* mulExpNode;
  // TokenInfo* op;
  // AddExpNode* addExpNode;
  std::vector<MulExpNode*> MulExpNodes;
  std::vector<TokenInfo*> ops;
  public:
  // AddExpNode(): isLeaf(false), mulExpNode(NULL), op(NULL), 
  // addExpNode(NULL) {} 
  std::string toString() override;
};

class RelExpNode : public SyntaxNode {
  public:
  // bool isLeaf;
  // AddExpNode* addExpNode;
  // TokenInfo* op;
  // RelExpNode* relExpNode;
  std::vector<AddExpNode*> addExpNodes;
  std::vector<TokenInfo*> ops;  
  public:
  // RelExpNode(): isLeaf(false), addExpNode(NULL), op(NULL), 
  // relExpNode(NULL) {}
  std::string toString() override;
};

class EqExpNode : public SyntaxNode {
  public:
  // bool isLeaf;
  // RelExpNode* relExpNode;
  // TokenInfo* op;
  // EqExpNode* eqExpNode;
  std::vector<RelExpNode*> relExpNodes;
  std::vector<TokenInfo*> ops;
  public:
  // EqExpNode(): isLeaf(false), relExpNode(NULL), op(NULL), eqExpNode(NULL) {}
  std::string toString() override;
};

class LAndExpNode : public SyntaxNode {
  public:
  // bool isLeaf;
  // EqExpNode* eqExpNode;
  // TokenInfo* op;
  // LAndExpNode* lAndExpNode;
  std::vector<EqExpNode*> eqExpNodes;
  std::vector<TokenInfo*> ops;
  public:
  // LAndExpNode(): isLeaf(false), eqExpNode(NULL), op(NULL), lAndExpNode(NULL) {}
  std::string toString() override;
};

class LOrExpNode : public SyntaxNode {
  public:
  // bool isLeaf;
  // LAndExpNode* lAndExpNode;
  // TokenInfo* op;
  // LOrExpNode* lOrExpNode;
  std::vector<LAndExpNode*> lAndExpNodes;
  std::vector<TokenInfo*> ops;
  public:
  // LOrExpNode(): isLeaf(false), lAndExpNode(NULL), op(NULL), lOrExpNode(NULL) {}
  std::string toString() override;
};

class ConstExpNode : public SyntaxNode {
  public:
  AddExpNode* addExpNode;
  
  public:
  ConstExpNode(): addExpNode(NULL) {}
  std::string toString() override;
};


#endif