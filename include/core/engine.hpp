#pragma once

#include <tbb/concurrent_vector.h>
#include <cstdint>
#include <highfive/H5DataSet.hpp>
#include <highfive/H5File.hpp>

#include "components/common.hpp"
#include "components/goods_demander.hpp"
#include "components/goods_supplier.hpp"
#include "components/labor_demander.hpp"
#include "components/labor_supplier.hpp"
#include "helper.hpp"
#include "world/message.hpp"

namespace core {
struct Firm {  // NOLINT
    agent_index::Component        index;
    firm_finance::Component       finance;
    labor_demander::LaborDemander labor;
    goods_supplier::GoodsSupplier goods;
};
struct HHold {  // NOLINT
    agent_index::Component        index;
    hhold_finance::Component      finance;
    labor_supplier::LaborSupplier labor;
    goods_demander::GoodsDemander goods;
};
struct HHoldTag {};
struct FirmTag {};

class Logger {
  public:
    explicit Logger();
    [[nodiscard]] auto isValid() const -> bool { return file_.isValid(); }

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

    auto makeSeed() -> std::uint64_t {
        return (static_cast<std::uint64_t>(masterRng_()) << 32) | masterRng_();
    }

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