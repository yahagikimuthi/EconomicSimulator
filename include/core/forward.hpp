#pragma once

namespace world {
struct LaborEntry;
struct LaborRequest;
struct GoodsRequest;
struct GoodsEntry;
struct RosterEntry;
struct CompanyBoard;
struct CensusDropBox;
}  // namespace world

namespace agent_index {
class Component;
}
namespace firm_finance {
class Component;
}
namespace hhold_finance {
class Component;
}
namespace labor_demander {
class LaborDemander;
}
namespace labor_supplier {
struct LaborSupplier;
}
namespace goods_demander {
class GoodsDemander;
}
namespace goods_supplier {
class GoodsSupplier;
}  // namespace goods_supplier

namespace core {
class Engine;
}