#include "analysis/pipeline.hpp"

namespace analysis {
void Pipeline::execute() {
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
}  // namespace analysis