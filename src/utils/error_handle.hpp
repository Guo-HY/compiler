#ifndef _ERROR_HANDLE_H
#define _ERROR_HANDLE_H 1

#include <vector>
#include <map>
#include "../include/error.hpp"

class ErrorInfo {
  public:
    int line;
    char errorType;

  ErrorInfo () : line(-1), errorType('#') {}

  ErrorInfo(int l, char e) : line(l), errorType(e) {}
};

class ErrorList {
  private:
  std::map<int, ErrorInfo*> errorInfos;

  public:
  void addErrorInfo(ErrorInfo* info) {
    //Log("in addErrorInfo\n");
    this->errorInfos.emplace(info->line, info);
  }

  void toString();
  
  bool hasError() {
    return this->errorInfos.size() > 0;
  }

};



#endif