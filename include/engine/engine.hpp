#pragma once

#include <cstdint>
#include <ranges>
#include <vector>

#include "engine/agents.hpp"
#include "engine/logger.hpp"
#include "others/setting.hpp"
#include "others/util.hpp"
#include "values/date.hpp"
#include "world/base_goods.hpp"
#include "world/drop_box.hpp"
#include "world/labor.hpp"

namespace abm {
class Engine final {
  public:
    [[nodiscard]] explicit Engine(const Date endingDay)
        : seed_{generateSeed()}, rng_{{seed_.state, seed_.stream}}, endingDay_{endingDay} {
        namespace cnt = global_setting::agent_count;

        capitalFirms_.reserve(cnt::capitalFirm);
        for (const auto _ : std::views::indices(cnt::capitalFirm)) capitalFirms_.emplace_back(rng_);

        goodsFirms_.reserve(cnt::goodsFirm);
        for (const auto _ : std::views::indices(cnt::goodsFirm)) goodsFirms_.emplace_back(rng_);

        hholds_.reserve(cnt::hhold);
        for (const auto _ : std::views::indices(cnt::hhold)) hholds_.emplace_back(rng_);
    }

    void run() noexcept {
        for (; today_ < endingDay_; ++today_) {
        }
    }

  private:
    void runLabor() noexcept {}

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

    LaborMarket   laborMarket_;
    CapitalMarket capitalMarket_;
    GoodsMarket   goodsMarket_;
    CensusDropBox dropBox_;
};
}  // namespace abm