#pragma once

#include "core/util.hpp"

namespace abm::bank {
class Bank {
  public:
    [[nodiscard]] explicit constexpr Bank(RandomGenerator& masterRng) noexcept;

  private:
};
}  // namespace abm::bank

namespace abm {
using Bank = bank::Bank;
}