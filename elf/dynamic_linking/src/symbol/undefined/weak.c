#include <stdio.h>

__attribute__((weak)) extern int a;

__attribute__((weak)) extern void non_existing(void);

int main(void) {
  printf("a = %d\n", a);
  non_existing();
  return 0;
}
