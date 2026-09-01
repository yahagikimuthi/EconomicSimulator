#pragma once

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
class Engine final {
  public:
    [[nodiscard]] explicit Engine(const Date endingDay)
        : seed_{generateSeed()}, rng_{{seed_.state, seed_.stream}}, endingDay_{endingDay} {
        namespace cnt = setting::agent_count;
        auto id       = 0;
        capitalFirms_.reserve(cnt::capitalFirm);
        for (; id < cnt::capitalFirm; ++id) {
            capitalFirms_.emplace_back(AgentID{id}, rng_);
        }

        goodsFirms_.reserve(cnt::consumerFirm + 5);
        for (; id < cnt::capitalFirm + cnt::consumerFirm; ++id) {
            goodsFirms_.emplace_back(AgentID{id}, rng_);
        }

        hholds_.reserve(cnt::hhold + 10);
        for (; id < cnt::capitalFirm + cnt::consumerFirm + cnt::hhold; ++id) {
            hholds_.emplace_back(AgentID{id}, rng_);
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

    std::vector<HHold>       hholds_;
    std::vector<GoodsFirm>   goodsFirms_;
    std::vector<CapitalFirm> capitalFirms_;

    Markets       markets_;
    CensusDropBox dropBox_;

    const Date endingDay_;
    Date       today_{1, 1, 1};
};
}  // namespace abm