#include <stdio.h>

int getint()
{
  int a;
  scanf("%d", &a);
  return a;
}


void putstr(char* str)
{
  for (int i = 0; str[i] != 0; i++) {
    putc(str[i], stdout);
  }
}

void putint(int a)
{
  printf("%d", a);
}

void putch(int ch)
{
  printf("%c", ch);
}