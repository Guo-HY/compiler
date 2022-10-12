#include "error_handle.hpp"
#include <stdio.h>

ErrorList errorList;

void ErrorList::toString() {
    printf("error info\n");
    for(int i = 0; i < errorInfos.size(); i++) {
      printf("%d %c\n", errorInfos[i]->line, errorInfos[i]->errorType);
    }
  }