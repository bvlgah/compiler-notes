#include "api.h"

#include <stdio.h>

void greet(void) {
  printf("Hello! This is %s of %s\n", __func__, __FILE__);
}
