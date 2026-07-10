#pragma once

#include <tbb/concurrent_vector.h>
#include <entt/entt.hpp>
#include <highfive/H5DataSet.hpp>
#include <highfive/H5File.hpp>
#include <string>

#include "world/message.hpp"

namespace core {
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

    entt::registry registry_;

    tbb::concurrent_vector<world::LaborRequest> laborRequestBox_;
    tbb::concurrent_vector<world::GoodsEntry>   goodsEntryBox_;

    world::CensusDropBox dropBox_;

    const int totalStep_;
    int       currentStep_{};
};
}  // namespace core