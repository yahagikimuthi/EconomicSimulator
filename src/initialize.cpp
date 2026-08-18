#include "core/engine.hpp"

#include <cstdlib>
#include <filesystem>
#include <highfive/H5DataSet.hpp>
#include <highfive/H5File.hpp>
#include <highfive/H5PropertyList.hpp>
#include <iostream>
#include <utility>

#include "components/base_goods_demander.hpp"
#include "components/base_goods_supplier/planner.hpp"
#include "components/base_goods_supplier/producer.hpp"
#include "components/common.hpp"
#include "components/consumer_goods_demander.hpp"
#include "components/consumer_goods_supplier/consumer_goods_supplier.hpp"
#include "components/labor_demander/labor_demander.hpp"
#include "components/labor_supplier/job_hunter.hpp"
#include "components/labor_supplier/labor_supplier.hpp"
#include "components/production_goods_demander.hpp"
#include "components/production_goods_supplier/production_goods_supplier.hpp"
#include "config.hpp"
#include "core/values/goods.hpp"
#include "util.hpp"
#include "world/common.hpp"
#include "world/goods.hpp"
#include "world/labor.hpp"

namespace abm {
[[nodiscard]] auto makeFirmFinanceComponent(RandomGenerator& masterRng) -> FirmFinance {
    const auto asset = Money{masterRng.rand(1000.0, 5000.0)};
    return FirmFinance{asset};
}
[[nodiscard]] auto makeHHoldFinanceComponent(RandomGenerator& masterRng) -> HHoldFinance {
    const auto asset = Money{masterRng.rand(100.0, 500.0)};
    return HHoldFinance{asset};
}

[[nodiscard]] auto makeConsumerGoodsDemander(RandomGenerator& masterRng, const int instanceCnt)
    -> ConsumerGoodsDemander {
    const auto rng = RandomGenerator{pcg32{masterRng.makeUint64(), masterRng.makeUint64()}};
    const auto mpc = masterRng.rand(0.7, 0.9);
    const auto budgetCalculator = base_goods::demander::BudgetCalculator{mpc};
    const auto myPhase          = Step{instanceCnt % config::goods_demander::maxPurchaseFrequency};
    return ConsumerGoodsDemander{rng, budgetCalculator, myPhase};
}

[[nodiscard]] auto makeProductionGoodsDemander(RandomGenerator& masterRng
) -> ProductionGoodsDemander {
    const auto rng = RandomGenerator{{masterRng.makeUint64(), masterRng.makeUint64()}};
    const auto mpc = masterRng.rand(0.7, 0.9);
    return ProductionGoodsDemander{rng, mpc};
}

namespace base_goods::supplier {
[[nodiscard]] auto makeMarkupPlanner(RandomGenerator& masterRng) -> MarkupPlanner {
    const auto rng       = RandomGenerator{{masterRng.makeUint64(), masterRng.makeUint64()}};
    const auto log       = masterRng.rand(0.1, 0.3);
    const auto adjustVol = masterRng.rand(0.1, 0.4);
    return {rng, log, adjustVol};
}

[[nodiscard]] auto makeDemandForecastManager(RandomGenerator& masterRng) -> DemandForecastManager {
    const auto adjustVol = masterRng.rand(0.1, 0.4);
    return DemandForecastManager{adjustVol};
}

[[nodiscard]] auto makePlanner(RandomGenerator& masterRng) -> Planner {
    const auto lastSupply     = GoodsQuantity{masterRng.rand(4.0, 15.0)};
    const auto isSold         = bool{masterRng.rand() < 0.5};
    const auto targetInvRatio = masterRng.rand(0.1, 0.2);
    return Planner{
        lastSupply,
        isSold,
        targetInvRatio,
        MarkupPlanner{makeMarkupPlanner(masterRng)},
        DemandForecastManager{makeDemandForecastManager(masterRng)}
    };
}

[[nodiscard]] auto makeProducer(RandomGenerator& masterRng) -> Producer {
    const auto firmProductPower = masterRng.rand(0.5, 2.0);
    const auto inventory        = GoodsQuantity{masterRng.rand(0.5, 2.0)};
    auto       workspace        = Workspace{firmProductPower};
    return Producer{std::move(workspace), firmProductPower, inventory};
}
}  // namespace base_goods::supplier

namespace consumer_goods::supplier {
[[nodiscard]] auto makeTrader(RandomGenerator& masterRng) -> Trader {
    const auto rng = RandomGenerator{{masterRng.makeUint64(), masterRng.makeUint64()}};
    return Trader{rng};
}

[[nodiscard]] auto makeSupplier(RandomGenerator& masterRng) -> ConsumerGoodsSupplier {
    return ConsumerGoodsSupplier{
        base_goods::supplier::makePlanner(masterRng),
        makeTrader(masterRng),
        base_goods::supplier::makeProducer(masterRng)
    };
}
}  // namespace consumer_goods::supplier

namespace production_goods::supplier {
[[nodiscard]] auto makeTrader(RandomGenerator& masterRng) -> Trader {
    const auto rng = RandomGenerator{{masterRng.makeUint64(), masterRng.makeUint64()}};
    return Trader{rng};
}

[[nodiscard]] auto make(RandomGenerator& masterRng) -> ProductionGoodsSupplier {
    return {
        base_goods::supplier::makePlanner(masterRng),
        makeTrader(masterRng),
        base_goods::supplier::makeProducer(masterRng)
    };
}
}  // namespace production_goods::supplier

namespace labor::supplier {
[[nodiscard]] auto makeJobHunter(RandomGenerator& masterRng) -> JobHunter {
    const auto rng = RandomGenerator{{masterRng.makeUint64(), masterRng.makeUint64()}};
    return JobHunter{rng};
}

[[nodiscard]] auto makeEmployment(RandomGenerator& masterRng) -> Employment {
    const auto productPower = masterRng.randNormal(1.0, 1.0 / 3.0, 0.0, 2.0);
    return Employment{productPower};
}

[[nodiscard]] auto makeJobSearchThreshold(RandomGenerator& masterRng) -> JobSearch {
    const auto rng = RandomGenerator{{masterRng.makeUint64(), masterRng.makeUint64()}};
    const auto jobSearchThreshold = masterRng.rand(0.01, 0.05);
    return {rng, jobSearchThreshold};
}

[[nodiscard]] auto make(RandomGenerator& masterRng) -> LaborSupplier {
    return LaborSupplier{
        makeJobHunter(masterRng), makeEmployment(masterRng), makeJobSearchThreshold(masterRng)
    };
}
}  // namespace labor::supplier

namespace {
[[nodiscard]] auto makeLaborDemander(
    RandomGenerator& masterRng, const AgentID id, const Market firmType
) -> ::abm::labor::demander::LaborDemander {
    return {masterRng, CompanyBoard{id, firmType}};
}
}  // namespace

Engine::Engine(const int totalStep) noexcept
    : totalStep_{totalStep}, seed_{generateSeed()}, rng_{pcg32{seed_.state, seed_.stream}} {
    if (not logger_.isValid()) {
        std::cerr << "can not create file\n";
        std::abort();
    }

    namespace cnt = config::agent_count;

    bToCFirms_.reserve(cnt::bToCFirm);
    int agentId{};
    for (; agentId < config::agent_count::bToCFirm; ++agentId) {
        bToCFirms_.emplace_back(BtoCFirm{
            .index           = AgentIndex{AgentID{agentId}},
            .finance         = makeFirmFinanceComponent(rng_),
            .labor           = makeLaborDemander(rng_, AgentID{agentId}, Market::consumerGoods),
            .consumerGoods   = consumer_goods::supplier::makeSupplier(rng_),
            .productionGoods = makeProductionGoodsDemander(rng_)
        });
    }

    bToBFirms_.reserve(cnt::bToBFirm);
    for (; agentId < cnt::bToCFirm + cnt::bToBFirm; ++agentId) {
        bToBFirms_.emplace_back(BtoBFirm{
            .index   = AgentIndex{AgentID{agentId}},
            .finance = makeFirmFinanceComponent(rng_),
            .labor   = makeLaborDemander(rng_, AgentID{agentId}, Market::productionGoods),
            .productionGoodsSupplier = production_goods::supplier::make(rng_),
            .productionGoodsDemander = makeProductionGoodsDemander(rng_)
        });
    }

    hholds_.reserve(cnt::hhold);
    for (; agentId < cnt::bToCFirm + cnt::bToBFirm + cnt::hhold; ++agentId) {
        hholds_.emplace_back(HHold{
            .index         = AgentIndex{AgentID{agentId}},
            .finance       = makeHHoldFinanceComponent(rng_),
            .labor         = labor::supplier::make(rng_),
            .consumerGoods = makeConsumerGoodsDemander(rng_, agentId)
        });
    }
}

Logger::Logger()
    : file_{[]() -> HighFive::File {
          const auto filepath =
              static_cast<std::string>(config::setting::simulationResultOutputPath);
          const auto path = std::filesystem::path{filepath};
          if (path.has_parent_path()) std::filesystem::create_directories(path.parent_path());
          return HighFive::File{
              filepath,
              HighFive::File::ReadWrite | HighFive::File::Create | HighFive::File::Truncate
          };
      }()} {}
}  // namespace abm