#pragma once

#include <concepts>
#include <iostream>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <vector>

#include "analysis/data_manager.hpp"

namespace abm::analysis {
class DataContext {
  public:
    [[nodiscard]] DataContext() noexcept = default;
    void set(std::string_view name, std::vector<double>&& data) noexcept {
        cache_.try_emplace(name, std::move(data));
    }

    [[nodiscard]] auto get(std::string_view name) const noexcept -> const std::vector<double>& {
        if (const auto* out = tryGet(name); out != nullptr) return *out;
        std::cerr << "this function return empty vector(size=0) now\n";
        return noneData_;
    }

    void clear() noexcept { cache_.clear(); }

  private:
    [[nodiscard]] auto tryGet(std::string_view name) const noexcept -> const std::vector<double>* {
        auto it = cache_.find(name);
        if (it == cache_.end()) {
            std::cerr << "required data not found in context: " << name << '\n';
            return nullptr;
        }
        return &it->second;
    }

    std::unordered_map<std::string_view, std::vector<double>> cache_;
    const std::vector<double>                                 noneData_;
};

template <typename Logic>
concept LogicType = requires(Logic logic, const DataContext& ctx) {
    { logic(ctx) } -> std::same_as<double>;
};

class IMetricTask {
  public:
    [[nodiscard]] IMetricTask(std::string&& outName) noexcept : outName_{std::move(outName)} {}

    virtual ~IMetricTask() noexcept                             = default;
    IMetricTask(const IMetricTask&) noexcept                    = default;
    auto operator=(const IMetricTask&) noexcept -> IMetricTask& = delete;
    IMetricTask(IMetricTask&&) noexcept                         = default;
    auto operator=(IMetricTask&&) noexcept -> IMetricTask&      = delete;

    void reserve(const std::size_t n) noexcept { results_.reserve(n); }

    virtual void process(const DataContext& ctx) noexcept = 0;

    void writeResult(OutputDataManager& output) {
        output.write(std::move(outName_), std::move(results_));
    }

  protected:
    void pushBackData(const double data) noexcept { results_.emplace_back(data); }

  private:
    std::vector<double> results_;
    std::string         outName_;
};

template <LogicType Logic>
class MetricTask final : public IMetricTask {
  public:
    MetricTask(std::string&& outName, const Logic logic) noexcept
        : IMetricTask(std::move(outName)), logic_{logic} {}

    void process(const DataContext& ctx) noexcept override { pushBackData(logic_(ctx)); }

  private:
    const Logic logic_;
};
}  // namespace abm::analysis