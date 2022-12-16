#include <stdio.h>

#include <execinfo.h>
#include <signal.h>

#include "frontend/lexer/lexer.hpp"
#include "frontend/parser/parser.hpp"
#include "frontend/parser/symbol_table.hpp"
#include "utils/error_handle.hpp"
#include "utils/test.hpp"
#include "include/error.hpp"
#include "ir/ir.hpp"
#include "ir/ir_build.hpp"
#include "machine/asm.hpp"
#include "machine/asm_build.hpp"
#include "machine/reg_allocator.hpp"
#include "ir/opt/opt.hpp"
#include "machine/opt/asm_opt.hpp"

extern SymbolTable* currentSymbolTable;
extern ErrorList errorList;

std::vector<std::string> funcDecls = {
  "declare i32 @getint()",
  "declare void @putint(i32)",
  "declare void @putch(i32)",
  "declare void @putstr(i8*)",
};

static void SignalHandle(int sig);

int main(int argc, char **argv)
{
  signal(SIGSEGV, SignalHandle);
  FILE* fp = fopen("testfile.txt", "r");
  bool ret;
  std::string s;
  if (fp == NULL) {
    panic("can't open source file");
  }

  Lexer* lexer = new Lexer(fp);
  ret = lexer->getAllToken();
  if (ret == false) {
    Log("lexer has something error\n");
    exit(1);
  }
  Parser* parser = new Parser();
  // parser->toString();
  parser->syntaxAnalyse();
  Log("after parser");
  // freopen("table.txt", "w", stdout);
  // currentSymbolTable->toString(0);

  // freopen("error.txt", "w", stdout);
  // errorList.toString();

  /* gen ir */
  Module* module = compUnit2ir(parser->getRoot(), funcDecls);
  Log("after genIr");
  
  /* ir opt */
  optir(module);
  freopen("output.ll", "w", stdout);
  s = module->toString();
  // printf("%s", s.c_str());

  /* gen asm */
  freopen("base.asm", "w", stdout);
  AsmModule* asmModule = module2asm(module);
  Log("after gen asm");
  s = asmModule->toString();
  // printf("%s", s.c_str());

  freopen("graph.asm", "w", stdout);
  graphRegAllocator(asmModule);
  Log("after graphRegAllocator");
  s = asmModule->toString();
  // printf("%s", s.c_str());

  freopen("mips.txt", "w", stdout);
  plainRegAllocator(asmModule);
  Log("after plainRegAllocator");
  s = asmModule->toString();
  printf("%s", s.c_str());

  return 0;
}


static void SignalHandle(int sig)
{
    void *array[20];
    size_t size;
    char **strings;
    size_t i;
    size = backtrace(array, 10);
    strings = backtrace_symbols(array, size);
    fprintf(stderr, "SIGNAL ocurre %d, stack tarce:\n", sig);
    fprintf(stderr, "obtained %ld stack frames.\n", size);
 
    for (i = 0; i < size; i++)
        fprintf(stderr, "%s\n", *strings);
 
    free(strings);
    fprintf(stderr, "stack trace over!\n");
    exit(0);
}
