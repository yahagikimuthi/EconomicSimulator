#include "engine.hpp"

#include <tbb/concurrent_vector.h>
#include <algorithm>
#include <execution>
#include <highfive/H5DataSet.hpp>
#include <highfive/H5File.hpp>
#include <ranges>
#include <utility>
#include <vector>

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
}  // namespace abm