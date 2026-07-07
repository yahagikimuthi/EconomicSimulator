#include "strategies/goods_supplier/level2.hpp"

#include <tbb/concurrent_vector.h>
#include <cassert>
#include <cmath>
#include <numeric>

#include "config/contract.hpp"
#include "helper.hpp"
#include "strategies/goods_supplier/level1.hpp"
#include "strategies/goods_supplier/level3.hpp"
#include "world/message.hpp"

namespace goods_supplier {
[[nodiscard]] auto calcSupply(const PostGoodsView& view) -> double {
    return (view.firmProductPower() * view.sumEmployeeProductPower()) + view.inventory();
}

[[nodiscard]] auto calcMarkup(const PostGoodsView& view, const double epsilonMarkup) -> double {
    const double alpha{std::abs(helper::randNormal(0.0, view.markupAdjustVol(), -1.0, 1.0))};
    const double nextMarkup{view.lastMarkup() * (view.isSold() ? 1.0 + alpha : 1.0 - alpha)};
    return std::max(epsilonMarkup, nextMarkup);
}

[[nodiscard]] auto judgePrice(
    const double markup, const double totalCost, const double epsilonPrice
) -> double {
    assert(markup > 0.0 && "markup is required > 0");
    assert(totalCost > 0.0 && "total cost is required > 0");
    const double price{totalCost * markup};
    return std::max(epsilonPrice, price);
}

[[nodiscard]] auto calcTotalDemand(const tbb::concurrent_vector<world::GoodsRequest>& requestBox)
    -> double {
    const double demand{std::accumulate(
        requestBox.begin(),
        requestBox.end(),
        0.0,
        [](const double sum, const world::GoodsRequest& request) -> double {
            return sum + request.amount_;
        }
    )};
    assert(demand >= 0.0 && "total demand is required >= 0");
    return demand;
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

void updateLog(UpdateLogView view, const double salesAmount) {
    const bool isSold{
        (view.supply() != 0.0) ? view.inventory() / view.supply() < view.getTargetInvRatio() : false
    };
    const double sales{salesAmount * view.price()};
    assert(sales >= 0.0 && "sales is required >= 0");
    view.updateLog(view.price(), sales, isSold);
}
}  // namespace goods_supplier