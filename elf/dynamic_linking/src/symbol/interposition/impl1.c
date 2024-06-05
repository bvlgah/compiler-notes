#include "api.h"

#include <stdio.h>

void greet(void) {
  printf("Hello, this is %s from %s\n", __func__, __FILE__);
}
