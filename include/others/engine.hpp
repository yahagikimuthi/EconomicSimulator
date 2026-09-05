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
#include "world/labor.hpp"

namespace abm {
template <typename... Agents>
    requires(IAgent<Agents> and ...)
class AgentRegistry final {
  public:
    explicit AgentRegistry() noexcept = default;

    void reserve(const std::size_t n) noexcept { agents_.reserve(n); }

    template <typename Type, typename... Args>
        requires((std::is_same_v<Type, Agents> or ...) and std::is_constructible_v<Type, Args...>)
    void emplaceBack(Args&&... args) noexcept {
        agents_.emplace_back(std::in_place_type<Type>, std::forward<Args>(args)...);
    }

    void act(const Date& date, MarketRegistry& markets) noexcept {
        for (auto& anyAgent : agents_)
            anyAgent.visit([&](auto& agent) noexcept -> void { agent.act(date, markets); });
    }

    [[nodiscard]] auto sumAsset() const noexcept -> Budget {
        auto sum = Budget{0.0};
        for (auto& anyAgent : agents_)
            sum += anyAgent.visit([](auto& agent) noexcept -> Budget { return agent.asset(); });
        return sum;
    }

  private:
    std::vector<std::variant<Agents...>> agents_;
};

class Engine final {
  public:
    [[nodiscard]] explicit Engine(const Date endingDay)
        : seed_{generateSeed()}, rng_{{seed_.state, seed_.stream}}, endingDay_{endingDay} {
        namespace cnt = global_setting::agent_count;
        auto count    = 0;

        capitalFirms_.reserve(cnt::capitalFirm);
        for (; count < cnt::capitalFirm; ++count) {
            capitalFirms_.emplace_back(rng_);
        }

        goodsFirms_.reserve(cnt::goodsFirm);
        for (; count < cnt::capitalFirm + cnt::goodsFirm; ++count) {
            goodsFirms_.emplace_back(rng_);
        }

        hholds_.reserve(cnt::hhold);
        for (; count < cnt::capitalFirm + cnt::goodsFirm + cnt::hhold; ++count) {
            hholds_.emplace_back(rng_);
        }
    }

    void run() noexcept {
        for (; today_ < endingDay_; ++today_) {
            for (auto& firm : capitalFirms_) {
                firm.act(today_, markets_);
            }
            for (auto& firm : goodsFirms_) {
                firm.act(today_, markets_);
            }
            for (auto& hhold : hholds_) {
                hhold.act(today_, markets_);
            }
            if (labor::toMarketPhase(today_.month()) == LaborMarketPhase::RecordRosterEntry)
                markets_.laborMarket.clear();
        }
    }

  private:
    [[nodiscard]] static constexpr auto generateSeed() noexcept -> PCG32Seed {
        if constexpr (not global_setting::useRuntimeRandomSeed) {
            return {
                .state = global_setting::fixedSeedState, .stream = global_setting::fixedSeedStream
            };
        }
        auto       rd     = std::random_device{};
        const auto state  = std::uint64_t{(static_cast<std::uint64_t>(rd()) << 32) | rd()};
        const auto stream = std::uint64_t{(static_cast<std::uint64_t>(rd()) << 32) | rd()};
        return {.state = state, .stream = stream};
    }

    const PCG32Seed seed_;
    RandomGenerator rng_;

    const Date endingDay_;
    Date       today_{1};

    Logger logger_;

    std::vector<CapitalFirm> capitalFirms_;
    std::vector<GoodsFirm>   goodsFirms_;
    std::vector<HHold>       hholds_;

    MarketRegistry markets_;
    CensusDropBox  dropBox_;
};
}  // namespace abm