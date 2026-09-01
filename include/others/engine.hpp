#pragma once

#include <cstddef>
#include <cstdint>
#include <type_traits>
#include <utility>
#include <variant>
#include <vector>

#include "agents/capital_firm.hpp"
#include "agents/common.hpp"
#include "agents/goods_firm.hpp"
#include "agents/hhold.hpp"
#include "others/logger.hpp"
#include "others/setting.hpp"
#include "others/util.hpp"
#include "values/date.hpp"
#include "world/drop_box.hpp"

namespace abm {
template <typename... Agents>
    requires(IAgent<Agents> and ...)
class AgentRegistry final {
    using Agent = std::variant<Agents...>;

  public:
    AgentRegistry() noexcept = default;

    void reserve(const std::size_t n) noexcept { agents_.reserve(n); }

    template <typename Type, typename... Args>
        requires((std::is_same_v<Type, Agents> or ...) and std::is_constructible_v<Type, Args...>)
    void emplaceBack(Args&&... args) noexcept {
        agents_.emplace_back(std::in_place_type<Type>, std::forward<Args>(args)...);
    }

    void act(const Date& date, MarketRegistry& markets) noexcept {
        for (auto& anyAgent : agents_) {
            anyAgent.visit([&](auto& agent) noexcept -> void { agent.act(date, markets); });
        }
    }

  private:
    std::vector<Agent> agents_;
};

class Engine final {
    using Agent = AgentRegistry<CapitalFirm, GoodsFirm, HHold>;

  public:
    [[nodiscard]] explicit Engine(const Date endingDay)
        : seed_{generateSeed()}, rng_{{seed_.state, seed_.stream}}, endingDay_{endingDay} {
        namespace cnt = setting::agent_count;
        auto id       = 0;

        agents_.reserve(cnt::capitalFirm + cnt::goodsFirm + cnt::hhold);
        for (; id < cnt::capitalFirm; ++id) {
            agents_.emplaceBack<CapitalFirm>(AgentID{id}, rng_);
        }
        for (; id < cnt::goodsFirm + cnt::goodsFirm; ++id) {
            agents_.emplaceBack<GoodsFirm>(AgentID{id}, rng_);
        }
        for (; id < cnt::capitalFirm + cnt::goodsFirm + cnt::hhold; ++id) {
            agents_.emplaceBack<HHold>(AgentID{id}, rng_);
        }
    }

    void run() noexcept {
        for (; today_ < endingDay_; ++today_) {
            agents_.act(today_, markets_);
        }
    }

  private:
    [[nodiscard]] static constexpr auto generateSeed() noexcept -> PCG32Seed {
        if constexpr (not setting::useRuntimeRandomSeed) {
            return {.state = setting::fixedSeedState, .stream = setting::fixedSeedStream};
        }
        auto       rd     = std::random_device{};
        const auto state  = std::uint64_t{(static_cast<std::uint64_t>(rd()) << 32) | rd()};
        const auto stream = std::uint64_t{(static_cast<std::uint64_t>(rd()) << 32) | rd()};
        return {.state = state, .stream = stream};
    }

    const PCG32Seed seed_;
    RandomGenerator rng_;

    Logger logger_;

    Agent agents_;

    MarketRegistry markets_;
    CensusDropBox  dropBox_;

    const Date endingDay_;
    Date       today_{1, 1, 1};
};
}  // namespace abm