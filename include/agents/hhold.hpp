#pragma once

#include "components/consumer_goods_demander.hpp"
#include "components/finance/finance.hpp"
#include "components/labor_supplier/labor_supplier.hpp"

namespace abm {
struct HHold final {
    finance::Finance      finance_;
    LaborSupplier         laborSupplier_;
    ConsumerGoodsDemander consumerGoodsDemander_;
};
}  // namespace abm

namespace abm::hhold {
void work(LaborSupplier& laborSupplier) noexcept { laborSupplier.product(); }
}  // namespace abm::hhold