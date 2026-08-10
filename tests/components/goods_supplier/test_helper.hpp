#pragma once

#include <optional>

#include "components/goods_supplier.hpp"
#include "core/forward.hpp"
#include "world/message.hpp"

namespace goods::supplier {
class [[nodiscard]] TraderTester {
  public:
    TraderTester(Trader& trader) : trader_{trader} {}

    auto myEntry() -> std::optional<world::GoodsEntry&>& { return trader_.myEntry_; }
    auto isPosting() -> bool& { return trader_.isPosting_; }
    auto inventory() -> GoodsQuantity& { return trader_.ledger_.inventory; }

  private:
    Trader& trader_;
};
}  // namespace goods::supplier