#pragma once

#include <tbb/concurrent_vector.h>
#include <tuple>

#include "components/goods_demander.hpp"
#include "config.hpp"
#include "core/base.hpp"
#include "core/forward.hpp"

namespace goods_demander {
struct [[nodiscard]] PurchaseView final : BaseView<Component> {
    using BaseView<Component>::BaseView;

    auto myPhase() const -> int { return comp_.parameter_.myPhase_; }
    void isPosting(const bool isPosting) { comp_.posting_.isPosting_ = isPosting; }
    auto purchasing() const -> double { return comp_.purchasing_.purchase_; }
    auto mpc() const -> double { return comp_.parameter_.mpc_; }
    void entry(
        const world::GoodsEntry&                                    entry,
        const tbb::concurrent_vector<world::GoodsRequest>::iterator myRequest  // NOLINT
    ) {
        comp_.posting_.myRequest_ = {&entry, &*myRequest};
    }
};

void purchase(
    PurchaseView                               view,
    const double                               availableAsset,
    tbb::concurrent_vector<world::GoodsEntry>& entryBox,
    const int  maxPurchaseFrequency = config::goods_demander::maxPurchaseFrequency,
    const int& step                 = config::currentStep
);

struct [[nodiscard]] AfterTradeView final : BaseView<Component> {
    using BaseView<Component>::BaseView;

    auto myRequest() const -> std::pair<const world::GoodsEntry&, const world::GoodsRequest&> {
        auto& [entry, myRequest] = comp_.posting_.myRequest_;
        return {*entry, *myRequest};
    }
    void purchasePlus(const double plus) { comp_.purchasing_.purchase_ += plus; }
    auto isPosting() const -> bool { return comp_.posting_.isPosting_; }
};

void afterTrade(AfterTradeView view);
void reset(Component& comp);
}  // namespace goods_demander