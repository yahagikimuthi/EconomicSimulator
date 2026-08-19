#pragma once

#include <tbb/concurrent_vector.h>
#include <optional>
#include <pcg_random.hpp>

#include "config.hpp"
#include "core/values/common.hpp"
#include "core/values/goods.hpp"
#include "util.hpp"
#include "world/goods.hpp"

namespace abm {
class ProductionGoodsDemander final {
    using Request = ProductionGoodsRequest;
    using Market  = ProductionGoodsMarket;

  public:
    [[nodiscard]] explicit ProductionGoodsDemander(RandomGenerator& masterRng) noexcept;

    void request(
        const AgentID id,
        const Money   asset,
        Market&       market,
        const int     sampleCnt = config::goods_demander::goodsSampleCnt
    ) noexcept {
        const auto budget = asset * mpc_;
        if (budget <= Money{0.0}) return;
        const auto pickedEntry = market.pickEntry(id, sampleCnt);
        if (not pickedEntry) return;
        myRequest_ = pickedEntry->request(budget / pickedEntry->price);
    }

    void afterTrade() noexcept {}

    void endStep() noexcept {}

  private:
    std::optional<const Request&> myRequest_{std::nullopt};
    const double                  mpc_;
};
}  // namespace abm