#include "api.h"

#include <stdio.h>

static void print(void) {
  printf("function %s in %s\n", __func__, __FILE__);
}

int main(void) {
  print();
  greet();
  return 0;
}
