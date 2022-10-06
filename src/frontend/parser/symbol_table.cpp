#include "symbol_table.hpp"
#include <map>
#define TAB_LINE 2

/* 当前符号表 */
SymbolTable* currentSymbolTable;
/* 全局函数表 */
SymbolTable* funcTable;

int currentTableId;

std::map<int, SymbolTable*> allSymbolTable;

char symbolTypeName[][20] {
  "NONE",
  "VAR",
  "CONST",
  "FUNC",
  "FUNCFPARAM",
};

bool SymbolTable::insertSymbol(std::string* symbolName, SymbolItem* symbolItem)
{
  if (this->symbols.count(*symbolName) != 0) {
    Log("error : currentSymbolTable already has symbol %s\n", symbolName->c_str());
    return false;
  }
  this->symbols[*symbolName] = symbolItem;
  return true;
}

SymbolItem* SymbolTable::findSymbol(std::string* symbolName)
{
  if (this->symbols.count(*symbolName) == 0) {
    if (this->parent == NULL) {
      return NULL;
    }
    return this->parent->findSymbol(symbolName);
  }
  return this->symbols[*symbolName];
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

void SymbolTable::insertNode(std::string* symbolName, SyntaxNode* node, SymbolType type)
{
  SymbolItem* symbolItem = new SymbolItem(node, type);
  this->insertSymbol(symbolName, symbolItem);
  /* need error handle */
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
    tprintf(tabNum, "symbol name=%s\tsymbol type=%s\n", iter->first.c_str(), symbolTypeName[iter->second->symbolType]);
  }
  tprintf(tabNum,"############################\n");
  for (int i = 0; i < this->childs.size(); i++) {
    this->childs[i]->toString(tabNum + 1 * TAB_LINE);
  }
}

void symbolTableInit()
{
  currentSymbolTable = new SymbolTable(1);
  funcTable = new SymbolTable(0);
  allSymbolTable[0] = funcTable;
  allSymbolTable[1] = currentSymbolTable;
  currentTableId = 1;
  /* 初始化时将main函数加入函数表 */
  funcTable->insertSymbol(new std::string("main"), new SymbolItem(NULL, SymbolType::FUNC));
}

void allSymbolTableToString()
{
  printf("we have %d tables\n", (int)allSymbolTable.size());
  std::map<int, SymbolTable*>::iterator iter;
  for (iter = allSymbolTable.begin(); iter != allSymbolTable.end(); iter++) {
    iter->second->toString(0);
  }
}