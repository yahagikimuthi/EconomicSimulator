#pragma once

#include <algorithm>
#include <limits>
#include <optional>

#include "components/finance/firm_finance.hpp"
#include "others/setting.hpp"
#include "others/util.hpp"
#include "values/common.hpp"
#include "values/goods.hpp"
#include "world/base_goods.hpp"

namespace abm::capital::demander {

struct Log final {
    const Money         purchase{std::numeric_limits<double>::epsilon()};
    const GoodsQuantity tradeAmount{std::numeric_limits<double>::infinity()};
};

class CapitalDemander final {
  public:
    explicit CapitalDemander(RandomGenerator& masterRng) noexcept
        : rng_{{masterRng.makeUint64(), masterRng.makeUint64()}} {}

    [[nodiscard]] auto planAndRequestBudget(const GoodsQuantity desiredAmount) noexcept -> Budget {
        const auto avgPrice = log_.purchase / log_.tradeAmount;
        purchaseAmountPlan_ = desiredAmount;
        budget_             = static_cast<Budget>(avgPrice * desiredAmount);
        return *budget_;
    }

    void revisePlan(const Budget budget) noexcept { budget_ = budget; }

    void plan(const GoodsQuantity desiredAmount) noexcept { purchaseAmountPlan_ = desiredAmount; }

    void request(
        const AgentID id,
        FirmFinance&  finance,
        Market&       market,
        const int     sampleCnt = setting::goodsSampleCnt
    ) noexcept {
        const auto pickedEntry = market.pickEntry(id, sampleCnt, rng_);
        if (not pickedEntry) return;
        const auto payment =
            std::min(static_cast<Budget>(*purchaseAmountPlan_ * pickedEntry->price), *budget_);
        const auto withdraw =
            finance.tryWithdraw(payment, FirmFinance::AccountItem::CapitalGoodsCost);
        myRequest_ = pickedEntry->request(withdraw);
    }

    void afterTrade(FirmFinance& finance) noexcept {
        if (not myRequest_) return;
        const auto remain = myRequest_->remainPaid();
        finance.assetPlus(remain, FirmFinance::AccountItem::CapitalGoodsCost);
    }

  private:
    RandomGenerator               rng_;
    Log                           log_;
    std::optional<GoodsQuantity>  purchaseAmountPlan_{std::nullopt};
    std::optional<Budget>         budget_{std::nullopt};
    std::optional<const Request&> myRequest_{std::nullopt};
};
}  // namespace abm::capital::demander

namespace abm {
using CapitalDemander = capital::demander::CapitalDemander;
}