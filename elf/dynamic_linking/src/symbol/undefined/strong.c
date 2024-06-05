#include "api.h"

extern void non_existing(void);

int main(void) {
  non_existing();
  return 0;
}

void foo(void) {
  non_existing();
}
