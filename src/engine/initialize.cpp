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
#include "components/labor_demander/hr_manager.hpp"
#include "components/labor_demander/labor_demander.hpp"
#include "components/labor_demander/planner.hpp"
#include "components/labor_demander/util.hpp"
#include "components/labor_supplier/job_hunter.hpp"
#include "components/labor_supplier/labor_supplier.hpp"
#include "components/production_goods_demander.hpp"
#include "components/production_goods_supplier/production_goods_supplier.hpp"
#include "components/util.hpp"
#include "config.hpp"
#include "core/values/goods.hpp"
#include "core/values/labor.hpp"
#include "world/message.hpp"

namespace abm {
using detail::RandomGenerator;
[[nodiscard]] auto makeFirmFinanceComponent(RandomGenerator& masterRng) -> FirmFinance {
    const Money asset{masterRng.rand(1000.0, 5000.0)};
    return FirmFinance{asset};
}
[[nodiscard]] auto makeHHoldFinanceComponent(RandomGenerator& masterRng) -> HHoldFinance {
    const Money asset{masterRng.rand(100.0, 500.0)};
    return HHoldFinance{asset};
}

[[nodiscard]] auto makeConsumerGoodsDemander(RandomGenerator& masterRng, const int instanceCnt)
    -> ConsumerGoodsDemander {
    const RandomGenerator rng{pcg32{masterRng.makeUint64(), masterRng.makeUint64()}};
    const double          mpc{masterRng.rand(0.7, 0.9)};
    const base_goods::demander::BudgetCalculator budgetCalculator{mpc};
    const Step myPhase{instanceCnt % config::goods_demander::maxPurchaseFrequency};
    return ConsumerGoodsDemander{rng, budgetCalculator, myPhase};
}

[[nodiscard]] auto makeProductionGoodsDemander(RandomGenerator& masterRng
) -> ProductionGoodsDemander {
    const RandomGenerator rng{{masterRng.makeUint64(), masterRng.makeUint64()}};
    const double          mpc{masterRng.rand(0.7, 0.9)};
    return {rng, mpc};
}

namespace base_goods::supplier {
[[nodiscard]] auto makeMarkupPlanner(RandomGenerator& masterRng) -> MarkupPlanner {
    const RandomGenerator rng{{masterRng.makeUint64(), masterRng.makeUint64()}};
    const double          log{masterRng.rand(0.1, 0.3)};
    const double          adjustVol{masterRng.rand(0.1, 0.4)};
    return {rng, log, adjustVol};
}

[[nodiscard]] auto makeDemandForecastManager(RandomGenerator& masterRng) -> DemandForecastManager {
    const double adjustVol{masterRng.rand(0.1, 0.4)};
    return {adjustVol};
}

[[nodiscard]] auto makePlanner(RandomGenerator& masterRng) -> Planner {
    const RandomGenerator rng{{masterRng.makeUint64(), masterRng.makeUint64()}};
    const GoodsQuantity   lastSupply{masterRng.rand(4.0, 15.0)};
    const GoodsQuantity   demandForecast{masterRng.rand(5.0, 20.0)};
    const bool            isSold{masterRng.rand() < 0.5};
    const double          targetInvRatio{masterRng.rand(0.1, 0.2)};
    return {
        lastSupply,
        isSold,
        targetInvRatio,
        makeMarkupPlanner(masterRng),
        makeDemandForecastManager(masterRng)
    };
}

[[nodiscard]] auto makeProducer(RandomGenerator& masterRng) -> Producer {
    const double        firmProductPower{masterRng.rand(0.5, 2.0)};
    const GoodsQuantity inventory{masterRng.rand(0.5, 2.0)};
    Workspace           workspace{firmProductPower};
    return Producer{std::move(workspace), firmProductPower, inventory};
}
}  // namespace base_goods::supplier

namespace consumer_goods::supplier {
[[nodiscard]] auto makeTrader(RandomGenerator& masterRng) -> Trader {
    const RandomGenerator rng{{masterRng.makeUint64(), masterRng.makeUint64()}};
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
    const RandomGenerator rng{{masterRng.makeUint64(), masterRng.makeUint64()}};
    return {rng};
}

[[nodiscard]] auto makeSupplier(RandomGenerator& masterRng) -> ProductionGoodsSupplier {
    return {
        base_goods::supplier::makePlanner(masterRng),
        makeTrader(masterRng),
        base_goods::supplier::makeProducer(masterRng)
    };
}
}  // namespace production_goods::supplier

namespace labor::demander {
[[nodiscard]] auto makeWagePlanner(RandomGenerator& masterRng) -> WagePlanner {
    const RandomGenerator rng{{masterRng.makeUint64(), masterRng.makeUint64()}};
    const double          adjustVol{masterRng.rand(0.01, 0.1)};
    return {rng, adjustVol};
}

[[nodiscard]] auto makeOfferPlanner(RandomGenerator& masterRng) -> OfferPlanner {
    const RandomGenerator rng{{masterRng.makeUint64(), masterRng.makeUint64()}};
    const double          offerRate{masterRng.rand()};
    const double          adjustVol{masterRng.rand(0.3, 0.5)};
    return {rng, offerRate, adjustVol};
}

[[nodiscard]] auto makePlanner(RandomGenerator& masterRng) -> RequestPlanner {
    return RequestPlanner{makeWagePlanner(masterRng), makeOfferPlanner(masterRng)};
}

[[nodiscard]] auto makeHumanResourceManager(const AgentID id, const Market firmType)
    -> HumanResource {
    CompanyBoard companyBoard{id, firmType};
    return HumanResource{std::move(companyBoard)};
}

[[nodiscard]] auto makePlanMemory(RandomGenerator& masterRng) -> RecruitPlanMemory {
    const Wage      wage{masterRng.rand(10.0, 50.0)};
    const HeadCount employ{masterRng.rand(10, 20)};
    const HeadCount offer{masterRng.rand(10, 20)};
    return {wage, employ, offer};
}

[[nodiscard]] auto makeResultMemory(RandomGenerator& masterRng) -> RecruitResultMemory {
    const HeadCount lastApplicants{masterRng.rand(10, 20)};
    const HeadCount lastEmploy{masterRng.rand(5, 15)};
    return {lastApplicants, lastEmploy};
}

[[nodiscard]] auto makeMemory(RandomGenerator& masterRng) -> Memory {
    return {makePlanMemory(masterRng), makeResultMemory(masterRng)};
}

[[nodiscard]] auto make(RandomGenerator& masterRng, const AgentID id, const Market firmType)
    -> LaborDemander {
    return {makePlanner(masterRng), makeHumanResourceManager(id, firmType), makeMemory(masterRng)};
}
}  // namespace labor::demander

namespace labor::supplier {
[[nodiscard]] auto makeJobHunter(RandomGenerator& masterRng) -> JobHunter {
    const RandomGenerator rng{{masterRng.makeUint64(), masterRng.makeUint64()}};
    return JobHunter{rng};
}

[[nodiscard]] auto makeEmployment(RandomGenerator& masterRng) -> Employment {
    const double productPower{masterRng.randNormal(1.0, 1.0 / 3.0, 0.0, 2.0)};
    return Employment{productPower};
}

[[nodiscard]] auto makeJobSearchThreshold(RandomGenerator& masterRng) -> JobSearchThreshold {
    const RandomGenerator rng{{masterRng.makeUint64(), masterRng.makeUint64()}};
    const double          jobSearchThreshold{masterRng.rand(0.01, 0.05)};
    return {rng, jobSearchThreshold};
}

[[nodiscard]] auto make(RandomGenerator& masterRng) -> LaborSupplier {
    const RandomGenerator rng{{masterRng.makeUint64(), masterRng.makeUint64()}};
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

    BtoCFirms_.reserve(config::agent_count::BtoCFirm);
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

    BtoBFirms_.reserve(config::agent_count::BtoBFirm);
    for (; agentId < config::agent_count::BtoBFirm; ++agentId) {
        BtoBFirms_.emplace_back(BtoBFirm{
            .index   = {AgentID{agentId}},
            .finance = makeFirmFinanceComponent(rng_),
            .labor   = labor::demander::make(rng_, AgentID{agentId}, Market::productionGoods),
            .productionGoodsSupplier = production_goods::supplier::makeSupplier(rng_),
            .productionGoodsDemander = makeProductionGoodsDemander(rng_)
        });
    }

    hholds_.reserve(config::agent_count::hhold);
    for (; agentId < config::agent_count::hhold; ++agentId) {
        HHold hhold{
            .index         = {AgentID{agentId}},
            .finance       = makeHHoldFinanceComponent(rng_),
            .labor         = labor::supplier::make(rng_),
            .consumerGoods = makeConsumerGoodsDemander(rng_, agentId)
        };
        hholds_.push_back(hhold);
    }
}

Logger::Logger()
    : file_{[]() -> HighFive::File {
          const std::string filepath{
              static_cast<std::string>(config::setting::simulationResultOutputPath)
          };
          const std::filesystem::path path{filepath};
          if (path.has_parent_path()) std::filesystem::create_directories(path.parent_path());
          return HighFive::File{
              filepath,
              HighFive::File::ReadWrite | HighFive::File::Create | HighFive::File::Truncate
          };
      }()} {}
}  // namespace abm