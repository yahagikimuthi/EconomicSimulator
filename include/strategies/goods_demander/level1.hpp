#pragma once

#include "components/goods_demander.hpp"
#include "config/init_setup.hpp"
#include "config/sim_vars.hpp"
#include "world/message.hpp"

namespace goods_demander {
struct PurchaseView {
    PurchaseView(Component& comp) : comp_{comp} {}

    [[nodiscard]] auto myPhase() const -> int { return comp_.parameter_.myPhase_; }

    void isPosting(const bool isPosting) { comp_.posting_.isPosting_ = isPosting; }

    [[nodiscard]] auto mpc() const -> double { return comp_.parameter_.mpc_; }

    void entry(
        const world::GoodsEntry&                                    entry,
        const tbb::concurrent_vector<world::GoodsRequest>::iterator myRequest  // NOLINT
    ) {
        comp_.posting_.myRequest_ = {&entry, myRequest};
    }

  private:
    Component& comp_;
};

void purchase(
    PurchaseView                               view,
    const double                               availableAsset,
    tbb::concurrent_vector<world::GoodsEntry>& entryBox,
    const int  maxPurchaseFrequency = config::goods_demander::maxPurchaseFrequency,
    const int& step                 = config::sim_vars::currentStep
);

struct AfterTradeView {
    AfterTradeView(Component& comp) : comp_{comp} {}

    [[nodiscard]] auto getMyRequest() const
        -> std::pair<const world::GoodsEntry&, const world::GoodsRequest&> {
        auto& [entry, myRequest] = comp_.posting_.myRequest_;
        return {*entry, *myRequest};
    }
    void recordPurchase(const double purchase) { comp_.purchasing_.purchase_ = purchase; }

    [[nodiscard]] auto isPosting() const -> bool { return comp_.posting_.isPosting_; }

  private:
    Component& comp_;
};

void afterTrade(AfterTradeView view);
}  // namespace goods_demander