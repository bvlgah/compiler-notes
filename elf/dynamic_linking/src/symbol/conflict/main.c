#include "api.h"

#include <stdio.h>

void greet(void) {
  printf("Hi, this is %s from %s\n", __func__, __FILE__);
}

int main(void) {
  greet();
  return 0;
}
