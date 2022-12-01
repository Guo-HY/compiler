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

void symbolTableInit()
{
  /* TODO: 需要检查，当symbolTable不为NULL时需要delete，防止内存泄露 */
  //Log("in symbolTableInit\n");
  currentSymbolTable = new SymbolTable(0);
  allSymbolTable[0] = currentSymbolTable;
  currentTableId = 0;
  globalSymbolTable = currentSymbolTable;
}

SymbolTable* SymbolTable::newSon()
{
  //Log("in newSon\n");
  currentTableId++;
  SymbolTable* table = new SymbolTable(currentTableId);
  table->parent = this;
  this->childs.push_back(table);
  allSymbolTable[currentTableId] = table;
  return table;
}

SymbolTable* SymbolTable::findParent()
{ 
  //Log("in findParent\n");
  if (this->parent == NULL) {
    Log("error : parent = NULL\n");
  }
  return this->parent;
}

bool SymbolTable::insertSymbol(std::string* symbolName, SymbolItem* symbolItem)
{
  //Log("in insertSymbol\n");
  if (this->name2symbols.count(*symbolName) != 0) {
    Log("error : currentSymbolTable already has symbol %s\n", symbolName->c_str());
    errorList.addErrorInfo(new ErrorInfo(symbolItem->line, 'b'));
    return false;
  }
  this->name2symbols[*symbolName] = symbolItem;
  return true;
}

SymbolItem* SymbolTable::findSymbol(std::string* symbolName)
{
  //Log("in findSymbol\n");
  if (this->name2symbols.count(*symbolName) == 0) {
    if (this->parent == NULL) {
      return NULL;
    }
    return this->parent->findSymbol(symbolName);
  }
  return this->name2symbols[*symbolName];
}

SymbolItem* SymbolTable::insertNode(std::string* symbolName, SyntaxNode* node, SyntaxNodeType type)
{
  //Log("in insertNode\n");
  SymbolItem* item;
  if (type == SyntaxNodeType::FUNC_SNT) {
    item = funcDefNode2FuncSymbolItem(node);
  } else {
    item = abstVarDefNode2ObjectSymbolItem((AbstVarDefNode*)node);
  }
  insertSymbol(symbolName, item);
  return item;
}

SymbolItem* SymbolTable::insertNodeWithLlvmIrId(std::string* symbolName, SyntaxNode* node, SyntaxNodeType type, int id)
{
  //Log("");
  SymbolItem* item = insertNode(symbolName, node, type);
  item->llvmIrId = id;
  if (this->id2symbols.count(id) != 0) {
    panic("error");
  }
  this->id2symbols[id] = item;
  return item;
}

FuncSymbolItem* SymbolTable::funcDefNode2FuncSymbolItem(SyntaxNode* node) 
{
  //Log("in funcDefNode2FuncSymbolItem\n");
  FuncSymbolItem* funcItem = new FuncSymbolItem();
  FuncDefNode* funcDefNode = (FuncDefNode*)node;
  funcItem->line = funcDefNode->ident->line;
  funcItem->symbolType = SymbolType::FUNC_ST;
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
  //Log("in abstVarDefNode2ObjectSymbolItem\n");
  ObjectSymbolItem* objectItem = new ObjectSymbolItem();
  //Log("before");
  objectItem->line = node->ident->line;
  //Log("after");
  objectItem->isConst = node->isConst;
  if (node->arrayDimension == 0) {
    objectItem->symbolType = SymbolType::INT_ST;
  } else {
    objectItem->symbolType = SymbolType::ARRAY_ST;
  }
  objectItem->dimension = node->arrayDimension;
  return objectItem;
}

ObjectSymbolItem* SymbolTable::getFuncReturnType(std::string* funcName)
{
  //Log("in getFuncReturnType\n");
  ObjectSymbolItem* o = new ObjectSymbolItem();
  SymbolItem* item = this->findSymbol(funcName);
  if (item == NULL || item->symbolType != SymbolType::FUNC_ST) {
    o->symbolType = SymbolType::NONE_ST;
    return o;
  }
  FuncSymbolItem* fitem = (FuncSymbolItem*)item;
  o->dimension = 0;
  o->isConst = false;
  o->symbolType = fitem->returnType;
  return o;
}

ObjectSymbolItem* SymbolTable::getLValType(LValNode* node)
{
  //Log("in getLValType\n");
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
  //Log("in getNumberType\n");
  ObjectSymbolItem* o = new ObjectSymbolItem();
  o->dimension = 0;
  o->isConst = false;
  o->symbolType = SymbolType::INT_ST;
  return o;
}

// void SymbolTable::setLlvmIrId(std::string* symbolName, int id)
// {
//   SymbolItem* item = this->findSymbol(symbolName);
//   if (item == NULL || (item->symbolType != SymbolType::ARRAY_ST && item->symbolType != SymbolType::INT_ST)) {
//     panic("error");
//   }
//   ObjectSymbolItem* oitem = (ObjectSymbolItem*)item;
//   oitem->llvmIrId = id;
// }

int SymbolTable::getLlvmIrId(std::string* symbolName)
{
  SymbolItem* item = this->findSymbol(symbolName);
  // if (item == NULL || (item->symbolType != SymbolType::ARRAY_ST && item->symbolType != SymbolType::INT_ST)) {
  //   panic("error");
  // }
  return item->llvmIrId;
}

SymbolItem* SymbolTable::findSymbolByLlvmIrId(int id)
{
  if (this->id2symbols.count(id) != 0) {
    return this->id2symbols[id];
  }
  if (this->parent == NULL) {
    panic("error");
  }
  return this->parent->findSymbolByLlvmIrId(id);
}

void SymbolTable::updateLlvmIrIdById(int oldId, int newId)
{
  SymbolItem* item = findSymbolByLlvmIrId(oldId);
  if (item == NULL ) {
    panic("error");
  }
  item->llvmIrId = newId;
}

void SymbolTable::addInitValue(std::string* symbolName, int value)
{
  // Log("add value : name is %s, value is %d", symbolName->c_str(), value);
  SymbolItem* item = this->findSymbol(symbolName);
  if (item == NULL) {
    panic("error");
  }
  if (item->symbolType != SymbolType::INT_ST && item->symbolType != SymbolType::ARRAY_ST) {
    panic("error");
  }
  ObjectSymbolItem* oitem = (ObjectSymbolItem*)item;
  oitem->initValues.push_back(value);
}

int SymbolTable::getInitValue(std::string* symbolName, std::vector<int> ptrs)
{
  SymbolItem* item = this->findSymbol(symbolName);
  if (item == NULL) {
    panic("error");
  }
  if (item->symbolType != SymbolType::INT_ST && item->symbolType != SymbolType::ARRAY_ST) {
    panic("error");
  }
  ObjectSymbolItem* oitem = (ObjectSymbolItem*)item;
  int ptr = 0;
  if (item->type == NULL) {
    panic("error");
  }
  if (item->type->typeIdtfr ==TypeIdtfr::INTEGER_TI) {
    return oitem->initValues[ptr];
  }
  if (ptrs.size() == 1) {
    return oitem->initValues[ptrs[0]];
  }
  std::vector<int> dims = ((ArrayType*)(item->type))->getDims();
  return oitem->initValues[ptrs[0] * dims[0] + ptrs[1]];
}

void SymbolTable::addVarType(std::string* symbolName, Type* type)
{
  SymbolItem* item = this->findSymbol(symbolName);
  item->type = type;
}

/* ----------------------------- error handler & toString ----------------------------- */

bool SymbolTable::undefSymbolHandler(std::string* symbolName, int line) 
{
  //Log("in undefSymbolHandler\n");
  if (findSymbol(symbolName) == NULL) {
    errorList.addErrorInfo(new ErrorInfo(line, 'c'));
    return true;
  }
  return false;
}

void allSymbolTableToString()
{
  //Log("in allSymbolTableToString\n");
  printf("we have %d tables\n", (int)allSymbolTable.size());
  std::map<int, SymbolTable*>::iterator iter;
  for (iter = allSymbolTable.begin(); iter != allSymbolTable.end(); iter++) {
    iter->second->toString(0);
  }
}

bool SymbolTable::funcCallErrorHandler(std::string* funcName, 
  std::vector<ObjectSymbolItem*>* funcCParams, int line)
{
  //Log("in funcCallErrorHandler\n");
  FuncSymbolItem* f = (FuncSymbolItem*)this->findSymbol(funcName);
  if (f == NULL) {
    return false;
  }
  int rNum = 0;
  for (u_long i = 0; i < funcCParams->size(); i++) {
    if ((*funcCParams)[i]->symbolType != SymbolType::NONE_ST) {
      rNum++;
    }
  }
  if (rNum != (int)f->funcFParams.size()) {
    /* 函数参数个数不匹配 */
    errorList.addErrorInfo(new ErrorInfo(line, 'd'));
    return true;
  }
  for (u_long i = 0; i < funcCParams->size(); i++) {
    if (!(*funcCParams)[i]->equals(f->funcFParams[i])) {
      /* 函数参数类型存在不匹配 */
      errorList.addErrorInfo(new ErrorInfo(line, 'e'));
      return true;
    }
  }
  return false;
} 

bool SymbolTable::funcReturnCheck(std::string* funcName, bool hasReturnExp, int line)
{
  //Log("in funcReturnCheck\n");
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
  //Log("in constModifyCheck\n");
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

void SymbolTable::toString(int tabNum)
{
  //Log("in SymbolTable::toString\n");
  tprintf(tabNum,"############################\n");
  tprintf(tabNum, "symbol table id = %d\n", this->tableId);
  if (this->parent != NULL) {
    tprintf(tabNum, "symbol table parent id = %d\n", this->parent->tableId);
  } else {
    tprintf(tabNum, "this is top table\n");
  }
  tprintf(tabNum, "----------------------------\n");
  std::unordered_map<std::string, SymbolItem*>::iterator iter;
  for (iter = this->name2symbols.begin(); iter != this->name2symbols.end(); iter++) {
    tprintf(tabNum, "symbol name=%s\tsymbol type=%s ", iter->first.c_str(), symbolTypeName[iter->second->symbolType]);
    if (iter->second->symbolType == SymbolType::FUNC_ST) {
      FuncSymbolItem* f = (FuncSymbolItem*)iter->second;
      printf("return type=%s ", symbolTypeName[f->returnType]);
      for (u_long i = 0; i < f->funcFParams.size(); i++) {
        ObjectSymbolItem* o = (ObjectSymbolItem*)(f->funcFParams[i]);
        printf("param%ld: const=%d,dim=%d",i, o->isConst, o->dimension);
      }
      printf("\n");
    } else {
      ObjectSymbolItem* o = (ObjectSymbolItem*)iter->second;
      printf("const=%d,dim=%d\n", o->isConst, o->dimension);
    }
  }
  tprintf(tabNum,"############################\n");
  for (u_long i = 0; i < this->childs.size(); i++) {
    this->childs[i]->toString(tabNum + 1 * TAB_LINE);
  }
}