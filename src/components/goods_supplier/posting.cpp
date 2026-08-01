#include "components/goods_supplier.hpp"

#include <tbb/concurrent_vector.h>
#include <cassert>
#include <cmath>
#include <core/values/common.hpp>
#include <pcg_random.hpp>

#include "config.hpp"
#include "core/values/goods.hpp"
#include "helper.hpp"
#include "world/message.hpp"

namespace {
[[nodiscard]] auto markupGuard(const double markup) -> double {
    return std::max(markup, config::goods_supplier::epsilonMarkup);
}
[[nodiscard]] auto priceGuard(const Price price) -> Price {
    return Price{std::max(price.value(), config::goods_supplier::epsilonPrice)};
}
}  // namespace

namespace goods_supplier {
auto Producer::product() const -> GoodsQuantity { return workspace_.totalInput() + inventory_; }

auto Planner::calcMarkup() const -> double {
    const double alpha{std::abs(helper::randNormal(rng_, 0.0, param_.markupAdjustVol))};
    const double nextMarkup{log_.markup + (log_.isSold ? alpha : -alpha)};
    return markupGuard(nextMarkup);
}

auto Planner::judgePrice(const GoodsQuantity supply, const double markup, const Money totalCost)
    const -> Price {
    const Money avgCost{(supply != GoodsQuantity{0.0}) ? totalCost.value() / supply.value() : 0.0};
    const Price price{avgCost.value() * (1.0 + markup)};
    return priceGuard(price);
}

void Planner::judgePlan(const GoodsQuantity supply, const Money totalCost) {
    const double markup{calcMarkup()};
    const Price  price{judgePrice(supply, markup, totalCost)};
    plan_ = {.markup = markup, .price = price, .supply = supply};
}

void Trader::post(
    const GoodsQuantity                        supply,
    const Price                                pricePlan,
    tbb::concurrent_vector<world::GoodsEntry>& entryBox
) {
    if (supply == GoodsQuantity{0.0}) return;
    auto it{entryBox.emplace_back(pricePlan, supply)};
    myEntry_ = &*it;
}

void GoodsSupplier::post(
    const Money totalCost, tbb::concurrent_vector<world::GoodsEntry>& entryBox
) {
    const GoodsQuantity supply{producer_.product()};
    planner_.judgePlan(supply, totalCost);
    trader_.post(supply, planner_.pricePlan(), entryBox);
}
}  // namespace goods_supplier