#pragma once

#include <optional>

#include "core/values/common.hpp"
#include "core/values/others.hpp"
#include "world/deposit.hpp"

namespace abm::finance::deposit::supplier {

class DepositSupplier final {
  public:
    [[nodiscard]] constexpr DepositSupplier() noexcept = default;

    [[nodiscard]] auto balance() const noexcept -> Deposit {
        return account_.transform(&DepositAccount::balance).value_or(Deposit{0.0});
    }

    [[nodiscard]] auto tryDeposit(const Money deposit) noexcept -> bool {
        ASSERT(deposit >= Money{0.0});
        if (not haveAccount()) return false;
        account_->deposit(deposit);
        return true;
    }

    [[nodiscard]] auto tryWithdraw(const Money withdraw) noexcept -> Money {
        ASSERT(withdraw >= Money{0.0});
        if (not haveAccount()) return Money{0.0};
        return account_->withdraw(withdraw);
    }

    void setAccount(DepositAccount& account) noexcept { account_ = account; }

  private:
    [[nodiscard]] auto haveAccount() const noexcept -> bool { return account_.has_value(); }

    std::optional<DepositAccount&> account_{std::nullopt};
};
}  // namespace abm::finance::deposit::supplier

namespace abm::finance {
using DepositSupplier = finance::deposit::supplier::DepositSupplier;
}