#pragma once

#include <concepts>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

#include "analysis/data_manager.hpp"

namespace analysis {

class [[nodiscard]] DataContext {
  public:
    void set(const std::string& name, std::vector<double>& data) {
        cache_.try_emplace(name, std::move(data));
    }

    auto get(const std::string& name) const -> const std::vector<double>& {
        const std::vector<double>* out{tryGet(name)};
        if (out == nullptr) return noneData_;
        return *out;
    }

    void clear() { cache_.clear(); }

  private:
    auto tryGet(const std::string& name) const -> const std::vector<double>* {
        auto it = cache_.find(std::string(name));
        if (it == cache_.end()) {
            std::cerr << "required data not found in context: " << name << '\n';
            return nullptr;
        }
        return &(it->second);
    }

    std::unordered_map<std::string, std::vector<double>> cache_;
    std::vector<double>                                  noneData_;
};

template <typename Logic>
concept LogicType = requires(Logic logic, const DataContext& ctx) {
    { logic(ctx) } -> std::same_as<double>;
};

class IMetricTask {
  public:
    IMetricTask(std::string&& outName) : outName_{std::move(outName)} {}

    virtual ~IMetricTask()                             = default;
    IMetricTask(const IMetricTask&)                    = default;
    auto operator=(const IMetricTask&) -> IMetricTask& = delete;
    IMetricTask(IMetricTask&&)                         = default;
    auto operator=(IMetricTask&&) -> IMetricTask&      = delete;

    void         reserve(const std::size_t n) { results_.reserve(n); }
    virtual void process(const DataContext& ctx) = 0;
    void         writeResult(OutputDataManager& output) { output.write(outName_, results_); }

  protected:
    void pushBackData(const double data) { results_.emplace_back(data); }

  private:
    std::vector<double> results_;
    const std::string   outName_;
};

template <LogicType Logic>
class MetricTask final : public IMetricTask {
  public:
    MetricTask(std::string&& outName, Logic logic)
        : IMetricTask(std::move(outName)), logic_{logic} {}

    void process(const DataContext& ctx) override { pushBackData(logic_(ctx)); }

  private:
    const Logic logic_;
};
}  // namespace analysis