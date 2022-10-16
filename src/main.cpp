#include <stdio.h>

#include "frontend/lexer/lexer.hpp"
#include "frontend/parser/parser.hpp"
#include "frontend/parser/symbol_table.hpp"
#include "utils/error_handle.hpp"
#include "utils/test.hpp"
#include "include/error.hpp"

extern SymbolTable* currentSymbolTable;
extern ErrorList errorList;

int main(int argc, char **argv)
{
  FILE* fp = fopen("testfile.txt", "r");
  freopen("output.txt", "w", stdout);
  bool ret;
  if (fp == NULL) {
    panic("can't open source file");
  }

  Lexer*lexer = new Lexer(fp);
  ret = lexer->getAllToken();
  if (ret == false) {
    Log("lexer has something error\n");
    exit(1);
  }

  Parser* parser = new Parser();
  parser->toString();
  Log("after parser");
  // freopen("table.txt", "w", stdout);
  // currentSymbolTable->toString(0);

  freopen("error.txt", "w", stdout);
  errorList.toString();

  return 0;
}

