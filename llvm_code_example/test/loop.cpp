int runLoop(unsigned X, unsigned Y, unsigned Z) {
  int Count = 0;

  for (unsigned I = 0; I < X; ++I) {
    for (unsigned J = 0; J < Y; ++J) {
      for (unsigned K = 0; K < Z; ++K) {
        ++Count;
      }
    }

    for (unsigned J = 0; J < Y; ++J)
      ++Count;
  }

  for (unsigned K = 0; K < Z; ++K)
    ++Count;

  return Count;
}
