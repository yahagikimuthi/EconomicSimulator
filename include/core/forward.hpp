#pragma once

namespace world {
struct LaborEntry;
struct LaborRequest;
struct ConsumerGoodsRequest;
struct ConsumerGoodsEntry;
class Workspace;
class RosterEntry;
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
namespace labor::supplier {
class LaborSupplier;
}
namespace goods::demander {
class ConsumerGoodsDemander;
}
namespace goods::supplier {
class ConsumerGoodsSupplier;
}  // namespace goods::supplier