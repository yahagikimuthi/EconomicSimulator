#pragma once

#include "values/common.hpp"
#include "world/deposit.hpp"

namespace abm::finance::deposit::supplier {

class DepositSupplier final {
  public:
    explicit DepositSupplier(const AgentID id) noexcept : account_{id} {}

    [[nodiscard]] auto balance() const noexcept -> Budget {
        return static_cast<Budget>(account_.balance());
    }

    void deposit(const Money deposit) noexcept {
        ASSERT(deposit >= Money{0.0});
        account_.deposit(deposit);
    }

    [[nodiscard]] auto tryWithdraw(const Money withdraw) noexcept -> Money {
        ASSERT(withdraw >= Money{0.0});
        return account_.withdraw(withdraw);
    }

  private:
    BankAccount account_;
};
}  // namespace abm::finance::deposit::supplier

namespace abm::finance {
using DepositSupplier = finance::deposit::supplier::DepositSupplier;
}