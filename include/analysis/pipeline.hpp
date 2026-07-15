#pragma once

#include <memory>
#include <string>

#include "analysis/context_task.hpp"
#include "analysis/data_manager.hpp"

namespace analysis {

class Pipeline {
  public:
    Pipeline(std::string&& inputPath, std::string&& outputPath)
        : inputManager_{std::move(inputPath)}, outputManager_{std::move(outputPath)} {}

    void requireData(std::string name) { requireDatas_.emplace_back(std::move(name)); }

    template <LogicType Logic>
    void registerMetric(std::string outName, Logic logic) {
        tasks_.emplace_back(std::make_unique<MetricTask<Logic>>(std::move(outName), logic));
    }

    void execute() {
        std::vector<std::string> stepKeys = inputManager_.getStepKeys();
        std::vector<double>      stepNumVec;
        stepNumVec.reserve(stepKeys.size());
        for (auto& task : tasks_) {
            task->reserve(stepKeys.size());
        }

        DataContext ctx;
        for (const std::string& stepKey : stepKeys) {
            stepNumVec.emplace_back(std::stoi(stepKey.substr(5)));

            for (const std::string& requireData : requireDatas_) {
                std::vector<double> data;
                inputManager_.read(stepKey, requireData, data);
                ctx.set(requireData, data);
            }

            for (auto& task : tasks_) {
                task->process(ctx);
            }
            ctx.clear();
        }

        outputManager_.write("step", stepNumVec);
        for (auto& task : tasks_) {
            task->writeResult(outputManager_);
        }
    }

  private:
    InputDataManager  inputManager_;
    OutputDataManager outputManager_;

    std::vector<std::string>                  requireDatas_;
    std::vector<std::unique_ptr<IMetricTask>> tasks_;
};
}  // namespace analysis