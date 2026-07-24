#pragma once

#include <utility>

#include "components/goods_demander.hpp"
#include "core/base.hpp"
#include "world/message.hpp"

namespace goods_demander {
struct [[nodiscard]] AfterTradeView final : BaseView<Component> {
    using BaseView<Component>::BaseView;

    auto myRequest() const -> std::pair<const world::GoodsEntry&, const world::GoodsRequest&> {
        SafePtr<const world::GoodsRequest> myRequest = comp_.posting_.myRequest_;
        return {myRequest->entry_, *myRequest};
    }
    void purchasePlus(const double plus) { comp_.purchasing_.purchase_ += plus; }
    auto isPosting() const -> bool { return comp_.posting_.isPosting_; }
};

void afterTrade(AfterTradeView view);
}  // namespace goods_demander