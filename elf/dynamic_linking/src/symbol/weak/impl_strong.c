#include "api.h"

#include <stdio.h>

void greet() {
  printf("Hi, I'm strong %s from %s\n", __func__, __FILE__);
}
