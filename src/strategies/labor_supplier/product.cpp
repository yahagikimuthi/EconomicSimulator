
#include "components/labor_supplier.hpp"
#include "core/base.hpp"
#include "world/message.hpp"

namespace labor_supplier {
struct [[nodiscard]] ProductView final : BaseView<Component> {
    using BaseView<Component>::BaseView;
    auto myWorkspace() -> world::Workspace& { return comp_.rosterEntry_->workspace_; }
    auto productPower() -> double { return comp_.productPower_; }
};
void product(ProductView view) {
    auto&        workspace = view.myWorkspace();
    const double product{view.productPower() * workspace.firmProductPower};
    workspace.totalLaborInput += product;
}
}  // namespace labor_supplier