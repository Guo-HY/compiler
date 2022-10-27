#ifndef _SYMBOL_TABLE_H
#define _SYMBOL_TABLE_H 1

#include <unordered_map>
#include <string>
#include <vector>
#include <utility>
#include "syntax_tree.hpp"

#define NO_LLVMIRID 0

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
  std::unordered_map<std::string, SymbolItem*> name2symbols;
  std::unordered_map<int, SymbolItem*> id2symbols;
  int tableId;
  std::vector<SymbolTable*> childs;

  public:
  SymbolTable(int id) : isTop(false), parent(NULL), tableId(id) {}

  /**
   * @brief 新建一个子表并返回
   * @return SymbolTable* 
   */
  SymbolTable* newSon();

  /**
   * @brief 返回父表指针
   * @return SymbolTable* 
   */
  SymbolTable* findParent();

  /**
   * @brief 在表中插入一条记录
   * @param symbolName 符号名
   * @param node 语法树节点
   * @param type 语法树节点类型
   */
  SymbolItem* insertNode(std::string* symbolName, SyntaxNode* node, SyntaxNodeType type);

  SymbolItem* insertNodeWithLlvmIrId(std::string* symbolName, SyntaxNode* node, SyntaxNodeType type, int id);

  void updateLlvmIrIdById(int oldId, int newId);

  int getLlvmIrId(std::string* symbolName);

  int getTableId() { return this->tableId; }

  /* ----------------------------- error handle -----------------------------*/
  /**
   * @brief 检查符号未定义错误
   * @param symbolName 符号名
   * @param line 符号所在行号
   * @return true 
   * @return false 
   */
  bool undefSymbolHandler(std::string* symbolName, int line);

  /**
   * @brief 函数参数个数不匹配 or 函数参数类型不匹配
   * @param funcName 函数名
   * @param funcCParams 函数实参表
   * @param line 函数名所在行号
   * @return true 
   * @return false 
   */
  bool funcCallErrorHandler(std::string* funcName, std::vector<ObjectSymbolItem*>* funcCParams, int line);

  /* 如果出错，返回NONE类型的Object */
  /**
   * @brief 根据函数名查找符号表返回函数返回类型
   * @param funcName 
   * @return ObjectSymbolItem* 
   */
  ObjectSymbolItem* getFuncReturnType(std::string* funcName);

  /**
   * @brief 计算左值节点类型
   * @param node 左值节点 
   * @return ObjectSymbolItem* 
   */
  ObjectSymbolItem* getLValType(LValNode* node);

  /**
   * @brief 计算Number的类型
   * @return ObjectSymbolItem* 
   */
  ObjectSymbolItem* getNumberType();

  /**
   * @brief 检查函数声明与函数定义中的返回语句是否匹配
   * @param funcName 函数名
   * @param hasReturnExp 函数定义中是否存在return Exp语句
   * @param line 函数块末尾'}'所在行号
   * @return true 
   * @return false 
   */
  bool funcReturnCheck(std::string* funcName, bool hasReturnExp, int line);

  /**
   * @brief 检查const类型变量是否被赋值
   * @param node 
   * @return true 
   * @return false 
   */
  bool constModifyCheck(LValNode* node);
  /* ----------------------------- error handle -----------------------------*/

  /**
   * @brief 打印符号表信息
   * @param tabRet 前导tab个数
   */
  void toString(int tabRet);

  private:

  /**
   * @brief 将符号对象插入符号表中，成功返回true，失败返回false
   * @param symbolName 
   * @param symbolItem 
   * @return true 
   * @return false 
   */
  bool insertSymbol(std::string* symbolName, SymbolItem* symbolItem);

  /**
   * @brief 根据符号名在符号表中寻找符号
   * @param symbolName 
   * @return SymbolItem* 
   */
  SymbolItem* findSymbol(std::string* symbolName);

  /**
   * @brief 将funcDefNode转换为FuncSymbolItem
   * @param node 
   * @return FuncSymbolItem* 
   */
  FuncSymbolItem* funcDefNode2FuncSymbolItem(SyntaxNode* node);
  
  /**
   * @brief 将abstVarDefNode转换为ObjectSymbolItem
   * @param node 
   * @return ObjectSymbolItem* 
   */
  ObjectSymbolItem* abstVarDefNode2ObjectSymbolItem(AbstVarDefNode* node);

  SymbolItem* findSymbolByLlvmIrId(int id);

};

class SymbolItem {
  public:
  SymbolType symbolType;
  int line;
  int llvmIrId;

  SymbolItem() : symbolType(SymbolType::NONE_ST), line(-1) {}

  SymbolItem(SymbolType type, int l) : symbolType(type), line(l), llvmIrId(NO_LLVMIRID) {}

};

class ObjectSymbolItem : public SymbolItem {
  public:
  bool isConst;
  int dimension;
  

  ObjectSymbolItem() : isConst(false), dimension(0) {
    SymbolItem();
  }
  /**
   * @brief 比较两个ObjectSymbolItem是否一致
   * 一致当且仅当维度相同 & const类型相同 & SymbolType一致
   * @param obj 
   * @return true 
   * @return false 
   */
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