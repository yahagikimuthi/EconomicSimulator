#pragma once

#include "values/common.hpp"
#include "world/deposit.hpp"

namespace abm::finance::deposit::supplier {

class DepositSupplier final {
  public:
    DepositSupplier(const AgentID id) noexcept;

    [[nodiscard]] auto balance() const noexcept -> Money { return account_.balance(); }

    void deposit(const Money deposit) noexcept {
        ASSERT(deposit >= Money{0.0});
        account_.deposit(deposit);
    }

    [[nodiscard]] auto tryWithdraw(const Money withdraw) noexcept -> Money {
        ASSERT(withdraw >= Money{0.0});
        return account_.withdraw(withdraw);
    }

  private:
    DepositAccount account_;
};
}  // namespace abm::finance::deposit::supplier

namespace abm::finance {
using DepositSupplier = finance::deposit::supplier::DepositSupplier;
}