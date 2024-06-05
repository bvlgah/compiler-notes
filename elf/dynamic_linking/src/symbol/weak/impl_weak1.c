#include "api.h"

#include <stdio.h>

WEAK void greet() {
  printf("Hi, I'm weak %s from %s\n", __func__, __FILE__);
}
