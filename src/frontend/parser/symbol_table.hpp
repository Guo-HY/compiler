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

  /* error handle */
  bool undefSymbolHandler(std::string* symbolName, int line);
  bool funcCallErrorHandler(std::string* funcName, std::vector<ObjectSymbolItem*>* funcCParams, int line);
  /* 如果出错，返回NONE类型的Object */
  ObjectSymbolItem* getFuncReturnType(std::string* funcName);
  ObjectSymbolItem* getLValType(LValNode* node);
  ObjectSymbolItem* getNumberType();
  bool funcReturnCheck(std::string* funcName, bool hasReturnExp, int line);
  bool constModifyCheck(LValNode* node);

  private:
  /* 将符号对象插入符号表中，成功返回true，失败返回false */
  bool insertSymbol(std::string* symbolName, SymbolItem* symbolItem);
  /* 根据符号名在符号表中寻找符号 */
  SymbolItem* findSymbol(std::string* symbolName);

  FuncSymbolItem* funcDefNode2FuncSymbolItem(SyntaxNode* node);
  ObjectSymbolItem* abstVarDefNode2ObjectSymbolItem(AbstVarDefNode* node);

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

  bool equals(ObjectSymbolItem* obj) {
    return (this->dimension == obj->dimension &&
    this->isConst == obj->isConst &&
    this->symbolType == obj->symbolType);
  }
};

class FuncSymbolItem : public SymbolItem {
  public:
  SymbolType returnType;
  std::vector<ObjectSymbolItem*> funcFParams;

  FuncSymbolItem() : returnType(SymbolType::NONE_ST) {
    SymbolItem();
  }

};


#endif