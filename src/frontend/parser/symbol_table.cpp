#include "symbol_table.hpp"
#include <map>
/* 当前符号表，需要在递归下降前手动分配 */
SymbolTable* currentSymbolTable;
SymbolTable* funcTable;

int currentTableFirstId;
int currentTableSecondId;
int lastTableFirstId;

std::map<std::pair<int, int>, SymbolTable*> allSymbolTable;

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
  currentTableFirstId++;
  // if (currentTableFirstId == lastTableFirstId) {
    currentTableSecondId++;
  // } else {
    // currentTableSecondId = 0;
  // }
  SymbolTable* table = new SymbolTable({currentTableFirstId, currentTableSecondId});
  table->parent = this;
  // Log("currentTableFirstId = %d, currentTableSecondId = %d\n", currentTableFirstId, currentTableSecondId);
  allSymbolTable[{currentTableFirstId, currentTableSecondId}] = table;
  return table;
}

SymbolTable* SymbolTable::findParent()
{ 
  // lastTableFirstId = currentTableFirstId;
  // currentTableFirstId = this->parent->getFirstId();
  // currentTableSecondId = this->parent->getSecondId();
  return this->parent;
}

void SymbolTable::insertNode(std::string* symbolName, SyntaxNode* node, SymbolType type)
{
  SymbolItem* symbolItem = new SymbolItem(node, type);
  this->insertSymbol(symbolName, symbolItem);
  /* need error handle */
}

void SymbolTable::toString()
{
  printf("----------------------------\n");
  printf("symbol table id = %d\t%d\n", this->tableId.first, this->tableId.second);
  if (this->parent != NULL) {
    printf("symbol table parent id = %d\t%d\n", this->parent->tableId.first, this->parent->tableId.second);
  } else {
    printf("this is top table\n");
  }
  std::unordered_map<std::string, SymbolItem*>::iterator iter;
  for (iter = this->symbols.begin(); iter != this->symbols.end(); iter++) {
    printf("symbol name=%s\tsymbol type=%s\n", iter->first.c_str(), symbolTypeName[iter->second->symbolType]);
  }
  printf("----------------------------\n");
}

void symbolTableInit()
{
  currentSymbolTable = new SymbolTable({0, 1});
  funcTable = new SymbolTable({0, 0});
  allSymbolTable[{0, 0}] = funcTable;
  allSymbolTable[{0, 1}] = currentSymbolTable;
  currentTableFirstId = 0;
  currentTableSecondId = 1;
  lastTableFirstId = 0;

  funcTable->insertSymbol(new std::string("main"), new SymbolItem(NULL, SymbolType::FUNC));

}

void allSymbolTableToString()
{
  Log("we have %d tables\n", (int)allSymbolTable.size());
  std::map<std::pair<int, int>, SymbolTable*>::iterator iter;
  for (iter = allSymbolTable.begin(); iter != allSymbolTable.end(); iter++) {
    iter->second->toString();
  }
}