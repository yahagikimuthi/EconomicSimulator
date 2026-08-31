#pragma once

#include <tbb/concurrent_vector.h>
#include <algorithm>

#include "others/util.hpp"
#include "values/common.hpp"
#include "values/others.hpp"

namespace abm::finance::deposit {
class DepositAccount final {
  public:
    [[nodiscard]] explicit DepositAccount(const AgentID id) noexcept : depositorId{id} {}

    const AgentID depositorId;

    void applyInterest(const InterestRate rate) noexcept {
        ASSERT(InterestRate{0.0} < rate and rate < InterestRate{1.0});
        deposit_ += deposit_ * (InterestRate{1.0} + rate);
    }

    void deposit(const Money add) noexcept {
        ASSERT(add >= Money{0.0});
        deposit_ += static_cast<Deposit>(add);
    }

    [[nodiscard]] auto withdraw(const Money sub) noexcept -> Money {
        ASSERT(sub >= Money{0.0});
        const auto out = Deposit{std::min(sub.value(), deposit_.value())};
        deposit_ -= out;
        return out;
    }

    [[nodiscard]] auto balance() const noexcept -> Deposit { return deposit_; }

  private:
    Deposit deposit_{0.0};
};
}  // namespace abm::finance::deposit