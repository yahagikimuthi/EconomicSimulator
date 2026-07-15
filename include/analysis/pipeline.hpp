#pragma once

#include <memory>
#include <string>

#include "analysis/context_task.hpp"
#include "analysis/data_manager.hpp"

namespace analysis {

class Pipeline {
  public:
    void requireData(std::string name) { requireDatas_.emplace_back(std::move(name)); }

    template <LogicType Logic>
    void registerMetric(std::string outName, Logic logic) {
        tasks_.emplace_back(std::make_unique<MetricTask<Logic>>(std::move(outName), logic));
    }

    void execute();

  private:
    InputDataManager  inputManager_;
    OutputDataManager outputManager_;

    std::vector<std::string>                  requireDatas_;
    std::vector<std::unique_ptr<IMetricTask>> tasks_;
};
}  // namespace analysis