#include "strategies/labor_supplier/product.hpp"

#include "world/message.hpp"

namespace labor_supplier {
void product(ProductView view) {
    auto&        workspace = view.myWorkspace();
    const double product{view.productPower() * workspace.firmProductPower};
    workspace.totalLaborInput += product;
}
}  // namespace labor_supplier