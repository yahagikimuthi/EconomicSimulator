#pragma once

#include <iostream>

#ifdef NDEBUG
#define REQUIRE(cond, msg) ((void)0)
#define ENSURE(cond, msg) ((void)0)
#define ACCESS(container, index) ((container)[(index)])
#else
// 違反時に情報を詳細に出力してプログラムを止める
#define REQUIRE(cond, msg)                                                  \
    if (!(cond)) {                                                          \
        std::cerr << "[Contract Violation: REQUIRE]" << #cond << '\n'       \
                  << "  Message: " << (msg) << '\n'                         \
                  << "  Location: " << __FILE__ << ":" << __LINE__ << '\n'; \
        std::abort();                                                       \
    }

#define ENSURE(cond, msg)                                                   \
    if (!(cond)) {                                                          \
        std::cerr << "[Contract Violation: ENSURE]" << #cond << '\n'        \
                  << "  Message: " << (msg) << '\n'                         \
                  << "  Location: " << __FILE__ << ":" << __LINE__ << '\n'; \
        std::abort();                                                       \
    }

#define ASSERT(cond, msg)                                                 \
    if (!(cond)) {                                                        \
        std::cerr << "[Contract Violation: ASSERT]: " << '\n'             \
                  << "Expression: " << #cond << '\n'                      \
                  << "Message: " << (msg) << '\n'                         \
                  << "Location: " << __FILE__ << ":" << __LINE__ << '\n'; \
        std::abort();                                                     \
    }

#define ACCESS(container, Index)                               \
    ([&](auto&& c, std::size_t _macro_idx) -> decltype(auto) { \
        ASSERT(_macro_idx < c.size(), "Index out of bounds!"); \
        return c[_macro_idx];                                  \
    })(container, Index)

#endif