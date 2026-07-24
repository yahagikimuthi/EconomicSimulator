#pragma once

#include <tbb/concurrent_vector.h>

#include "components/goods_demander.hpp"
#include "core/base.hpp"
#include "world/message.hpp"

namespace goods_demander {
struct [[nodiscard]] PurchaseView final : BaseView<Component> {
    using BaseView<Component>::BaseView;

    auto myPhase() const -> int { return comp_.parameter_.myPhase_; }
    void isPosting(const bool isPosting) { comp_.posting_.isPosting_ = isPosting; }
    auto purchasing() const -> double { return comp_.purchasing_.purchase_; }
    auto mpc() const -> double { return comp_.parameter_.mpc_; }
    void entry(
        const tbb::concurrent_vector<world::GoodsRequest>::iterator it  // NOLINT
    ) {
        comp_.posting_.myRequest_ = &*it;
    }
    auto rng() -> pcg32& { return comp_.rng_; }
};

void purchase(
    PurchaseView                               view,
    const double                               availableAsset,
    tbb::concurrent_vector<world::GoodsEntry>& entryBox,
    const int                                  step
);
}  // namespace goods_demander