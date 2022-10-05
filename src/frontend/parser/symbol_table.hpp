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
  std::pair<int, int> tableId; /* first为层数，second为该层顺序解析到的第几个table */
  // std::vector<SymbolTable*> childs;
  public:
  SymbolTable(std::pair<int, int> id) : parent(NULL), tableId(id) {}

  bool insertSymbol(std::string* symbolName, SymbolItem* symbolItem);

  SymbolItem* findSymbol(std::string* symbolName);
  /* 有副作用 */
  SymbolTable* newSon();
  /* 有副作用 */
  SymbolTable* findParent();

  void insertNode(std::string* symbolName, SyntaxNode* node, SymbolType type);

  void toString();

  int getFirstId() { return tableId.first; }
  int getSecondId() { return tableId.second; }

};

class SymbolItem {
  public:
  SymbolType symbolType;
  SyntaxNode* defNode;
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