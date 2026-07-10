#pragma once

#include <tbb/concurrent_vector.h>
#include <tuple>

#include "components/goods_demander.hpp"
#include "config/init_setup.hpp"
#include "config/sim_vars.hpp"
#include "core/forward.hpp"

namespace goods_demander {
struct PurchaseView {
    explicit PurchaseView(Component& comp) : comp_{comp} {}

    [[nodiscard]] auto myPhase() const -> int { return comp_.parameter_.myPhase_; }

    void isPosting(const bool isPosting) { comp_.posting_.isPosting_ = isPosting; }

    [[nodiscard]] auto purchasing() const -> double { return comp_.purchasing_.purchase_; }

    [[nodiscard]] auto mpc() const -> double { return comp_.parameter_.mpc_; }

    void entry(
        const world::GoodsEntry&                                    entry,
        const tbb::concurrent_vector<world::GoodsRequest>::iterator myRequest  // NOLINT
    ) {
        comp_.posting_.myRequest_ = {&entry, &*myRequest};
    }

  private:
    Component& comp_;
};

void purchase(
    PurchaseView                               view,
    const double                               availableAsset,
    tbb::concurrent_vector<world::GoodsEntry>& entryBox,
    const int  maxPurchaseFrequency = config::goods_demander::maxPurchaseFrequency,
    const int& step                 = config::currentStep
);

struct AfterTradeView {
    explicit AfterTradeView(Component& comp) : comp_{comp} {}

    [[nodiscard]] auto myRequest() const
        -> std::pair<const world::GoodsEntry&, const world::GoodsRequest&> {
        auto& [entry, myRequest] = comp_.posting_.myRequest_;
        return {*entry, *myRequest};
    }
    void purchasePlus(const double plus) { comp_.purchasing_.purchase_ += plus; }

    [[nodiscard]] auto isPosting() const -> bool { return comp_.posting_.isPosting_; }

  private:
    Component& comp_;
};

void afterTrade(AfterTradeView view);
void reset(Component& comp);
}  // namespace goods_demander