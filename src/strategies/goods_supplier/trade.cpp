#include "strategies/goods_supplier.hpp"

#include <tbb/concurrent_vector.h>
#include <algorithm>
#include <cassert>
#include <cstddef>
#include <helper.hpp>
#include <numeric>
#include <random>
#include <ranges>

#include "world/message.hpp"

namespace goods_supplier {
namespace {
[[nodiscard]] auto calcTotalDemand(const tbb::concurrent_vector<world::GoodsRequest>& requestBox)
    -> double {
    const double demand{std::ranges::fold_left(
        requestBox | std::ranges::views::transform(&world::GoodsRequest::amount_),
        0.0,
        std::plus<>()
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

void performRationedTrade(
    const double supply, tbb::concurrent_vector<world::GoodsRequest>& requestBox
) {
    static thread_local std::vector<std::size_t> consumerIdxs;
    shuffleIdx(requestBox.size(), consumerIdxs);

    double remainAmount{supply};
    for (const std::size_t i : consumerIdxs) {
        auto&        request = requestBox[i];
        const double requestAmount{request.amount_};
        if (remainAmount <= requestAmount) {
            request.tradeAmount_ = remainAmount;
            return;
        }
        request.tradeAmount_ = requestAmount;
        remainAmount -= requestAmount;
    }

    assert(false && "runtime error");
}

void performFullTrade(tbb::concurrent_vector<world::GoodsRequest>& requestBox) {
    for (auto& request : requestBox) {
        request.tradeAmount_ = request.amount_;
    }
}
}  // namespace

void trade(TradeView view) {
    if (not view.isPosting()) return;
    auto& myEntry    = view.myEntry();
    auto& requestBox = myEntry.requestBox_;

    assert(myEntry.price_ == view.price() && "price is different");
    assert(myEntry.supply_ <= view.inventory());

    const double totalDemand{calcTotalDemand(requestBox)};
    const bool   isExcessDemand{totalDemand >= view.inventory()};
    const double salesAmount{std::min(view.inventory(), totalDemand)};
    isExcessDemand ? performRationedTrade(view.inventory(), requestBox)
                   : performFullTrade(requestBox);
    view.inventoryMinus(salesAmount);
    view.salesPlus(salesAmount + view.price());
}
}  // namespace goods_supplier