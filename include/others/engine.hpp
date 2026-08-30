#pragma once

#include <vector>

#include "agents/capital_firm.hpp"
#include "agents/consumer_firm.hpp"
#include "agents/hhold.hpp"
#include "others/logger.hpp"
#include "others/util.hpp"
#include "values/date.hpp"
#include "world/capital.hpp"
#include "world/common.hpp"
#include "world/consumer_goods.hpp"
#include "world/labor.hpp"

namespace abm {
class Engine final {
    template <typename T>
    using TBBVec = tbb::concurrent_vector<T>;

  public:
    [[nodiscard]] explicit Engine(const Date endingDay) noexcept;

    void run() noexcept {
        for (; today_ < endingDay_; ++today_) {
            if (today_.isBeginingYear()) {
                runYearlyPhase();
            } else if (today_.isBeginingMonth()) {
                runMonthlyPhase();
            } else {
                runDailyPhase();
            }
        }
    }

  private:
    void runYearlyPhase() noexcept {
        for (auto& firm : consumerFirms_) {
            firm.beginingYear();
        }
        for (auto& firm : capitalFirms_) {
            firm.beginingYear();
        }
        runLabor();
        runCapital();
        runConsumerGoods();
    }

    void runMonthlyPhase() noexcept {}
    void runDailyPhase() noexcept {}

    void runLabor() noexcept {}
    void runCapital() noexcept {}
    void runConsumerGoods() noexcept {}

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

    std::vector<HHold>        hholds_;
    std::vector<ConsumerFirm> consumerFirms_;
    std::vector<CapitalFirm>  capitalFirms_;

    LaborMarket         laborMarket_;
    ConsumerGoodsMarket consumerGoodsMarket_;
    CapitalMarket       capitalMarket_;
    CensusDropBox       dropBox_;

    const Date endingDay_;
    Date       today_{1U, 1U, 1U};
};
}  // namespace abm