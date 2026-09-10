#pragma once

#include <span>

#include "components/government.hpp"
#include "engine/agents.hpp"
#include "system/goods.hpp"
#include "world/base_goods.hpp"
#include "world/drop_box.hpp"

namespace abm::engine {
class GoodsEngine final {
  public:
    explicit GoodsEngine(GoodsDropBox& dropBox) noexcept : dropBox_{dropBox} {}

    void run(
        std::span<GoodsFirm> goodsFirm, std::span<HHold> hholds, Government& government
    ) noexcept {
        using namespace goods;

        for (auto& firm : goodsFirm) {
            entry(firm.id, firm.goodsSupplier, market_);
        }

        for (auto& hhold : hholds) {
            request(hhold.id, hhold.finance, hhold.goods, market_);
        }

        for (auto& firm : goodsFirm) {
            trade(firm.finance, firm.goodsSupplier, government);
        }

        for (auto& hhold : hholds) {
            afterTrade(hhold.finance, hhold.goods);
        }

        market_.clear();

        for (auto& firm : goodsFirm) {
            logging(dropBox_, firm.goodsSupplier);
        }
    }

  private:
    GoodsMarket   market_;
    GoodsDropBox& dropBox_;
};
}  // namespace abm::engine