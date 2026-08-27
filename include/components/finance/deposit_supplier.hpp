#pragma once

#include <optional>

#include "core/values/common.hpp"
#include "core/values/others.hpp"
#include "world/deposit.hpp"

namespace abm::deposit::supplier {

class DepositSupplier final {
    using Entry   = DepositEntry;
    using Request = DepositRequest;
    using Market  = DepositMarket;

  public:
    [[nodiscard]] constexpr DepositSupplier() noexcept = default;

    [[nodiscard]] auto balance() const noexcept -> Deposit {
        return account_.transform(&DepositAccount::balance).value_or(Deposit{0.0});
    }

    void setAccount(DepositAccount& account) noexcept { account_ = account; }

  private:
    [[nodiscard]] auto haveAccount() const noexcept -> bool { return account_.has_value(); }

    std::optional<DepositAccount&> account_{std::nullopt};
};
}  // namespace abm::deposit::supplier

namespace abm {
using DepositSupplier = deposit::supplier::DepositSupplier;
}