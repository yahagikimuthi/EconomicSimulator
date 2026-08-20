#pragma once

#include <cassert>

#ifdef __clang__
#define PRE(...)
#define POST(...)
#define ASSERT(...) assert(__VA_ARGS__)
#else
#define PRE(...) pre(__VA_ARGS__)
#define POST(...) post(__VA_ARGS__)
#define ASSERT(...) contract_assert(__VA_ARGS__)
#endif