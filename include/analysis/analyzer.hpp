#pragma once

#include <memory>
#include <string>
#include <string_view>
#include <vector>

#include "analysis/context_task.hpp"
#include "analysis/data_manager.hpp"

namespace abm::analysis {

class Analyzer final {
  public:
    [[nodiscard]] constexpr Analyzer() noexcept = default;
    void requireData(std::string_view name) noexcept { requireDatas_.emplace_back(name); }

    template <LogicType Logic>
    void registerMetric(std::string&& outName, Logic logic) noexcept {
        tasks_.emplace_back(std::make_unique<MetricTask<Logic>>(std::move(outName), logic));
    }

    void execute() {
        auto stepKeys   = inputManager_.getStepKeys();
        auto stepNumVec = std::vector<double>{};
        stepNumVec.reserve(stepKeys.size());
        for (auto& task : tasks_) {
            task->reserve(stepKeys.size());
        }

        auto ctx = DataContext{};
        for (const std::string& stepKey : stepKeys) {
            stepNumVec.emplace_back(std::stoi(stepKey.substr(5)));  // NOLINT

            for (std::string_view requireData : requireDatas_) {
                auto data = std::vector<double>{};
                inputManager_.read(stepKey, requireData, data);
                ctx.set(requireData, std::move(data));
            }

            for (auto& task : tasks_) {
                task->process(ctx);
            }
            ctx.clear();
        }

        outputManager_.write("step", std::move(stepNumVec));
        for (auto& task : tasks_) {
            task->writeResult(outputManager_);
        }
    }

  private:
    InputDataManager  inputManager_;
    OutputDataManager outputManager_;

    std::vector<std::string_view>             requireDatas_;
    std::vector<std::unique_ptr<IMetricTask>> tasks_;
};
}  // namespace abm::analysis