#pragma once

#include <algorithm>
#include <atomic>

#include "others/util.hpp"
#include "values/common.hpp"

namespace abm {
class BankAccount final {
  public:
    explicit BankAccount(const AgentID id) noexcept : depositorId{id} {}

    const AgentID depositorId;

    void deposit(const Money add) noexcept {
        ASSERT(add.isZeroOrMore());
        deposit_.fetch_add(add.value());
    }

    [[nodiscard]] auto withdraw(const Money sub) noexcept -> Money {
        ASSERT(sub.isPositive());
        const auto out = std::min(sub, Money{deposit_.load()});
        deposit_.fetch_sub(out.value());
        return out;
    }

    [[nodiscard]] auto balance() const noexcept -> Money { return Money{deposit_.load()}; }

  private:
    std::atomic<double> deposit_;
};

class DepositAccount {
  public:
    explicit DepositAccount(BankAccount& account) noexcept : account_{account} {}

    void deposit(const Money add) noexcept { account_.deposit(add); }

  private:
    BankAccount& account_;
};
}  // namespace abm