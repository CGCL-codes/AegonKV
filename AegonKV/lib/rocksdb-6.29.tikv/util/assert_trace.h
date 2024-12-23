//
// Created by 冯昊 on 2023/10/20.
//

#ifndef TITAN_ASSERT_TRACE_H
#define TITAN_ASSERT_TRACE_H

#include <execinfo.h>

#include <cassert>
#include <cstdio>
#include <cstdlib>

namespace {
#define assert_traceback(X) \
  if (!(X)) {               \
    DumpTraceback();        \
    assert(false);          \
  }

void DumpTraceback() {
  const int size = 200;
  void *buffer[size];
  char **strings;
  int nptrs = backtrace(buffer, size);
  printf("backtrace() returned %d address\n", nptrs);
  // backtrace_symbols函数不可重入， 可以使用backtrace_symbols_fd替换
  strings = backtrace_symbols(buffer, nptrs);
  if (strings) {
    for (int i = 0; i < nptrs; ++i) {
      printf("%s\n", strings[i]);
    }
    free(strings);
  }
}
}  // namespace

#endif  // TITAN_ASSERT_TRACE_H
