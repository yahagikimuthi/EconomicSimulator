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
#include "components/labor_demander/human_resource.hpp"
#include "components/labor_demander/labor_demander.hpp"
#include "components/labor_demander/planner.hpp"
#include "components/labor_supplier/job_hunter.hpp"
#include "components/labor_supplier/labor_supplier.hpp"
#include "components/production_goods_demander.hpp"
#include "components/production_goods_supplier/production_goods_supplier.hpp"
#include "config.hpp"
#include "core/values/goods.hpp"
#include "core/values/labor.hpp"
#include "util.hpp"
#include "world/message.hpp"

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
    return {rng, mpc};
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
    return {adjustVol};
}

[[nodiscard]] auto makePlanner(RandomGenerator& masterRng) -> Planner {
    const auto lastSupply     = GoodsQuantity{masterRng.rand(4.0, 15.0)};
    const auto isSold         = bool{masterRng.rand() < 0.5};
    const auto targetInvRatio = masterRng.rand(0.1, 0.2);
    return {
        lastSupply,
        isSold,
        targetInvRatio,
        makeMarkupPlanner(masterRng),
        makeDemandForecastManager(masterRng)
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
    return {rng};
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
    return {rng};
}

[[nodiscard]] auto make(RandomGenerator& masterRng) -> ProductionGoodsSupplier {
    return {
        base_goods::supplier::makePlanner(masterRng),
        makeTrader(masterRng),
        base_goods::supplier::makeProducer(masterRng)
    };
}
}  // namespace production_goods::supplier

namespace labor::demander::planner {
[[nodiscard]] auto makeWagePlannerMemory(RandomGenerator& masterRng) -> WagePlannerMemory {
    const auto lastWage         = Wage{masterRng.rand(10.0, 50.0)};
    const auto lastTargetEmploy = HeadCount{masterRng.rand(10, 20)};
    const auto lastApplicants   = HeadCount{masterRng.rand(10, 20)};
    return {lastWage, lastTargetEmploy, lastApplicants};
}

[[nodiscard]] auto makeWagePlanner(RandomGenerator& masterRng) -> WagePlanner {
    const auto rng       = RandomGenerator{{masterRng.makeUint64(), masterRng.makeUint64()}};
    const auto adjustVol = masterRng.rand(0.01, 0.1);
    return {rng, makeWagePlannerMemory(masterRng), adjustVol};
}

[[nodiscard]] auto makeOfferPlanner(RandomGenerator& masterRng) -> OfferPlanner {
    const auto rng       = RandomGenerator{{masterRng.makeUint64(), masterRng.makeUint64()}};
    const auto offerRate = masterRng.rand();
    const auto adjustVol = masterRng.rand(0.3, 0.5);
    return {rng, offerRate, adjustVol};
}

[[nodiscard]] auto makePlanner(RandomGenerator& masterRng) -> RecruitPlanner {
    return {makeWagePlanner(masterRng), makeOfferPlanner(masterRng)};
}
}  // namespace labor::demander::planner

namespace labor::demander::human_resource {
[[nodiscard]] auto makeHumanResourceManager(const AgentID id, const Market firmType)
    -> HumanResource {
    auto companyBoard = CompanyBoard{id, firmType};
    return HumanResource{std::move(companyBoard)};
}
}  // namespace labor::demander::human_resource

namespace labor::demander {
[[nodiscard]] auto makeRecruitSystem(RandomGenerator& masterRng) -> RecruitSystem {
    return {planner::makePlanner(masterRng)};
}

[[nodiscard]] auto make(RandomGenerator& masterRng, const AgentID id, const Market firmType)
    -> LaborDemander {
    return {makeRecruitSystem(masterRng), human_resource::makeHumanResourceManager(id, firmType)};
}
}  // namespace labor::demander

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

Engine::Engine(const int totalStep)
    : totalStep_{totalStep}, seed_{generateSeed()}, rng_{pcg32{seed_.state, seed_.stream}} {
    if (not logger_.isValid()) {
        std::cerr << "can not create file\n";
        std::abort();
    }

    namespace cnt = config::agent_count;

    BtoCFirms_.reserve(cnt::BtoCFirm);
    int agentId{};
    for (; agentId < config::agent_count::BtoCFirm; ++agentId) {
        BtoCFirms_.emplace_back(BtoCFirm{
            .index           = {AgentID{agentId}},
            .finance         = makeFirmFinanceComponent(rng_),
            .labor           = labor::demander::make(rng_, AgentID{agentId}, Market::consumerGoods),
            .consumerGoods   = consumer_goods::supplier::makeSupplier(rng_),
            .productionGoods = makeProductionGoodsDemander(rng_)
        });
    }

    BtoBFirms_.reserve(cnt::BtoBFirm);
    for (; agentId < cnt::BtoCFirm + cnt::BtoBFirm; ++agentId) {
        BtoBFirms_.emplace_back(BtoBFirm{
            .index   = {AgentID{agentId}},
            .finance = makeFirmFinanceComponent(rng_),
            .labor   = labor::demander::make(rng_, AgentID{agentId}, Market::productionGoods),
            .productionGoodsSupplier = production_goods::supplier::make(rng_),
            .productionGoodsDemander = makeProductionGoodsDemander(rng_)
        });
    }

    hholds_.reserve(cnt::hhold);
    for (; agentId < cnt::BtoCFirm + cnt::BtoBFirm + cnt::hhold; ++agentId) {
        hholds_.emplace_back(HHold{
            .index         = {AgentID{agentId}},
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