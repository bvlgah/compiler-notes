#pragma once

#ifndef NDEBUG
#include <iostream>
#define DEBUG_MESSAGE(Message)                                         \
  do {                                                                \
    std::cerr << Message << std::endl;                                \
  } while(0)
#else
#define DEBUG_MESSAGE(Message) do { ; } while (0)
#endif
