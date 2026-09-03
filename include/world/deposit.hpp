#pragma once

#include <algorithm>

#include "others/util.hpp"
#include "values/common.hpp"

namespace abm {
class BankAccount final {
  public:
    explicit BankAccount(const AgentID id) noexcept : depositorId{id} {}

    const AgentID depositorId;

    void deposit(const Money add) noexcept {
        ASSERT(add.isZeroOrMore());
        deposit_ += add;
    }

    [[nodiscard]] auto withdraw(const Money sub) noexcept -> Money {
        ASSERT(sub.isPositive());
        const auto out = std::min(sub, deposit_);
        deposit_ -= out;
        return out;
    }

    [[nodiscard]] auto balance() const noexcept -> Money { return deposit_; }

  private:
    Money deposit_{0.0};
};

class DepositAccount {
  public:
    explicit DepositAccount(BankAccount& account) noexcept : account_{account} {}

    void deposit(const Money add) noexcept { account_.deposit(add); }

  private:
    BankAccount& account_;
};
}  // namespace abm