#ifndef _ERROR_HANDLE_H
#define _ERROR_HANDLE_H 1

#include <vector>
class ErrorInfo;

class ErrorList {
  private:
  std::vector<ErrorInfo*> errorInfos;

  public:
  
  void addErrorInfo(ErrorInfo* info) {
    this->errorInfos.push_back(info);
  }

};

class ErrorInfo {
  public:
    int line;
    char errorType;

  ErrorInfo () : line(-1), errorType('#') {}

  ErrorInfo(int l, char e) : line(l), errorType(e) {}
};

#endif