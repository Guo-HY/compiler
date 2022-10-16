#include "parser.hpp"
#include "../include/error.hpp"

extern char tokenName[][20];
extern char tokenValue[][20];
extern SymbolTable* currentSymbolTable;
extern SymbolTable* globalSymbolTable;
extern ErrorList errorList;
extern void symbolTableInit();
/* 当前循环层数 */
int cycleDepth = 0;
/* ------------------ tools function ------------------ */
TokenInfo* Parser::popToken() 
{
  if (tokenInfoList[nowTokenListPtr]->tokenType == TokenType::END) {
    Log("error : pop END\n");
    return NULL;
  }
  TokenInfo* now = tokenInfoList[nowTokenListPtr];
  Log("\t\t\tpop %s\n", now->str.c_str());
  nowTokenListPtr++;
  return now;
}

TokenInfo* Parser::peekToken(int num)
{
  if (nowTokenListPtr + num >= (int)tokenInfoList.size() ||
      nowTokenListPtr + num < 0) {
    Log("error : peek out of bounds\n");
    return NULL;
  }
  return tokenInfoList[nowTokenListPtr + num];
}
/* ------------------ tools function ------------------ */

/* ------------------ error handle ------------------ */
bool Parser::tokenLackHandler(TokenType tokenType)
{
  Log("in tokenLackHandler\n");
  if (peekToken(0)->tokenType != tokenType) {
    ErrorInfo* errorInfo = new ErrorInfo();
    errorInfo->line = peekToken(-1)->line;
    switch (tokenType)
    {
    case TokenType::SEMICN:
      errorInfo->errorType = 'i';
      break;
    case TokenType::RPARENT:
      errorInfo->errorType = 'j';
      break;
    case TokenType::RBRACK:
      errorInfo->errorType = 'k';
      break;
    default:
      errorInfo->errorType = '#';
      break;
    }
    errorList.addErrorInfo(errorInfo);
    return true;
  }
  popToken();
  return false;
}

bool Parser::formatErrorHandler(int pLine, int formatNum, TokenInfo* token)
{
  Log("in formatErrorHandler\n");
  const char* str = token->str.c_str();
  int fNum = 0;
  int fLine = token->line;
  for (int i = 1; str[i] != '"'; i++) {
    if (str[i] == 32 || str[i] == 33 || (str[i] >= 40 && str[i] <= 126)) {
      if (str[i] == '\\' && str[i + 1] != 'n') {
        errorList.addErrorInfo(new ErrorInfo(fLine, 'a'));
        return true;
      }
    } else if (str[i] == '%' && str[i + 1] == 'd') {
      fNum++;
    } else {
      errorList.addErrorInfo(new ErrorInfo(fLine, 'a'));
      return true;
    }
  }
  if (fNum != formatNum) {
    errorList.addErrorInfo(new ErrorInfo(pLine, 'l'));
    return true;
  }
  return false;
}

bool Parser::errorMHandler(int line) 
{
  Log("in errorMHandler\n");
  if (cycleDepth == 0) {
    errorList.addErrorInfo(new ErrorInfo(line, 'm'));
    return true;
  }
  return false;
}
/* ------------------ error handle ------------------ */

CompUnitNode* Parser::compUnitAnalyse() 
{
  Log("in compUnitAnalyse\n");
  /* 初始化符号表 */
  symbolTableInit();
  CompUnitNode* node = new CompUnitNode();
  /* deal Decl : CONSTTK | INTTK IDENFR !LPARENT */ 
  while ((peekToken(0)->tokenType == TokenType::CONSTTK) || 
        (peekToken(0)->tokenType == TokenType::INTTK && 
        peekToken(1)->tokenType == TokenType::IDENFR && 
        peekToken(2)->tokenType != TokenType::LPARENT)) {
          node->declNodes.push_back(declAnalyse());
        }
  /* deal FuncDef : VOIDTK | INTTK !MAINTK */
  while ((peekToken(0)->tokenType == TokenType::VOIDTK) ||
        (peekToken(0)->tokenType == TokenType::INTTK &&
        peekToken(1)->tokenType != TokenType::MAINTK)) {
          node->funcDefNodes.push_back(funcDefAnalyse());
        }

  node->mainFuncDefNode = mainFuncDefAnalyse();
  return node;
}

DeclNode* Parser::declAnalyse()
{
  Log("in declAnalyse\n");
  DeclNode* node = new DeclNode();
  /* deal ConstDecl : CONSTTK */
  if (peekToken(0)->tokenType == TokenType::CONSTTK) {
    node->declNodeType = DeclNodeType::DECL_CONST;
    node->constDeclNode = constDeclAnalyse();
  } else {
    /* deal VarDecl : INTTK */
    node->declNodeType = DeclNodeType::DECL_VAR;
    node->varDeclNode = varDeclAnalyse();
  }
  return node;
}

ConstDeclNode* Parser::constDeclAnalyse()
{
  Log("in constDeclAnalyse\n");
  ConstDeclNode* node = new ConstDeclNode();
  popToken(); /* eat CONSTTK */
  node->bTypeNode = bTypeAnalyse();
  node->constDefNodes.push_back(constDefAnalyse(node->bTypeNode));
  /* deal { ',' ConstDef }  */
  while (peekToken(0)->tokenType == TokenType::COMMA) {
    popToken(); /* eat COMMA */
    node->constDefNodes.push_back(constDefAnalyse(node->bTypeNode));
  }
  /* check if lack SEMICN ,if not lack then eat */
  tokenLackHandler(TokenType::SEMICN);
  return node;
}

BTypeNode* Parser::bTypeAnalyse()
{
  Log("in bTypeAnalyse\n");
  BTypeNode* node = new BTypeNode();
  popToken(); /* eat INTTK */
  return node;
}

ConstDefNode* Parser::constDefAnalyse(BTypeNode* bType)
{
  Log("in constDefAnalyse\n");
  ConstDefNode* node = new ConstDefNode();
  node->isConst = true;
  node->bTypeNode = bType;
  node->ident = peekToken(0);
  popToken(); /* eat IDENFR */

  /* deal { '[' ConstExp ']' } */
  while (peekToken(0)->tokenType == TokenType::LBRACK) {
    popToken(); /* eat LBRACK */
    (node->arrayDimension)++;
    node->constExpNodes.push_back(constExpAnalyse());
    /* check if lack RBRACK if not lack then eat */
    tokenLackHandler(TokenType::RBRACK);
  }
  popToken(); /* eat ASSIGN */
  node->constInitValNode = constInitValAnalyse();

  /* insert symbol to currentSymbolTable */
  currentSymbolTable->insertNode(&(node->ident->str), (SyntaxNode*)node, SyntaxNodeType::ABSTVAR_SNT);

  return node;
}

ConstInitValNode* Parser::constInitValAnalyse()
{ 
  Log("in constInitValAnalyse\n");
  ConstInitValNode* node = new ConstInitValNode();
  /* deal '{' [ ConstInitVal { ',' ConstInitVal } ] '}' */
  if (peekToken(0)->tokenType == TokenType::LBRACE) {
    node->initArray = true;
    popToken(); /* eat LBRACE */
    if (peekToken(0)->tokenType == TokenType::RBRACE) {
      popToken(); /* eat RBRACE */
      return node;
    }
    node->constInitValNodes.push_back(constInitValAnalyse());
    while (peekToken(0)->tokenType == TokenType::COMMA) {
      popToken(); /* eat COMMA */
      node->constInitValNodes.push_back(constInitValAnalyse());
    }
    popToken(); /* eat RBRACE */
    return node;
  }
  /* deal ConstExp */
  node->initArray = false;
  node->constExpNode = constExpAnalyse();
  return node;
}

VarDeclNode* Parser::varDeclAnalyse()
{
  Log("in varDeclAnalyse\n");
  VarDeclNode* node = new VarDeclNode();
  node->bTypeNode = bTypeAnalyse();
  node->varDefNodes.push_back(varDefAnalyse(node->bTypeNode));
  /* deal { ',' VarDef } */
  while (peekToken(0)->tokenType == TokenType::COMMA) {
    popToken(); /* eat COMMA */
    node->varDefNodes.push_back(varDefAnalyse(node->bTypeNode));
  }
  /* check if lack SEMICN ,if not lack then eat */
  tokenLackHandler(TokenType::SEMICN);
  return node;
} 

VarDefNode* Parser::varDefAnalyse(BTypeNode* bType)
{
  Log("in varDefAnalyse\n");
  VarDefNode* node = new VarDefNode();
  node->bTypeNode = bType;
  node->ident = peekToken(0);
  popToken(); /* eat IDENFR */
  /* deal { '[' ConstExp ']' } */
  while (peekToken(0)->tokenType == TokenType::LBRACK) {
    popToken(); /* eat LBRACK */
    (node->arrayDimension)++;
    node->constExpNodes.push_back(constExpAnalyse());
    /* check if lack RBRACK ,if not lack then eat */
    tokenLackHandler(TokenType::RBRACK);
  } 
  /* deal [ '=' InitVal ] */
  if (peekToken(0)->tokenType == TokenType::ASSIGN) {
    popToken(); /* eat ASSIGN */
    node->hasInitVal = true;
    node->initValNode = initValAnalyse();
  }

  /* insert symbol to currentSymbolTable */
  currentSymbolTable->insertNode(&(node->ident->str), (SyntaxNode*)node, SyntaxNodeType::ABSTVAR_SNT);

  return node;
}

InitValNode* Parser::initValAnalyse()
{
  Log("in initValAnalyse\n");
  InitValNode* node = new InitValNode();
  /* deal '{' [ InitVal { ',' InitVal } ] '}' */
  if (peekToken(0)->tokenType == TokenType::LBRACE) {
    popToken(); /* eat LBRACE */
    node->initArray = true;
    if (peekToken(0)->tokenType == TokenType::RBRACE) {
      popToken(); /* eat RBRACE */
      return node;
    }
    node->initValNodes.push_back(initValAnalyse());
    while (peekToken(0)->tokenType == TokenType::COMMA) {
      popToken(); /* eat COMMA */
      node->initValNodes.push_back(initValAnalyse());
    }
    popToken(); /* eat RBRACE */
    return node;
  }
  /* deal Exp */
  node->initArray = false;
  node->expNode = expAnalyse(NULL, false, NULL);
  return node;
}

FuncDefNode* Parser::funcDefAnalyse()
{
  Log("in funcDefAnalyse\n");
  /* 在解析函数形参前需要新建符号表 */
  currentSymbolTable = currentSymbolTable->newSon();

  FuncDefNode* node = new FuncDefNode();
  node->funcTypeNode = funcTypeAnalyse();
  node->ident = peekToken(0);
  popToken(); /* eat IDENFR */
  popToken(); /* eat LPARENT */
  /* deal [FuncFParams] */
  if (peekToken(0)->tokenType == TokenType::INTTK) {
    node->funcFParamsNode = funcFParamsAnalyse();
    node->hasFuncFParams = true;
  }
  /* check if lack RPARENT ,if not lack then eat */
  tokenLackHandler(TokenType::RPARENT);

  /* insert symbol to globalSymbolTable 需要在解析block前，解析形参后将函数插入符号表 */
  globalSymbolTable->insertNode(&(node->ident->str), (SyntaxNode*)node, SyntaxNodeType::FUNC_SNT);
  /* deal Block */
  node->blockNode = blockAnalyse(false, &(node->ident->str), 
  (node->funcTypeNode->funcType->tokenType == INTTK));
  /* 回溯符号表 */
  currentSymbolTable = currentSymbolTable->findParent();
  return node;
}

MainFuncDefNode* Parser::mainFuncDefAnalyse()
{
  Log("in mainFuncDefAnalyse\n");
  /* 在解析函数形参前需要新建符号表 */
  currentSymbolTable = currentSymbolTable->newSon();

  MainFuncDefNode* node = new MainFuncDefNode();
  node->funcTypeNode = funcTypeAnalyse();
  node->ident = peekToken(0);
  popToken(); /* eat MAINTK */
  popToken(); /* eat LPARENT */
  /* check if lack RPARENT ,if not lack then eat */
  tokenLackHandler(TokenType::RPARENT);

  /* insert symbol to globalSymbolTable 需要在解析block前，解析形参后将函数插入符号表 */
  globalSymbolTable->insertNode(&(node->ident->str), (SyntaxNode*)node, SyntaxNodeType::FUNC_SNT);
  /* deal Block */
  node->blockNode = blockAnalyse(false, &(node->ident->str), true);
  /* 回溯符号表 */
  currentSymbolTable = currentSymbolTable->findParent();
  return node;
}

FuncTypeNode* Parser::funcTypeAnalyse()
{
  Log("in funcTypeAnalyse\n");
  FuncTypeNode* node = new FuncTypeNode();
  node->funcType = peekToken(0);
  popToken(); /* eat VOIDTK or INTTK */
  return node;
}

FuncFParamsNode* Parser::funcFParamsAnalyse()
{
  Log("in funcFParamsAnalyse\n");
  FuncFParamsNode* node = new FuncFParamsNode();
  node->funcFParamNodes.push_back(FuncFParamAnalyse());
  /* deal { ',' FuncFParam } */
  while (peekToken(0)->tokenType == TokenType::COMMA) {
    popToken(); /* eat COMMA */
    node->funcFParamNodes.push_back(FuncFParamAnalyse());
  }
  return node;
}

FuncFParamNode* Parser::FuncFParamAnalyse()
{
  Log("in FuncFParamAnalyse\n");
  FuncFParamNode* node = new FuncFParamNode();
  node->bTypeNode = bTypeAnalyse();
  node->ident = peekToken(0);
  popToken(); /* eat IDENFR */
  if (peekToken(0)->tokenType == TokenType::LBRACK) {
    popToken(); /* eat LBRACK */
    /* check if lack RBRACK ,if not lack then eat */
    tokenLackHandler(TokenType::RBRACK);
    node->arrayDimension = 1;
    while (peekToken(0)->tokenType == TokenType::LBRACK) {
      popToken(); /* eat LBRACK */
      node->arrayDimension++;
      node->constExpNodes.push_back(constExpAnalyse());
      /* check if lack RBRACK ,if not lack then eat */
      tokenLackHandler(TokenType::RBRACK);
    }
  }

  /* insert symbol to currentSymbolTable */
  currentSymbolTable->insertNode(&(node->ident->str), (SyntaxNode*)node, SyntaxNodeType::ABSTVAR_SNT);

  return node;
}

BlockNode* Parser::blockAnalyse(bool newSymbolTable, std::string* funcName, bool needRE)
{
  Log("in blockAnalyse\n");
  bool hasRE = false;
  if (newSymbolTable) {
    currentSymbolTable = currentSymbolTable->newSon();
  }
  BlockNode* node = new BlockNode();
  popToken(); /* eat LBRACE */
  while (peekToken(0)->tokenType != TokenType::RBRACE) {
    node->blockItemNodes.push_back(blockItemAnalyse(funcName, &hasRE));
  }
  /* check error g */
  if (needRE && !hasRE) {
    errorList.addErrorInfo(new ErrorInfo(peekToken(0)->line, 'g'));
  }
  popToken(); /* eat RBRACE */

  if (newSymbolTable) {
    currentSymbolTable = currentSymbolTable->findParent();
  }
  return node;
}

BlockItemNode* Parser::blockItemAnalyse(std::string* funcName, bool* hasRE)
{
  Log("in blockItemAnalyse\n");
  BlockItemNode* node = new BlockItemNode();
  /* deal Decl */
  if (peekToken(0)->tokenType == TokenType::CONSTTK || 
    peekToken(0)->tokenType == TokenType::INTTK) {
      node->blockItemType = BlockItemType::BLOCKITEM_DECL;
      node->declNode = declAnalyse();
      *hasRE = false;
      return node;
    }
  /* deal Stmt */
  node->blockItemType = BlockItemType::BLOCKITEM_STMT;
  node->stmtNode = stmtAnalyse(funcName, hasRE);
  return node;
}

bool Parser::isExpFirst() 
{
  Log("in isExpFirst\n");
  TokenType type = peekToken(0)->tokenType;
  if (type == TokenType::LPARENT || type == TokenType::IDENFR ||
      type == TokenType::INTCON || type == TokenType::PLUS ||
      type == TokenType::MINU || type == TokenType::NOT) {
    return true;
  }
  return false;
}

StmtNode* Parser::stmtAnalyse(std::string* funcName, bool* hasRE)
{
  Log("in stmtAnalyse\n");
  *hasRE = false;
  bool tmpRE;
  StmtNode* node = new StmtNode();
  TokenType t = peekToken(0)->tokenType;
  
  if (t == TokenType::PRINTFTK) {
    /* deal 'printf' '(' FormatString{ ',' Exp} ')' ';' */
    node->stmtType = StmtType::STMT_PRINTF;
    int fNum = 0; /* 解析到的参数个数 */
    int pLine = peekToken(0)->line; /* printf的行号 */
    popToken(); /* eat PRINTFTK */
    popToken(); /* eat LPARENT */
    node->formatString = peekToken(0);
    popToken(); /* eat STRCON */
    while (peekToken(0)->tokenType == TokenType::COMMA) {
      popToken(); /* eat COMMA */
      node->expNodes.push_back(expAnalyse(NULL, false, NULL));
      fNum++;
    }
    /* check error a & l */
    formatErrorHandler(pLine, fNum, node->formatString);
    /* check if lack RPARENT if not lack then eat */
    tokenLackHandler(TokenType::RPARENT);
    /* check if lack SEMICN if not lack then eat */
    tokenLackHandler(TokenType::SEMICN);
  } else if (t == TokenType::RETURNTK) {
    /* deal 'return' [Exp] ';' */ 
    node->stmtType = StmtType::STMT_RETURN;
    int rLine = peekToken(0)->line; /* return的行号 */
    popToken(); /* eat RETURNTK */
    if (isExpFirst()) { /* 存在Exp */
      node->expNodes.push_back(expAnalyse(NULL, false, NULL));
      /* check error f */
      globalSymbolTable->funcReturnCheck(funcName, true, rLine);
      *hasRE = true;
    }
    /* check if lack SEMICN ,if not lack then eat */
    tokenLackHandler(TokenType::SEMICN);
  } else if (t == TokenType::BREAKTK) {
    /* deal 'break' ';' */
    node->stmtType = StmtType::STMT_BREAK;
    errorMHandler(peekToken(0)->line);
    popToken(); /* eat BREAKTK */
    /* check if lack SEMICN ,if not lack then eat */
    tokenLackHandler(TokenType::SEMICN);
  } else if (t == TokenType::CONTINUETK) {
    /* deal 'continue' ';' */
    errorMHandler(peekToken(0)->line);
    node->stmtType = StmtType::STMT_CONTINUE;
    popToken(); /* eat CONTINUETK */
    /* check if lack SEMICN ,if not lack then eat */
    tokenLackHandler(TokenType::SEMICN);
  } else if (t == TokenType::WHILETK) {
    /* deal 'while' '(' Cond ')' Stmt */
    node->stmtType = StmtType::STMT_WHILE;
    cycleDepth++; /* 所处循环层数+1 */
    popToken(); /* eat WHILETK */
    popToken(); /* eat LPARENT */
    node->condNode = condAnalyse();
    /* check if lack RPARENT ,if not lack then eat */
    tokenLackHandler(TokenType::RPARENT);
    node->stmtNodes.push_back(stmtAnalyse(funcName, &tmpRE));
    cycleDepth--; /* 退出时循环层数-1 */
  } else if (t == TokenType::IFTK) {
    /* deal 'if' '(' Cond ')' Stmt [ 'else' Stmt ] */
    node->stmtType = StmtType::STMT_IF;
    popToken(); /* eat IFTK */
    popToken(); /* eat LPARENT */
    node->condNode = condAnalyse();
    /* check if lack RPARENT ,if not lack then eat */
    tokenLackHandler(TokenType::RPARENT);
    node->stmtNodes.push_back(stmtAnalyse(funcName, &tmpRE));
    if (peekToken(0)->tokenType == TokenType::ELSETK) {
      popToken(); /* eat ELSETK */
      node->hasElse = true;
      node->stmtNodes.push_back(stmtAnalyse(funcName, &tmpRE));
    }
  } else if (t == TokenType::LBRACE) {
    /* deal Block */
    node->stmtType = StmtType::STMT_BLOCK;
    node->blockNode = blockAnalyse(true, funcName, false);

    /* 此时需要解析LVal '=' Exp ';' 或者 [Exp] ';'
        如果当前token是IDENFR并且下一个token不是LPARENT，那么当前元素一定为LVal，
       并且无法判断是LVal '=' Exp ';'还是[Exp] ';'，此时先解析一个LVal，然后判断接下来的token是否为等号，
       如果是等号就解析LVal '=' Exp ';'，否则解析[Exp] ';'
        否则就一定是[Exp] ';'
    */
  } else if (peekToken(0)->tokenType == TokenType::IDENFR && 
             peekToken(1)->tokenType != TokenType::LPARENT) {
    /* 此时一定是一个LVal，先解析出来 */
    LValNode* lValNodeTmp = lValAnalyse();
    if (peekToken(0)->tokenType == TokenType::ASSIGN) { /* LVal '=' Exp ';' */
      /* check error h */
      currentSymbolTable->constModifyCheck(lValNodeTmp);
      node->stmtType = StmtType::STMT_ASSIGN;
      node->lValNode = lValNodeTmp;
      popToken(); /* eat ASSIGN */
      if (peekToken(0)->tokenType == TokenType::GETINTTK) { /* LVal '=' 'getint' '(' ')' ';' */
        node->stmtType = StmtType::STMT_GETINT;
        popToken(); /* eat GETINTTK */
        popToken(); /* eat LPARENT */
        /* check if lack RPARENT ,if not lack then eat */
        tokenLackHandler(TokenType::RPARENT);
        /* check if lack SEMICN , if not lack then eat */
        tokenLackHandler(TokenType::SEMICN);
        return node;
      }
      node->expNodes.push_back(expAnalyse(NULL, false, NULL));
      /* check if lack SEMICN ,if not lack then eat */
      tokenLackHandler(TokenType::SEMICN);
    } else { /* [Exp] ';' */
      node->stmtType = StmtType::STMT_EXP;
      node->expNodes.push_back(expAnalyse(lValNodeTmp, false, NULL));// 此处传入已经分析好的lval
      /* check if lack SEMICN ,if not lack then eat */
      tokenLackHandler(TokenType::SEMICN);
    }
  } else {
    /* 此时一定为[Exp] ';' */
    node->stmtType = StmtType::STMT_EXP;
    if (isExpFirst()) {
      node->expNodes.push_back(expAnalyse(NULL, false, NULL));
    }
    /* check if lack SEMICN ,if not lack then eat */
    tokenLackHandler(TokenType::SEMICN);
  }

  return node;
}

ExpNode* Parser::expAnalyse(LValNode* lval, bool addToNFCP, 
  std::vector<ObjectSymbolItem*>* funcCParams)
{
  Log("in expAnalyse\n");
  ExpNode* node = new ExpNode();
  node->addExpNode = addExpAnalyse(lval, addToNFCP, funcCParams);
  return node;
}

CondNode* Parser::condAnalyse() 
{
  Log("in condAnalyse\n");
  CondNode* node = new CondNode();
  node->lOrExpNode = lOrExpAnalyse();
  return node;
}

LValNode* Parser::lValAnalyse()
{
  Log("in lValAnalyse\n");
  LValNode* node = new LValNode();
  node->ident = peekToken(0);
  popToken(); /* eat IDENFR */
  /* check error c */
  currentSymbolTable->undefSymbolHandler(&(node->ident->str), node->ident->line);
  while (peekToken(0)->tokenType == TokenType::LBRACK) {
    popToken(); /* eat LBRACK */
    node->expNodes.push_back(expAnalyse(NULL, false, NULL));
    /* check if lack RBRACK ,if not lack then eat */
    tokenLackHandler(TokenType::RBRACK);
  }
  return node;
}

PrimaryExpNode* Parser::primaryExpAnalyse(LValNode* lval, bool addToNFCP, 
  std::vector<ObjectSymbolItem*>* funcCParams)
{   
  Log("in primaryExpAnalyse\n");
  PrimaryExpNode* node = new PrimaryExpNode();
  if (lval != NULL) {
    node->primaryExpType = PrimaryExpType::PRIMARY_LVAL;
    node->lValNode = lval;
    if (addToNFCP == true) {
      /* 向funcCParams中添加元素 */
      funcCParams->push_back(currentSymbolTable->getLValType(lval));
    }
    return node;
  }

  TokenType t = peekToken(0)->tokenType;
  switch (t)
  {
  case TokenType::LPARENT:  /* deal '(' Exp ')' */
    node->primaryExpType = PrimaryExpType::PRIMARY_EXP;
    popToken(); /* eat LPARENT */
    node->expNode = expAnalyse(NULL, addToNFCP, funcCParams);
    /* check if lack RPARENT ,if not lack then eat */
    tokenLackHandler(TokenType::RPARENT);
    break;
  case TokenType::IDENFR: /* deal LVal */
    node->primaryExpType = PrimaryExpType::PRIMARY_LVAL;
    node->lValNode = lValAnalyse();
    if (addToNFCP == true) {
      /* 向funcCParams中添加元素 */
      funcCParams->push_back(currentSymbolTable->getLValType(node->lValNode));
    }
    break;
  case TokenType::INTCON: /* deal Number */
    node->primaryExpType = PrimaryExpType::PRIMARY_NUMBER;
    node->numberNode = numberAnalyse();
    if (addToNFCP == true) {
      /* 向funcCParams中添加元素 */
      funcCParams->push_back(currentSymbolTable->getNumberType());
    }
    break;
  default:
    Log("error:tokenType=%s,tokenValue=%s,poi=%d\n", tokenName[t], tokenValue[t], nowTokenListPtr);
    break;
  }
  return node;
}

NumberNode* Parser::numberAnalyse()
{
  Log("in numberAnalyse\n");
  NumberNode* node = new NumberNode();
  node->intConst = peekToken(0);
  popToken(); /* eat INTCON */
  return node;
}

UnaryExpNode* Parser::unaryExpAnalyse(LValNode* lval, bool addToNFCP, 
  std::vector<ObjectSymbolItem*>* funcCParams)
{
  Log("in unaryExpAnalyse\n");
  bool ret;
  UnaryExpNode* node = new UnaryExpNode();
  if (lval != NULL) {
    node->unaryExpType = UnaryExpType::UNARY_PRIMARYEXP;
    node->primaryExpNode = primaryExpAnalyse(lval, addToNFCP, funcCParams);
    return node;
  }
  /* deal UnaryOp UnaryExp */
  if (peekToken(0)->tokenType == TokenType::PLUS || 
      peekToken(0)->tokenType == TokenType::MINU ||
      peekToken(0)->tokenType == TokenType::NOT) 
  {
    node->unaryExpType = UnaryExpType::UNARY_PREFIX;
    node->unaryOpNode = unaryOpAnalyse();
    node->unaryExpNode = unaryExpAnalyse(NULL, addToNFCP, funcCParams);
  } 
  /* deal Ident '(' [FuncRParams] ')' */
  else if (peekToken(0)->tokenType == TokenType::IDENFR && 
           peekToken(1)->tokenType == TokenType::LPARENT) 
  {
    node->unaryExpType = UnaryExpType::UNARY_FUNCCALL;
    node->ident = peekToken(0);
    /* check error c ,需要在全局符号表中查找,未找到返回true */
    ret = globalSymbolTable->undefSymbolHandler(&(node->ident->str), node->ident->line);
    popToken(); /* eat IDENFR */
    popToken(); /* eat LPARENT */
    std::vector<ObjectSymbolItem*>* thisFuncCParams = new std::vector<ObjectSymbolItem*>;
    if (isExpFirst()) {
      node->hasFuncRParams = true;
      node->funcRParamsNode = funcRParamsAnalyse(thisFuncCParams);
    }
    if (ret == false) {
      /* check error d, e */
      globalSymbolTable->funcCallErrorHandler(&(node->ident->str), thisFuncCParams, node->ident->line);
    }
    if (ret == false && addToNFCP == true) {
      /* 如果符号表中存在函数名，并且addToNFCP为真，就要在funcCParams中填入函数返回值类型 */
      funcCParams->push_back(globalSymbolTable->getFuncReturnType(&(node->ident->str)));
    }
    /* check if lack RPARENT ,if not lack then eat */
    tokenLackHandler(TokenType::RPARENT);
  } 
  /* deal PrimaryExp */
  else 
  {
    node->unaryExpType = UnaryExpType::UNARY_PRIMARYEXP;
    node->primaryExpNode = primaryExpAnalyse(NULL, addToNFCP, funcCParams);
  }
  return node;
}

UnaryOpNode* Parser::unaryOpAnalyse()
{
  Log("in unaryOpAnalyse\n");
  UnaryOpNode* node = new UnaryOpNode();
  node->unaryOp = peekToken(0);
  popToken(); /* eat PLUS | MINU | NOT */
  return node;
}

FuncRParamsNode* Parser::funcRParamsAnalyse(std::vector<ObjectSymbolItem*>* funcCParams)
{
  Log("in funcRParamsAnalyse\n");
  FuncRParamsNode* node = new FuncRParamsNode();
  node->expNodes.push_back(expAnalyse(NULL, true, funcCParams));
  while (peekToken(0)->tokenType == TokenType::COMMA) {
    popToken(); /* eat COMMA */
    node->expNodes.push_back(expAnalyse(NULL, true, funcCParams));
  }
  return node;
}

MulExpNode* Parser::mulExpAnalyse(LValNode* lval, bool addToNFCP, std::vector<ObjectSymbolItem*>* funcCParams)
{
  Log("in mulExpAnalyse\n");
  MulExpNode* node = new MulExpNode(BinaryExpType::MUL);
  node->unaryExpNodes.push_back(unaryExpAnalyse(lval, addToNFCP, funcCParams));
  while (peekToken(0)->tokenType == TokenType::MULT ||
  peekToken(0)->tokenType == TokenType::DIV ||
  peekToken(0)->tokenType == TokenType::MOD) {
    node->ops.push_back(peekToken(0));
    popToken(); /* eat MULT | DIV | MID */
    node->unaryExpNodes.push_back(unaryExpAnalyse(NULL, false, NULL));
  }
  return node;
}

AddExpNode* Parser::addExpAnalyse(LValNode* lval, bool addToNFCP, std::vector<ObjectSymbolItem*>* funcCParams)
{
  Log("in addExpAnalyse\n");
  AddExpNode* node = (AddExpNode*)new BinaryExpNode(BinaryExpType::ADD);
  node->operands.push_back(mulExpAnalyse(lval, addToNFCP, funcCParams));
  while (peekToken(0)->tokenType == TokenType::PLUS ||
  peekToken(0)->tokenType == TokenType::MINU) {
    node->ops.push_back(peekToken(0));
    popToken(); /* eat PLUS | MINU */
    node->operands.push_back(mulExpAnalyse(NULL, false, NULL));
  }
  return node;
}

RelExpNode* Parser::relExpAnalyse()
{
  Log("in relExpAnalyse\n");
  RelExpNode* node = (RelExpNode*)new BinaryExpNode(BinaryExpType::REL);
  node->operands.push_back(addExpAnalyse(NULL, false, NULL));
  while (peekToken(0)->tokenType == TokenType::LSS || 
  peekToken(0)->tokenType == TokenType::GRE ||
  peekToken(0)->tokenType == TokenType::LEQ ||
  peekToken(0)->tokenType == TokenType::GEQ) {
    node->ops.push_back(peekToken(0));
    popToken(); /* eat LSS | GRE | LEQ | GEQ */
    node->operands.push_back(addExpAnalyse(NULL, false, NULL));
  }
  return node;
}

EqExpNode* Parser::eqExpAnalyse()
{
  Log("in eqExpAnalyse\n");
  EqExpNode* node = (EqExpNode*)new BinaryExpNode(BinaryExpType::EQ);
  node->operands.push_back(relExpAnalyse());
  while (peekToken(0)->tokenType == TokenType::EQL ||
  peekToken(0)->tokenType == TokenType::NEQ) {
    node->ops.push_back(peekToken(0));
    popToken(); /* eat EQL | NEQ */
    node->operands.push_back(relExpAnalyse());
  }
  return node;
}

LAndExpNode* Parser::lAndExpAnalyse()
{
  Log("in lAndExpAnalyse\n");
  LAndExpNode* node = (LAndExpNode*)new BinaryExpNode(BinaryExpType::LAND);
  node->operands.push_back(eqExpAnalyse());
  while (peekToken(0)->tokenType == TokenType::AND) {
    node->ops.push_back(peekToken(0));
    popToken(); /* eat AND */
    node->operands.push_back(eqExpAnalyse());
  }
  return node;
}

LOrExpNode* Parser::lOrExpAnalyse()
{
  Log("in lOrExpAnalyse\n");
  LOrExpNode* node = (LOrExpNode*)new BinaryExpNode(BinaryExpType::LOR);
  node->operands.push_back(lAndExpAnalyse());
  while (peekToken(0)->tokenType == TokenType::OR) {
    node->ops.push_back(peekToken(0));
    popToken(); /* eat OR */
    node->operands.push_back(lAndExpAnalyse());
  }
  return node;
}

ConstExpNode* Parser::constExpAnalyse()
{
  Log("in constExpAnalyse\n");
  ConstExpNode* node = new ConstExpNode();
  node->addExpNode = addExpAnalyse(NULL, false, NULL);
  return node;
}

void Parser::toString()
{
  Log("in\n");
  if (root == NULL) {
    Log("before compUnitAnalyse\n");
    root = compUnitAnalyse();
    Log("after compUnitAnalyse\n");
  }
  std::string s = root->toString();
  Log("after root->toString\n");
  printf("%s", s.c_str());
}

SyntaxNode* Parser::syntaxAnalyse()
{ 
  if (root != NULL) {
    return root;
  }
  root = compUnitAnalyse();
  return root;
}