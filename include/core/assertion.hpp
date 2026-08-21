#pragma once

#include <cassert>

#ifdef __clang__
#define ASSERT(...) assert(__VA_ARGS__)
#else
#define ASSERT(...) contract_assert(__VA_ARGS__)
#endif