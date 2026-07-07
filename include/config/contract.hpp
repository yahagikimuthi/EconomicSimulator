#pragma once

#include <cassert>

#ifdef NDEBUG
#define ACCESS(container, index) ((container)[(index)])
#else

#define ACCESS(container, Index)                                 \
    ([&](auto&& c, std::size_t _macro_idx) -> decltype(auto) {   \
        assert(_macro_idx < c.size() && "Index out of bounds!"); \
        return c[_macro_idx];                                    \
    })(container, Index)

#endif