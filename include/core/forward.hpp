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
struct Component;
}
namespace firm_finance {
struct Component;
}
namespace hhold_finance {
struct Component;
}
namespace labor_demander {
class LaborDemander;
}
namespace labor_supplier {
struct Component;
}
namespace goods_demander {
class GoodsDemander;
}
namespace goods_supplier {
class GoodsSupplier;
struct Component;
}  // namespace goods_supplier