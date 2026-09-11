#include "others/util.hpp"

#include "doctest.h"

namespace abm {
namespace {
TEST_CASE("ScopeExitのテスト") {  // NOLINT
    auto x = 10;
    {
        auto _ = makeScopeExit([&x]() noexcept -> void { x = 0; });
    }
    CHECK(x == 0);
}
}  // namespace
}  // namespace abm