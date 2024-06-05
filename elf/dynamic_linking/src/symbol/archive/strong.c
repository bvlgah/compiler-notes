#include <stdio.h>

extern void foo(void) {
  puts(__func__);
}
