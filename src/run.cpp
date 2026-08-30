#include "engine.hpp"

#include <tbb/concurrent_vector.h>
#include <algorithm>
#include <execution>
#include <highfive/H5DataSet.hpp>
#include <highfive/H5File.hpp>
#include <ranges>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "others/setting.hpp"
#include "values/common.hpp"
#include "world/common.hpp"

namespace abm {
namespace {
template <typename T>
    requires requires(T t) { t.finance.asset().value(); }
[[nodiscard]] auto calcSumAsset(const std::vector<T>& agents) noexcept -> double {
    return std::ranges::fold_left(
        agents | std::views::transform([](const T& agent) noexcept -> double {
            return agent.finance.asset().value();
        }),
        0.0,
        std::plus{}
    );
}

template <typename T, typename F>
    requires std::invocable<F, T&> and requires(std::vector<T>& agents, F&& f) {
        std::for_each(std::execution::par, agents.begin(), agents.end(), std::forward<F>(f));
    }
void forEach(std::vector<T>& agents, F&& f) {
    std::for_each(agents.begin(), agents.end(), std::forward<F>(f));
}
}  // namespace

void Engine::run() noexcept {}

void Logger::save(const CensusDropBox& dropBox, const Step step) noexcept {
    namespace name = setting::save_name;
    auto groupPath = std::string{"/step_" + std::to_string(step.value())};
    auto group     = HighFive::Group{file_.createGroup(groupPath)};

    auto create =
        [&group](std::string_view dataName, const std::vector<double>& data) noexcept -> void {
        group.createDataSet(static_cast<std::string>(dataName), data);
    };

    create(name::firmAssets, dropBox.firmAssets);
    create(name::postedEmployments, dropBox.postedEmployments);
    create(name::postedWages, dropBox.postedWages);
    create(name::employments, dropBox.employments);
    create(name::sumWages, dropBox.sumWages);
    create(name::prices, dropBox.prices);
    create(name::supplies, dropBox.supplies);
    create(name::markups, dropBox.markups);
    create(name::inventories, dropBox.inventories);
    create(name::householdAssets, dropBox.hholdAssets);
    create(name::wages, dropBox.wages);
}
}  // namespace abm