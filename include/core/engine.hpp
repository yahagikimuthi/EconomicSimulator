#pragma once

#include <entt/entt.hpp>

namespace core {
struct HHoldTag {};
struct FirmTag {};

class Engine {
    entt::registry registry_;
    const int      totalStep_;
    int            currentStep_{};

  public:
    Engine(const int totalStep);
};
}  // namespace core