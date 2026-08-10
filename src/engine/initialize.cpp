#include "core/engine.hpp"

#include <cassert>

#include <cstdlib>
#include <filesystem>
#include <highfive/H5DataSet.hpp>
#include <highfive/H5File.hpp>
#include <highfive/H5PropertyList.hpp>
#include <iostream>

#include "components/common.hpp"
#include "components/goods_demander.hpp"
#include "components/goods_supplier.hpp"
#include "components/labor_demander/hr_manager.hpp"
#include "components/labor_demander/labor_demander.hpp"
#include "components/labor_demander/planner.hpp"
#include "components/labor_demander/recruiter.hpp"
#include "components/labor_supplier/job_hunter.hpp"
#include "components/labor_supplier/labor_supplier.hpp"
#include "config.hpp"
#include "core/values/goods.hpp"
#include "helper.hpp"
#include "world/message.hpp"

/*
namespace {
void createFirm(const int id, entt::registry& registry) {
    entt::entity firm{registry.create()};
    registry.emplace<agent_index::Component>(firm, id);
    registry.emplace<firm_finance::Component>(firm);
    registry.emplace<labor_demander::Component>(firm);
    registry.emplace<goods_supplier::Component>(firm);
    registry.emplace<FirmTag>(firm);
}

void createHHold(const int id, entt::registry& registry) {
    entt::entity hhold{registry.create()};
    registry.emplace<agent_index::Component>(hhold, id);
    registry.emplace<hhold_finance::Component>(hhold);
    registry.emplace<labor_supplier::Component>(hhold);
    registry.emplace<goods_demander::Component>(hhold);
    registry.emplace<HHoldTag>(hhold);
}
}  // namespace 1000,5000
*/

namespace {
[[nodiscard]] auto makeFirmFinanceComponent(pcg32& masterRng) -> firm_finance::Component {
    const Money asset{helper::rand(masterRng, 1000.0, 5000.0)};
    return firm_finance::Component{asset};
}
[[nodiscard]] auto makeHHoldFinanceComponent(pcg32& masterRng) -> hhold_finance::Component {
    const Money asset{helper::rand(masterRng, 100.0, 500.0)};
    return hhold_finance::Component{asset};
}

[[nodiscard]] auto makeGoodsDemander(pcg32& masterRng, const int instanceCnt)
    -> goods::demander::GoodsDemander {
    const pcg32  rng{helper::makeSeed(masterRng), helper::makeSeed(masterRng)};
    const double mpc{helper::rand(masterRng, 0.7, 0.9)};
    const Step   myPhase{instanceCnt % config::goods_demander::maxPurchaseFrequency};
    return goods::demander::GoodsDemander{rng, mpc, myPhase};
}

[[nodiscard]] auto makeGoodsSupplierPlanner(pcg32& masterRng) -> goods::supplier::Planner {
    const pcg32         rng{helper::makeSeed(masterRng), helper::makeSeed(masterRng)};
    const double        lastMarkup{helper::rand(masterRng, 0.1, 0.3)};
    const GoodsQuantity lastSupply{helper::rand(masterRng, 4.0, 15.0)};
    const GoodsQuantity demandForecast{helper::rand(masterRng, 5.0, 20.0)};
    const bool          isSold{helper::rand(masterRng) < 0.5};
    const double        targetInvRatio{helper::rand(masterRng, 0.1, 0.2)};
    const double        markupAdjustVol{helper::rand(masterRng, 0.01, 0.02)};
    const double        demandForecastAdjustVol{helper::rand(masterRng, 0.1, 0.4)};
    return goods::supplier::Planner{
        rng,
        lastMarkup,
        lastSupply,
        demandForecast,
        isSold,
        targetInvRatio,
        markupAdjustVol,
        demandForecastAdjustVol
    };
}

[[nodiscard]] auto makeGoodsSupplierTrader(pcg32& masterRng) -> goods::supplier::Trader {
    const pcg32 rng{helper::makeSeed(masterRng), helper::makeSeed(masterRng)};
    return goods::supplier::Trader{rng};
}

[[nodiscard]] auto makeGoodsSupplierProducer(pcg32& masterRng) -> goods::supplier::Producer {
    const double        firmProductPower{helper::rand(masterRng, 0.5, 2.0)};
    const GoodsQuantity inventory{helper::rand(masterRng, 0.5, 2.0)};
    world::Workspace    workspace;
    workspace.firmProductPower = firmProductPower;
    return goods::supplier::Producer{std::move(workspace), firmProductPower, inventory};
}

[[nodiscard]] auto makeGoodsSupplier(pcg32& masterRng) -> goods::supplier::GoodsSupplier {
    return goods::supplier::GoodsSupplier{
        makeGoodsSupplierPlanner(masterRng),
        makeGoodsSupplierTrader(masterRng),
        makeGoodsSupplierProducer(masterRng)
    };
}

[[nodiscard]] auto makeLaborDemanderRecruiterPlanner(pcg32& masterRng
) -> labor::demander::RequestPlanner {
    const pcg32     rng{helper::makeSeed(masterRng), helper::makeSeed(masterRng)};
    const Wage      lastWage{helper::rand(masterRng, 10.0, 50.0)};
    const HeadCount lastEmploy{helper::randInt(masterRng, 4, 12)};
    const HeadCount lastOfferPlan{helper::randInt(masterRng, 10, 20)};
    const HeadCount applicantNum{helper::randInt(masterRng, 10, 20)};
    const double    offerRate{helper::rand(masterRng)};
    const double    wageAdjustVol{helper::rand(masterRng, 0.01, 0.1)};
    const double    offerAdjustVol{helper::rand(masterRng, 0.3, 0.5)};
    return labor::demander::RequestPlanner{
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

[[nodiscard]] auto makeLaborDemanderRecruiter(pcg32& masterRng) -> labor::demander::Recruiter {
    return labor::demander::Recruiter{makeLaborDemanderRecruiterPlanner(masterRng)};
}

[[nodiscard]] auto makeLaborDemanderHumanResourceManager(const AgentID id
) -> labor::demander::HumanResourceManager {
    world::CompanyBoard companyBoard{id};
    return labor::demander::HumanResourceManager{std::move(companyBoard)};
}

[[nodiscard]] auto makeLaborDemander(pcg32& masterRng, const AgentID id)
    -> labor::demander::LaborDemander {
    return {makeLaborDemanderRecruiter(masterRng), makeLaborDemanderHumanResourceManager(id)};
}

[[nodiscard]] auto makeLaborSupplierJobHunter(pcg32& masterRng) -> labor::supplier::JobHunter {
    const pcg32 rng{helper::makeSeed(masterRng), helper::makeSeed(masterRng)};
    return labor::supplier::JobHunter{rng};
}

[[nodiscard]] auto makeLaborSupplierEmployment(pcg32& masterRng) -> labor::supplier::Employment {
    const double productPower{helper::randNormal(masterRng, 1.0, 1.0 / 3.0, 0.0, 2.0)};
    return labor::supplier::Employment{productPower};
}

[[nodiscard]] auto makeLaborSupplier(pcg32& masterRng) -> labor::supplier::LaborSupplier {
    const pcg32  rng{helper::makeSeed(masterRng), helper::makeSeed(masterRng)};
    const double jobSearchThreshold{helper::rand(masterRng, 0.01, 0.05)};
    return labor::supplier::LaborSupplier{
        rng,
        makeLaborSupplierJobHunter(masterRng),
        makeLaborSupplierEmployment(masterRng),
        jobSearchThreshold
    };
}
}  // namespace

namespace core {
Engine::Engine(const int totalStep) : totalStep_{totalStep}, seed_{helper::generatePCG32Seed()} {
    if (not logger_.isValid()) {
        std::cerr << "can not create file\n";
        std::abort();
    }
    masterRng_ = {seed_.state, seed_.stream};
    firms_.reserve(config::agent_count::firm);
    for (const auto i : std::views::iota(0, config::agent_count::firm)) {
        firms_.emplace_back(Firm{
            .index   = {AgentID{i}},
            .finance = makeFirmFinanceComponent(masterRng_),
            .labor   = makeLaborDemander(masterRng_, AgentID{i}),
            .goods   = makeGoodsSupplier(masterRng_)
        });
    }

    hholds_.reserve(config::agent_count::hhold);
    for (const auto i : std::views::iota(0, config::agent_count::hhold)) {
        HHold hhold{
            .index   = {AgentID{i}},
            .finance = makeHHoldFinanceComponent(masterRng_),
            .labor   = makeLaborSupplier(masterRng_),
            .goods   = makeGoodsDemander(masterRng_, i)
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
          if (path.has_parent_path()) {
              std::filesystem::create_directories(path.parent_path());
          }
          return HighFive::File{
              filepath,
              HighFive::File::ReadWrite | HighFive::File::Create | HighFive::File::Truncate
          };
      }()} {}
}  // namespace core