#ifndef _SYMBOL_TABLE_H
#define _SYMBOL_TABLE_H 1

#include <unordered_map>
#include <string>
#include <vector>
#include <utility>
#include "syntax_tree.hpp"

class SymbolItem;
class SymbolTable;
class ObjectSymbolItem;
class FuncSymbolItem;

enum SymbolType {
  NONE_ST,
  INT_ST,
  VOID_ST,
  ARRAY_ST,
  FUNC_ST,
};

enum SyntaxNodeType {
  FUNC_SNT, 
  ABSTVAR_SNT,

};


class SymbolTable {
  private:

  bool isTop;
  SymbolTable* parent;
  std::unordered_map<std::string, SymbolItem*> symbols;
  int tableId;
  std::vector<SymbolTable*> childs;

  public:
  SymbolTable(int id) : isTop(false), parent(NULL), tableId(id) {}
  
  SymbolTable* newSon();
  SymbolTable* findParent();

  void insertNode(std::string* symbolName, SyntaxNode* node, SyntaxNodeType type);

  void toString(int tabRet);

  int getTableId() { return this->tableId; }

  bool undefSymbolHandler(std::string* symbolName, int line);

  private:
  /* 将符号对象插入符号表中，成功返回true，失败返回false */
  bool insertSymbol(std::string* symbolName, SymbolItem* symbolItem);
  /* 根据符号名在符号表中寻找符号 */
  SymbolItem* findSymbol(std::string* symbolName);

  FuncSymbolItem* funcDefNode2FuncSymbolItem(SyntaxNode* node);
  ObjectSymbolItem* abstVarDefNode2ObjectSymbolItem(AbstVarDefNode* node);
  // ObjectSymbolItem* funcFParamNode2ObjectSymbolItem(SyntaxNode* node);
  // ObjectSymbolItem* constDefNode2ObjectSymbolItem(SyntaxNode* node);
  // ObjectSymbolItem* varDefNode2ObjectSymbolItem(SyntaxNode* node);

};

class SymbolItem {
  public:
  SymbolType symbolType;
  int line;

  SymbolItem() : symbolType(SymbolType::NONE_ST), line(-1) {}

  SymbolItem(SymbolType type, int l) : symbolType(type), line(l) {}

};

class ObjectSymbolItem : public SymbolItem {
  public:
  bool isConst;
  int dimension;

  ObjectSymbolItem() : isConst(false), dimension(0) {
    SymbolItem();
  }

};

class FuncSymbolItem : public SymbolItem {
  public:
  SymbolType returnType;
  std::vector<SymbolItem*> funcFParams;

  FuncSymbolItem() : returnType(SymbolType::NONE_ST) {
    SymbolItem();
  }

};


#endif