#include "error_handle.hpp"
#include <stdio.h>

ErrorList errorList;

void ErrorList::toString() {
    // printf("error info\n");
    std::map<int, ErrorInfo*>::iterator iter;
    for(iter = this->errorInfos.begin(); iter != errorInfos.end(); iter++) {
      printf("%d %c\n", iter->second->line, iter->second->errorType);
    }
  }