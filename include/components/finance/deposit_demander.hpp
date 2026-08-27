#pragma once

#include <deque>
#include <optional>
#include <span>
#include <vector>

#include "components/common.hpp"
#include "core/util.hpp"
#include "core/values/common.hpp"
#include "core/values/others.hpp"
#include "world/deposit.hpp"

namespace abm::finance::deposit::demander {
class DepositDemander final {
  public:
    [[nodiscard]] explicit constexpr DepositDemander(RandomGenerator& masterRng) noexcept;

    template <AssetMinusFn F>
    void endStep(F&& assetMinus) noexcept {
        for (auto& account : depositAccounts_) {
            account.applyInterest(interestRate_);
            const auto minus = account.balance() * interestRate_;
            assetMinus(static_cast<Money>(minus));
        }
    }

  private:
    InterestRate               interestRate_;
    std::deque<DepositAccount> depositAccounts_;
};
}  // namespace abm::finance::deposit::demander

namespace abm {
using DepositDemander = finance::deposit::demander::DepositDemander;
}