#pragma once

#include <tbb/concurrent_vector.h>
#include <cstdint>
#include <highfive/H5DataSet.hpp>
#include <highfive/H5File.hpp>

#include "components/common.hpp"
#include "components/goods_demander.hpp"
#include "components/goods_supplier.hpp"
#include "components/labor_demander/labor_demander.hpp"
#include "components/labor_demander/planner.hpp"
#include "components/labor_demander/recruiter.hpp"
#include "components/labor_supplier.hpp"
#include "helper.hpp"
#include "world/message.hpp"

namespace core {
struct Firm {  // NOLINT
    agent_index::Component  index;
    firm_finance::Component finance;
    labor::demander::LaborDemander<labor::demander::Recruiter<labor::demander::RequestPlanner>>
                                   labor;
    goods::supplier::GoodsSupplier goods;
};
struct HHold {  // NOLINT
    agent_index::Component         index;
    hhold_finance::Component       finance;
    labor::supplier::LaborSupplier labor;
    goods::demander::GoodsDemander goods;
};
struct HHoldTag {};
struct FirmTag {};

class [[nodiscard]] Logger {
  public:
    explicit Logger();
    auto isValid() const -> bool { return file_.isValid(); }
    void save(const world::CensusDropBox& dropBox, const Step step);

  private:
    HighFive::File file_;
};

class [[nodiscard]] Engine {
  public:
    explicit Engine(const int totalStep);

    void run();

  private:
    void runLabor();
    void runGoods();
    void update();
    void logging();
    void reset();
    void check() const;

    auto makeSeed() -> std::uint64_t { return helper::makeSeed(masterRng_); }

    Logger logger_;
    // entt::registry     registry_;
    std::vector<Firm>  firms_;
    std::vector<HHold> hholds_;

    std::deque<world::CompanyBoard>             companyBoards_;
    std::deque<world::Workspace>                workspaces_;
    tbb::concurrent_vector<world::LaborRequest> laborRequestBox_;
    tbb::concurrent_vector<world::GoodsEntry>   goodsEntryBox_;

    world::CensusDropBox dropBox_;

    const Step totalStep_;
    Step       currentStep_{0};

    const helper::PCG32Seed seed_;
    pcg32                   masterRng_;
};
}  // namespace core