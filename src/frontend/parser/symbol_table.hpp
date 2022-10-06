#ifndef _SYMBOL_TABLE_H
#define _SYMBOL_TABLE_H 1

#include <unordered_map>
#include <string>
#include <vector>
#include <utility>
#include "syntax_tree.hpp"

class SymbolItem;
class SymbolTable;

enum SymbolType {
  NONE,
  VAR,
  CONST,
  FUNC,
  FUNCFPARAM,
};

class SymbolTable {
  private:

  SymbolTable* parent;
  std::unordered_map<std::string, SymbolItem*> symbols;
  int tableId;
  std::vector<SymbolTable*> childs;
  public:
  SymbolTable(int id) : parent(NULL), tableId(id) {}

  bool insertSymbol(std::string* symbolName, SymbolItem* symbolItem);

  SymbolItem* findSymbol(std::string* symbolName);
  /* 有副作用 */
  SymbolTable* newSon();
  /* 有副作用 */
  SymbolTable* findParent();

  void insertNode(std::string* symbolName, SyntaxNode* node, SymbolType type);

  void toString(int tabRet);

  int getTableId() { return this->tableId; }

};

class SymbolItem {
  public:
  SymbolType symbolType;
  SyntaxNode* defNode;    /* defNode只可能是SymbolType定义的几种类型之一 */
  // VarDefNode* varDef;
  // ConstDefNode* constDef;
  // FuncDefNode* funcDef;

  SymbolItem() : symbolType(SymbolType::NONE), defNode(NULL) {}

  SymbolItem(SyntaxNode* node, SymbolType type) : symbolType(type), defNode(node) {}
  // SymbolItem() : symbolType(SymbolType::NONE), varDef(NULL), 
  //   constDef(NULL), funcDef(NULL) {}

  // SymbolItem(VarDefNode* node) : symbolType(SymbolType::VAR), 
  //   varDef(node), constDef(NULL), funcDef(NULL) {}
  
  // SymbolItem(ConstDefNode* node) : symbolType(SymbolType::VAR), 
  //   varDef(NULL), constDef(node), funcDef(NULL) {}

  // SymbolItem(FuncDefNode* node) : symbolType(SymbolType::VAR), 
  //   varDef(NULL), constDef(NULL), funcDef(node) {}
};


#endif