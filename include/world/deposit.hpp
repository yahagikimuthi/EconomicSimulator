#pragma once

#include <algorithm>
#include <cassert>

#include "values/common.hpp"

namespace abm {
class BankAccount final {
  public:
    explicit BankAccount(const AgentID id) noexcept : depositorId{id} {}

    const AgentID depositorId;

    void deposit(const Money add) noexcept {
        assert(add.isZeroOrMore());
        deposit_ += add;
    }

    [[nodiscard]] auto withdraw(const Money sub) noexcept -> Money {
        assert(sub.isZeroOrMore());
        const auto out = std::min(sub, deposit_);
        deposit_ -= out;
        return out;
    }

    [[nodiscard]] auto balance() const noexcept -> Budget { return static_cast<Budget>(deposit_); }

  private:
    Money deposit_{0.0};
};
}  // namespace abm