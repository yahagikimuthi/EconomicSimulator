#include "components/common.hpp"

#include "doctest.h"
#include "values/common.hpp"

namespace abm {
namespace {
class MyClass1 final {
  public:
    void listen(const Money listenAmount) noexcept { money = listenAmount; }

    Money money{0.0};
};

class MyClass12 final {
  public:
    void listen(const Money listenAmount) noexcept { money = listenAmount; }

    Money money{0.0};
};

TEST_CASE("Listenerのテスト") {  // NOLINT
    auto listener = Listener<MyClass1, MyClass12>{};

    auto c1 = MyClass1{};
    auto c2 = MyClass12{};

    listener.add(c1);
    listener.add(c2);

    listener.notice([](auto& arg) noexcept -> void { arg.listen(Money{10.0}); });

    CHECK(c1.money.value() == 10.0);
    CHECK(c2.money.value() == 10.0);
}
}  // namespace
}  // namespace abm