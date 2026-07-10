#pragma once

#include <tbb/concurrent_vector.h>
#include <highfive/H5DataSet.hpp>
#include <highfive/H5File.hpp>
#include <string>

#include "components/common.hpp"
#include "components/goods_demander.hpp"
#include "components/goods_supplier.hpp"
#include "components/labor_demander.hpp"
#include "components/labor_supplier.hpp"
#include "world/message.hpp"

namespace core {
struct Firm {
    agent_index::Component    index;
    firm_finance::Component   finance;
    labor_demander::Component labor;
    goods_supplier::Component goods;

    Firm() : index{instanceCnt++} {}

  private:
    static inline int instanceCnt{};
};
struct HHold {
    agent_index::Component    index;
    hhold_finance::Component  finance;
    labor_supplier::Component labor;
    goods_demander::Component goods;

    HHold() : index{instanceCnt++} {}

  private:
    static inline int instanceCnt{};
};
struct HHoldTag {};
struct FirmTag {};

class Logger {
  public:
    explicit Logger(const std::string& filename);
    [[nodiscard]] auto isValid() const -> bool { return file_.isValid(); }

    void save(const world::CensusDropBox& dropBox, const int step);

  private:
    HighFive::File file_;
};

class Engine {
  public:
    explicit Engine(const int totalStep, const std::string& filename);

    void   run();
    Logger logger;

  private:
    void runLabor();
    void runGoods();
    void update();
    void logging();
    void reset();

    // entt::registry     registry_;
    std::vector<Firm>  firms_;
    std::vector<HHold> hholds_;

    tbb::concurrent_vector<world::LaborRequest> laborRequestBox_;
    tbb::concurrent_vector<world::GoodsEntry>   goodsEntryBox_;

    world::CensusDropBox dropBox_;

    const int totalStep_;
    int       currentStep_{};
};
}  // namespace core