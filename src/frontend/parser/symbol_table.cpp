#include "symbol_table.hpp"
#include "../../utils/error_handle.hpp"
#include <map>
#define TAB_LINE 2

/* 全局符号表 */
SymbolTable* globalSymbolTable;
/* 当前符号表 */
SymbolTable* currentSymbolTable;
/* 当前表id */
int currentTableId;

std::map<int, SymbolTable*> allSymbolTable;
/* 存放错误信息 */
extern ErrorList errorList;

char symbolTypeName[][20] {
  "NONE",
  "INT",
  "VOID",
  "ARRAY",
  "FUNC",
};

bool SymbolTable::insertSymbol(std::string* symbolName, SymbolItem* symbolItem)
{
  /* 全局符号表中变量名前加 @V@ ，函数名前加 @F@ ，以示区别 */
  // if (this->isTop) {
  //   if (this->symbols.count("@F@" + *symbolName) != 0 ||
  //       this->symbols.count("@V@" + *symbolName) != 0) {
  //     Log("error : currentSymbolTable already has symbol %s\n", symbolName->c_str());
  //     errorList.addErrorInfo(new ErrorInfo(symbolItem->line, 'b'));
  //     return false;
  //   }
  //   if (symbolItem->symbolType == SymbolType::FUNC) {
  //     this->symbols["@F@" + *symbolName] = symbolItem;
  //   } else {
  //     this->symbols["@V@" + *symbolName] = symbolItem;
  //   }
  //   return true;
  // }
  if (this->symbols.count(*symbolName) != 0) {
    // Log("error : currentSymbolTable already has symbol %s\n", symbolName->c_str());
    errorList.addErrorInfo(new ErrorInfo(symbolItem->line, 'b'));
    return false;
  }
  this->symbols[*symbolName] = symbolItem;
  return true;
}

SymbolItem* SymbolTable::findSymbol(std::string* symbolName)
{
  // if (this->isTop) {
  //   if (this->symbols.count("@F@" + *symbolName) != 0) {
  //     return this->symbols["@F@" + *symbolName];
  //   } 
  //   if (this->symbols.count("@V@" + *symbolName) != 0) {
  //     return this->symbols["@V@" + *symbolName];
  //   }
  //   return NULL;
  // }
  if (this->symbols.count(*symbolName) == 0) {
    if (this->parent == NULL) {
      return NULL;
    }
    return this->parent->findSymbol(symbolName);
  }
  return this->symbols[*symbolName];
}

bool SymbolTable::undefSymbolHandler(std::string* symbolName, int line) 
{
  if (findSymbol(symbolName) == NULL) {
    errorList.addErrorInfo(new ErrorInfo(line, 'c'));
    return true;
  }
  return false;
}

SymbolTable* SymbolTable::newSon()
{
  currentTableId++;
  SymbolTable* table = new SymbolTable(currentTableId);
  table->parent = this;
  this->childs.push_back(table);
  allSymbolTable[currentTableId] = table;
  return table;
}

SymbolTable* SymbolTable::findParent()
{ 
  return this->parent;
}

void SymbolTable::insertNode(std::string* symbolName, SyntaxNode* node, SyntaxNodeType type)
{
  SymbolItem* item;
  if (type == SyntaxNodeType::FUNC_SNT) {
    item = funcDefNode2FuncSymbolItem(node);
  } else {
    item = abstVarDefNode2ObjectSymbolItem((AbstVarDefNode*)node);
  }
  insertSymbol(symbolName, item);
}

FuncSymbolItem* SymbolTable::funcDefNode2FuncSymbolItem(SyntaxNode* node) {
  FuncSymbolItem* funcItem = new FuncSymbolItem();
  FuncDefNode* funcDefNode = (FuncDefNode*)node;
  funcItem->line = funcDefNode->ident->line;
  if (funcDefNode->funcTypeNode->funcType->tokenType == TokenType::INTTK) {
    funcItem->returnType = SymbolType::INT_ST;
  } else {
    funcItem->returnType = SymbolType::VOID_ST;
  }
  if (funcDefNode->hasFuncFParams) {
    std::vector<FuncFParamNode*>* funcFParamNodes = &(funcDefNode->funcFParamsNode->funcFParamNodes);
    std::vector<FuncFParamNode*>::iterator iter;
    for (iter = funcFParamNodes->begin(); iter != funcFParamNodes->end(); iter++) {
      funcItem->funcFParams.push_back(abstVarDefNode2ObjectSymbolItem(*iter));
    }
  }
  return funcItem;
}

ObjectSymbolItem* SymbolTable::abstVarDefNode2ObjectSymbolItem(AbstVarDefNode* node) 
{
  ObjectSymbolItem* objectItem = new ObjectSymbolItem();
  objectItem->line = node->ident->line;
  if (node->arrayDimension == 0) {
    objectItem->symbolType = SymbolType::INT_ST;
  } else {
    objectItem->symbolType = SymbolType::ARRAY_ST;
  }
  objectItem->dimension = node->arrayDimension;
  return objectItem;
}

void SymbolTable::toString(int tabNum)
{
  tprintf(tabNum,"############################\n");
  tprintf(tabNum, "symbol table id = %d\n", this->tableId);
  if (this->parent != NULL) {
    tprintf(tabNum, "symbol table parent id = %d\n", this->parent->tableId);
  } else {
    tprintf(tabNum, "this is top table\n");
  }
  tprintf(tabNum, "----------------------------\n");
  std::unordered_map<std::string, SymbolItem*>::iterator iter;
  for (iter = this->symbols.begin(); iter != this->symbols.end(); iter++) {
    tprintf(tabNum, "symbol name=%s\tsymbol type=%s ", iter->first.c_str(), symbolTypeName[iter->second->symbolType]);
    if (iter->second->symbolType == SymbolType::FUNC_ST) {
      FuncSymbolItem* f = (FuncSymbolItem*)iter->second;
      printf("return type=%s ", symbolTypeName[f->returnType]);
      for (int i = 0; i < f->funcFParams.size(); i++) {
        ObjectSymbolItem* o = (ObjectSymbolItem*)(f->funcFParams[i]);
        printf("param%d: const=%d,dim=%d",i, o->isConst, o->dimension);
      }
      printf("\n");
    } else {
      ObjectSymbolItem* o = (ObjectSymbolItem*)iter->second;
      printf("const=%d,dim=%d\n", o->isConst, o->dimension);
    }
  }
  tprintf(tabNum,"############################\n");
  for (int i = 0; i < this->childs.size(); i++) {
    this->childs[i]->toString(tabNum + 1 * TAB_LINE);
  }
}

void symbolTableInit()
{
  currentSymbolTable = new SymbolTable(0);
  allSymbolTable[0] = currentSymbolTable;
  currentTableId = 0;
  globalSymbolTable = currentSymbolTable;
}

void allSymbolTableToString()
{
  printf("we have %d tables\n", (int)allSymbolTable.size());
  std::map<int, SymbolTable*>::iterator iter;
  for (iter = allSymbolTable.begin(); iter != allSymbolTable.end(); iter++) {
    iter->second->toString(0);
  }
}

bool SymbolTable::funcCallErrorHandler(std::string* funcName, 
  std::vector<ObjectSymbolItem*>* funcCParams, int line)
{
  FuncSymbolItem* f = (FuncSymbolItem*)this->findSymbol(funcName);
  if (f == NULL) {
    return false;
  }
  int rNum = 0;
  for (int i = 0; i < funcCParams->size(); i++) {
    if ((*funcCParams)[i]->symbolType != SymbolType::NONE_ST) {
      rNum++;
    }
  }
  if (rNum != f->funcFParams.size()) {
    /* 函数参数个数不匹配 */
    errorList.addErrorInfo(new ErrorInfo(line, 'd'));
    return true;
  }
  for (int i = 0; i < funcCParams->size(); i++) {
    if (!(*funcCParams)[i]->equals(f->funcFParams[i])) {
      /* 函数参数类型存在不匹配 */
      errorList.addErrorInfo(new ErrorInfo(line, 'e'));
      return true;
    }
  }
  return false;
} 

ObjectSymbolItem* SymbolTable::getFuncReturnType(std::string* funcName)
{
  ObjectSymbolItem* o = new ObjectSymbolItem();
  SymbolItem* item = this->findSymbol(funcName);
  if (item == NULL || item->symbolType != SymbolType::FUNC_ST) {
    o->symbolType = SymbolType::NONE_ST;
    return o;
  }
  o->dimension = 0;
  o->isConst = false;
  o->symbolType = item->symbolType;
  return o;
}

ObjectSymbolItem* SymbolTable::getLValType(LValNode* node)
{
  ObjectSymbolItem* o = new ObjectSymbolItem();
  SymbolItem* item = this->findSymbol(&(node->ident->str));
  if (item == NULL ) {
    o->symbolType = SymbolType::NONE_ST;
    return o;
  }
  int deDim = node->expNodes.size();
  ObjectSymbolItem* oItem = (ObjectSymbolItem*)item;
  o->isConst = oItem->isConst;
  o->dimension = oItem->dimension - deDim;
  if (o->dimension > 0) {
    o->symbolType = SymbolType::ARRAY_ST;
  } else {
    o->symbolType = SymbolType::INT_ST;
  }
  return o;
}

ObjectSymbolItem* SymbolTable::getNumberType()
{
  ObjectSymbolItem* o = new ObjectSymbolItem();
  o->dimension = 0;
  o->isConst = false;
  o->symbolType = SymbolType::INT_ST;
  return o;
}

bool SymbolTable::funcReturnCheck(std::string* funcName, bool hasReturnExp, int line)
{
  FuncSymbolItem* f = (FuncSymbolItem*)this->findSymbol(funcName);
  if (f == NULL) {
    return false;
  }
  if (f->returnType == SymbolType::VOID_ST && hasReturnExp) {
    errorList.addErrorInfo(new ErrorInfo(line, 'f'));
    return true;
  }

  return false;
}

bool SymbolTable::constModifyCheck(LValNode* node)
{
  SymbolItem* item = this->findSymbol(&(node->ident->str));
  if (item == NULL || item->symbolType == SymbolType::FUNC_ST) {
    return false;
  }
  if (((ObjectSymbolItem*)item)->isConst) {
    errorList.addErrorInfo(new ErrorInfo(node->ident->line, 'h'));
    return true;
  }
  return false;
}