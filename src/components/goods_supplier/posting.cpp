#include "components/goods_supplier.hpp"

#include <tbb/concurrent_vector.h>
#include <cassert>
#include <cmath>
#include <pcg_random.hpp>

#include "config.hpp"
#include "helper.hpp"
#include "world/message.hpp"

namespace {
[[nodiscard]] auto markupGuard(const double markup) -> double {
    return std::max(markup, config::goods_supplier::epsilonMarkup);
}
[[nodiscard]] auto priceGuard(const double price) -> double {
    return std::max(price, config::goods_supplier::epsilonPrice);
}
}  // namespace

namespace goods_supplier {
auto Producer::product() const -> double { return workspace_.totalLaborInput + inventory_; }

auto Planner::calcMarkup() const -> double {
    const double alpha{std::abs(helper::randNormal(rng_, 0.0, param_.markupAdjustVol))};
    const double nextMarkup{log_.markup + (log_.isSold ? alpha : -alpha)};
    return markupGuard(nextMarkup);
}

auto Planner::judgePrice(const double supply, const double markup, const double totalCost) const
    -> double {
    const double avgCost{(supply != 0.0) ? totalCost / supply : 0.0};
    const double price{avgCost * (1.0 + markup)};
    return priceGuard(price);
}

void Planner::judgePlan(const double supply, const double totalCost) {
    const double markup{calcMarkup()};
    const double price{judgePrice(supply, markup, totalCost)};
    plan_ = {.markup = markup, .price = price, .supply = supply};
}

void Trader::post(
    const double supply, const double pricePlan, tbb::concurrent_vector<world::GoodsEntry>& entryBox
) {
    if (supply == 0.0) return;
    auto it{entryBox.emplace_back(pricePlan, supply)};
    myEntry_ = &*it;
}

void GoodsSupplier::post(
    const double totalCost, tbb::concurrent_vector<world::GoodsEntry>& entryBox
) {
    const double supply{producer_.product()};
    planner_.judgePlan(supply, totalCost);
    trader_.post(supply, planner_.pricePlan(), entryBox);
}
}  // namespace goods_supplier