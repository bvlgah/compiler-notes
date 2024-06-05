#include "api.h"

#include <stdio.h>

__attribute__((weak)) extern void foo(void);

void greet(void) {
  printf("Hi! This is %s from %s\n", __func__, __FILE__);
}

int main(void) {
  greet();
  println("Hi!");
  foo();
  return 0;
}
