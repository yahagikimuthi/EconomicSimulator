#pragma once

#include <tbb/concurrent_vector.h>
#include <entt/entt.hpp>

#include "world/message.hpp"

namespace core {
struct HHoldTag {};
struct FirmTag {};

class Engine {
  public:
    Engine(const int totalStep);
    void run();

  private:
    void runLabor();
    void runGoods();
    void reset();

    entt::registry registry_;

    tbb::concurrent_vector<world::LaborRequest> laborRequestBox_;
    tbb::concurrent_vector<world::GoodsEntry>   goodsEntryBox_;

    const int totalStep_;
    int       currentStep_{};
};
}  // namespace core