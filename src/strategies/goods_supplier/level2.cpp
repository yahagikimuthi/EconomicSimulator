#include "strategies/goods_supplier/level2.hpp"

#include <tbb/concurrent_vector.h>
#include <cmath>
#include <numeric>

#include "config/contract.hpp"
#include "helper.hpp"
#include "strategies/goods_supplier/level3.hpp"
#include "world/message.hpp"

namespace goods_supplier {
[[nodiscard]] auto calcSupply(const CalcSupplyCtx ctx) -> double {
    return (ctx.firmProductPower() * ctx.sumEmployeeProductPower()) + ctx.inventory();
}

[[nodiscard]] auto calcMarkup(const CalcMarkupCtx ctx, const double epsilonMarkup) -> double {
    const double alpha{std::abs(helper::randNormal(0.0, ctx.markupAdjustVol(), -1.0, 1.0))};
    const double nextMarkup{ctx.lastMarkup() * (ctx.isSold() ? 1.0 + alpha : 1.0 - alpha)};
    return std::max(epsilonMarkup, nextMarkup);
}

[[nodiscard]] auto judgePrice(
    const double markup, const double totalCost, const double epsilonPrice
) -> double {
    const double price{totalCost * markup};
    return std::max(epsilonPrice, price);
}

[[nodiscard]] auto calcTotalDemand(const tbb::concurrent_vector<world::GoodsRequest>& requestBox)
    -> double {
    return std::accumulate(
        requestBox.begin(),
        requestBox.end(),
        0.0,
        [](const double sum, const world::GoodsRequest& request) -> double {
            return sum + request.amount_;
        }
    );
}

[[nodiscard]] auto performRationedTrade(
    const double supply, tbb::concurrent_vector<world::GoodsRequest>& requestBox
) -> double {
    static thread_local std::vector<std::size_t> consumerIdxs;
    shuffleIdx(requestBox.size(), consumerIdxs);

    double remainAmount{supply};
    for (const std::size_t i : consumerIdxs) {
        auto&        request = ACCESS(requestBox, i);
        const double requestAmount{request.amount_};
        if (remainAmount <= requestAmount) {
            request.tradeAmount_ = remainAmount;
            return supply;
        }
        request.tradeAmount_ = requestAmount;
        remainAmount -= requestAmount;
    }

    ASSERT(false, "runtime error");
    return 1;
}

[[nodiscard]] auto performFullTrade(tbb::concurrent_vector<world::GoodsRequest>& requestBox)
    -> double {
    double salesAmount{};
    for (auto& request : requestBox) {
        request.tradeAmount_ = request.amount_;
        salesAmount += request.amount_;
    }

    ENSURE(salesAmount >= 0.0, "runtime error");
    return salesAmount;
}

void updateLog(UpdateLogCtx ctx, const double salesAmount) {
    const bool   isSold{ctx.inventory() / ctx.supply() < ctx.getTargetInvRatio()};
    const double sales{salesAmount * ctx.price()};
    ctx.updateLog(ctx.price(), sales, isSold);
}
}  // namespace goods_supplier