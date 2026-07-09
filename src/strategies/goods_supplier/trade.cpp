#include "strategies/goods_supplier.hpp"

#include <cassert>
#include <helper.hpp>
#include <numeric>
#include <random>

#include "config/contract.hpp"
#include "world/message.hpp"

namespace goods_supplier {
namespace {
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

void shuffleIdx(
    const std::size_t         resizeNum,
    std::vector<std::size_t>& shuffleVec,
    std::mt19937&             gen = helper::gen
) {
    shuffleVec.resize(resizeNum);
    std::ranges::iota(shuffleVec, 0UZ);
    std::ranges::shuffle(shuffleVec, gen);
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

    assert(false && "runtime error");
    return 1;
}

[[nodiscard]] auto performFullTrade(tbb::concurrent_vector<world::GoodsRequest>& requestBox)
    -> double {
    double salesAmount{};
    for (auto& request : requestBox) {
        request.tradeAmount_ = request.amount_;
        salesAmount += request.amount_;
    }

    assert(salesAmount >= 0.0 && "runtime error");
    return salesAmount;
}

void updateLog(TradeView& view, const double salesAmount) {
    const bool isSold{
        (view.supply() != 0.0) ? view.inventory() / view.supply() < view.getTargetInvRatio() : false
    };
    const double sales{salesAmount * view.price()};
    assert(sales >= 0.0 && "sales is required >= 0");
    view.updateLog(view.price(), sales, isSold);
}
}  // namespace
void trade(TradeView view) {
    auto         myEntry{view.getMyEntry()};
    auto&        requestBox = myEntry->requestBox_;
    const double totalDemand{calcTotalDemand(requestBox)};
    const bool   isExcessDemand{totalDemand >= view.supply()};
    const double salesAmount{
        isExcessDemand ? performRationedTrade(view.supply(), requestBox)
                       : performFullTrade(requestBox)
    };
    view.setInventory(view.supply() - salesAmount);
    updateLog(view, salesAmount);
}
}  // namespace goods_supplier