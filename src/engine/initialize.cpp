#include "core/engine.hpp"

#include <cstdlib>
#include <filesystem>
#include <highfive/H5DataSet.hpp>
#include <highfive/H5File.hpp>
#include <highfive/H5PropertyList.hpp>
#include <iostream>
#include <utility>

#include "components/base_goods_supplier/planner.hpp"
#include "components/base_goods_supplier/producer.hpp"
#include "components/common.hpp"
#include "components/consumer_goods_demander.hpp"
#include "components/consumer_goods_supplier/consumer_goods_supplier.hpp"
#include "components/labor_demander/hr_manager.hpp"
#include "components/labor_demander/labor_demander.hpp"
#include "components/labor_demander/planner.hpp"
#include "components/labor_demander/recruiter.hpp"
#include "components/labor_supplier/job_hunter.hpp"
#include "components/labor_supplier/labor_supplier.hpp"
#include "components/production_goods_demander.hpp"
#include "components/production_goods_supplier/production_goods_supplier.hpp"
#include "config.hpp"
#include "core/values/goods.hpp"
#include "helper.hpp"
#include "world/message.hpp"

namespace abm {
[[nodiscard]] auto makeFirmFinanceComponent(pcg32& masterRng) -> FirmFinance {
    const Money asset{helper::rand(masterRng, 1000.0, 5000.0)};
    return FirmFinance{asset};
}
[[nodiscard]] auto makeHHoldFinanceComponent(pcg32& masterRng) -> HHoldFinance {
    const Money asset{helper::rand(masterRng, 100.0, 500.0)};
    return HHoldFinance{asset};
}

[[nodiscard]] auto makeConsumerGoodsDemander(pcg32& masterRng, const int instanceCnt)
    -> ConsumerGoodsDemander {
    const pcg32  rng{helper::makeSeed(masterRng), helper::makeSeed(masterRng)};
    const double mpc{helper::rand(masterRng, 0.7, 0.9)};
    const Step   myPhase{instanceCnt % config::goods_demander::maxPurchaseFrequency};
    return ConsumerGoodsDemander{rng, mpc, myPhase};
}

[[nodiscard]] auto makeProductionGoodsDemander(pcg32& masterRng) -> ProductionGoodsDemander {
    const pcg32  rng{helper::makeSeed(masterRng), helper::makeSeed(masterRng)};
    const double mpc{helper::rand(masterRng, 0.7, 0.9)};
    return {rng, mpc};
}

namespace base_goods::supplier {
[[nodiscard]] auto makeMarkupPlanner(pcg32& masterRng) -> MarkupPlanner {
    using namespace helper;
    const pcg32  rng{makeSeed(masterRng), makeSeed(masterRng)};
    const double log{rand(masterRng, 0.1, 0.3)};
    const double adjustVol{rand(masterRng, 0.1, 0.4)};
    return {rng, log, adjustVol};
}

[[nodiscard]] auto makeDemandForecastManager(pcg32& masterRng) -> DemandForecastManager {
    const double adjustVol{helper::rand(masterRng, 0.1, 0.4)};
    return {adjustVol};
}

[[nodiscard]] auto makePlanner(pcg32& masterRng) -> Planner {
    const pcg32         rng{helper::makeSeed(masterRng), helper::makeSeed(masterRng)};
    const GoodsQuantity lastSupply{helper::rand(masterRng, 4.0, 15.0)};
    const GoodsQuantity demandForecast{helper::rand(masterRng, 5.0, 20.0)};
    const bool          isSold{helper::rand(masterRng) < 0.5};
    const double        targetInvRatio{helper::rand(masterRng, 0.1, 0.2)};
    return {
        lastSupply,
        isSold,
        targetInvRatio,
        makeMarkupPlanner(masterRng),
        makeDemandForecastManager(masterRng)
    };
}

[[nodiscard]] auto makeProducer(pcg32& masterRng) -> Producer {
    const double        firmProductPower{helper::rand(masterRng, 0.5, 2.0)};
    const GoodsQuantity inventory{helper::rand(masterRng, 0.5, 2.0)};
    Workspace           workspace{firmProductPower};
    return Producer{std::move(workspace), firmProductPower, inventory};
}
}  // namespace base_goods::supplier

namespace consumer_goods::supplier {
[[nodiscard]] auto makeTrader(pcg32& masterRng) -> Trader {
    const pcg32 rng{helper::makeSeed(masterRng), helper::makeSeed(masterRng)};
    return {rng};
}

[[nodiscard]] auto makeSupplier(pcg32& masterRng) -> ConsumerGoodsSupplier {
    return ConsumerGoodsSupplier{
        base_goods::supplier::makePlanner(masterRng),
        makeTrader(masterRng),
        base_goods::supplier::makeProducer(masterRng)
    };
}
}  // namespace consumer_goods::supplier

namespace production_goods::supplier {
[[nodiscard]] auto makeTrader(pcg32& masterRng) -> Trader {
    const pcg32 rng{helper::makeSeed(masterRng), helper::makeSeed(masterRng)};
    return {rng};
}

[[nodiscard]] auto makeSupplier(pcg32& masterRng) -> ProductionGoodsSupplier {
    return {
        base_goods::supplier::makePlanner(masterRng),
        makeTrader(masterRng),
        base_goods::supplier::makeProducer(masterRng)
    };
}
}  // namespace production_goods::supplier

namespace labor::demander {
[[nodiscard]] auto makePlanner(pcg32& masterRng) -> RequestPlanner {
    const pcg32     rng{helper::makeSeed(masterRng), helper::makeSeed(masterRng)};
    const Wage      lastWage{helper::rand(masterRng, 10.0, 50.0)};
    const HeadCount lastEmploy{helper::randInt(masterRng, 4, 12)};
    const HeadCount lastOfferPlan{helper::randInt(masterRng, 10, 20)};
    const HeadCount applicantNum{helper::randInt(masterRng, 10, 20)};
    const double    offerRate{helper::rand(masterRng)};
    const double    wageAdjustVol{helper::rand(masterRng, 0.01, 0.1)};
    const double    offerAdjustVol{helper::rand(masterRng, 0.3, 0.5)};
    return RequestPlanner{
        rng,
        lastWage,
        lastEmploy,
        lastOfferPlan,
        applicantNum,
        offerRate,
        wageAdjustVol,
        offerAdjustVol
    };
}

[[nodiscard]] auto makeRecruiter(pcg32& masterRng) -> Recruiter { return {makePlanner(masterRng)}; }

[[nodiscard]] auto makeHumanResourceManager(const AgentID id, const Market firmType)
    -> HumanResourceManager {
    CompanyBoard companyBoard{id, firmType};
    return HumanResourceManager{std::move(companyBoard)};
}

[[nodiscard]] auto make(pcg32& masterRng, const AgentID id, const Market firmType)
    -> LaborDemander {
    return {makeRecruiter(masterRng), makeHumanResourceManager(id, firmType)};
}
}  // namespace labor::demander

namespace labor::supplier {
[[nodiscard]] auto makeJobHunter(pcg32& masterRng) -> JobHunter {
    const pcg32 rng{helper::makeSeed(masterRng), helper::makeSeed(masterRng)};
    return JobHunter{rng};
}

[[nodiscard]] auto makeEmployment(pcg32& masterRng) -> Employment {
    const double productPower{helper::randNormal(masterRng, 1.0, 1.0 / 3.0, 0.0, 2.0)};
    return Employment{productPower};
}

[[nodiscard]] auto make(pcg32& masterRng) -> LaborSupplier {
    const pcg32  rng{helper::makeSeed(masterRng), helper::makeSeed(masterRng)};
    const double jobSearchThreshold{helper::rand(masterRng, 0.01, 0.05)};
    return LaborSupplier{
        rng, makeJobHunter(masterRng), makeEmployment(masterRng), jobSearchThreshold
    };
}
}  // namespace labor::supplier

Engine::Engine(const int totalStep) : totalStep_{totalStep}, seed_{helper::generatePCG32Seed()} {
    if (not logger_.isValid()) {
        std::cerr << "can not create file\n";
        std::abort();
    }
    masterRng_ = {seed_.state, seed_.stream};

    BtoCFirms_.reserve(config::agent_count::BtoCFirm);
    int agentId{};
    for (; agentId < config::agent_count::BtoCFirm; ++agentId) {
        BtoCFirms_.emplace_back(BtoCFirm{
            .index   = {AgentID{agentId}},
            .finance = makeFirmFinanceComponent(masterRng_),
            .labor   = labor::demander::make(masterRng_, AgentID{agentId}, Market::consumerGoods),
            .consumerGoods   = consumer_goods::supplier::makeSupplier(masterRng_),
            .productionGoods = makeProductionGoodsDemander(masterRng_)
        });
    }

    BtoBFirms_.reserve(config::agent_count::BtoBFirm);
    for (; agentId < config::agent_count::BtoBFirm; ++agentId) {
        BtoBFirms_.emplace_back(BtoBFirm{
            .index   = {AgentID{agentId}},
            .finance = makeFirmFinanceComponent(masterRng_),
            .labor   = labor::demander::make(masterRng_, AgentID{agentId}, Market::productionGoods),
            .productionGoodsSupplier = production_goods::supplier::makeSupplier(masterRng_),
            .productionGoodsDemander = makeProductionGoodsDemander(masterRng_)
        });
    }

    hholds_.reserve(config::agent_count::hhold);
    for (; agentId < config::agent_count::hhold; ++agentId) {
        HHold hhold{
            .index         = {AgentID{agentId}},
            .finance       = makeHHoldFinanceComponent(masterRng_),
            .labor         = labor::supplier::make(masterRng_),
            .consumerGoods = makeConsumerGoodsDemander(masterRng_, agentId)
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