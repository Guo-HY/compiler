#ifndef _ERROR_HANDLE_H
#define _ERROR_HANDLE_H 1

#include <vector>
#include "../include/error.hpp"
class ErrorInfo;

class ErrorList {
  private:
  std::vector<ErrorInfo*> errorInfos;

  public:
  
  void addErrorInfo(ErrorInfo* info) {
    Log("in addErrorInfo\n");
    this->errorInfos.push_back(info);
  }

  void toString();

};

class ErrorInfo {
  public:
    int line;
    char errorType;

  ErrorInfo () : line(-1), errorType('#') {}

  ErrorInfo(int l, char e) : line(l), errorType(e) {}
};

#endif