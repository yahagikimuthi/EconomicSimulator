#pragma once

#include <optional>
#include <vector>

#include "core/util.hpp"
#include "core/values/common.hpp"
#include "core/values/others.hpp"
#include "world/finance.hpp"

namespace abm::deposit::demander {
class DepositDemander {
    using Market  = DepositMarket;
    using Request = DepositRequest;

  public:
    [[nodiscard]] explicit constexpr DepositDemander(RandomGenerator& masterRng) noexcept;

    void postRequest(const AgentID id, Market& market) noexcept;

  private:
    std::optional<DepositRequest&> myRequest_;
    InterestRate                   interestRate_;
    std::vector<SavingAccount>     savingAccount_;
};
}  // namespace abm::deposit::demander

namespace abm {
using DepositDemander = deposit::demander::DepositDemander;
}