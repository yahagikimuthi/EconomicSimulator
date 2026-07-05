#include "strategies/goods_supplier/level1.hpp"

namespace goods_supplier {
[[nodiscard]] auto isSold(const IsSoldCtx& ctx) -> bool {
    if (ctx.lastSupply() == 0.0) return false;
    return ctx.inventory() / ctx.lastSupply() < ctx.targetInvRatio();
}
}  // namespace goods_supplier