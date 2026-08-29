#pragma once

#include <atomic>

#include "core/assertion.hpp"
#include "values/goods.hpp"

namespace abm::base_goods {
class Workspace final {
  public:
    [[nodiscard]] constexpr Workspace() noexcept = default;
    ~Workspace() noexcept                        = default;
    [[nodiscard]] Workspace(const Workspace& other) noexcept
        : totalInput_{other.totalInput_.load()} {}
    auto operator=(const Workspace& other) noexcept -> Workspace& {
        if (this == &other) return *this;
        const auto input = other.totalInput_.load();
        totalInput_.store(input);
        return *this;
    }
    [[nodiscard]] Workspace(Workspace&& other) noexcept : totalInput_{other.totalInput_.load()} {
        other.totalInput_.store(0.0);
    }
    auto operator=(Workspace&& other) noexcept -> Workspace& {
        if (this == &other) return *this;
        const double input{other.totalInput_.load()};
        totalInput_.store(input);

        other.totalInput_.store(0.0);
        return *this;
    }

    void addInput(const double workerProductPower) noexcept {
        ASSERT(workerProductPower > 0.0);
        totalInput_.fetch_add(workerProductPower);
    }
    [[nodiscard]] auto totalInput() const noexcept -> GoodsQuantity {
        return GoodsQuantity{totalInput_.load()};
    }
    void resetInput() noexcept { totalInput_.store(0.0); }

  private:
    std::atomic<double> totalInput_;
};
}  // namespace abm::base_goods