#include "api.h"

#include <stdio.h>

static void print(void) {
  printf("function %s in %s\n", __func__, __FILE__);
}

void greet() {
  print();
}
