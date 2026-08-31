#pragma once

#include <vector>

#include "others/agents.hpp"
#include "others/logger.hpp"
#include "others/util.hpp"
#include "system/capital_firm.hpp"
#include "system/consumer_firm.hpp"
#include "system/hhold.hpp"
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
            consumer_firm::beginingMonth(
                firm.finance, firm.laborDemander, firm.consumerGoodsSupplier, firm.capitalDemander
            );
        }
        for (auto& firm : capitalFirms_) {
            capital_firm::beginingYear(
                firm.finance, firm.laborDemander, firm.capitalDemander, firm.capitalSupplier
            );
        }
        runLabor();
        runCapital();
        runConsumerGoods();
    }

    void runMonthlyPhase() noexcept {
        for (auto& firm : consumerFirms_) {
            consumer_firm::beginingMonth(
                firm.finance, firm.laborDemander, firm.consumerGoodsSupplier, firm.capitalDemander
            );
        }
        for (auto& firm : capitalFirms_) {
            capital_firm::beginingMonth(
                firm.finance, firm.laborDemander, firm.capitalDemander, firm.capitalSupplier
            );
        }
        runCapital();
        runConsumerGoods();
    }

    void runDailyPhase() noexcept {
        runCapital();
        runConsumerGoods();
    }

    void runLabor() noexcept {
        for (auto& firm : consumerFirms_) {
            consumer_firm::layOffs(firm.laborDemander);
            consumer_firm::postLaborRequest(firm.id, firm.laborDemander, laborMarket_);
        }
        for (auto& firm : capitalFirms_) {
            capital_firm::layOffs(firm.laborDemander);
            capital_firm::postLaborRequest(firm.id, firm.laborDemander, laborMarket_);
        }

        for (auto& hhold : hholds_) {
            hhold::laborEntry(hhold.id, hhold.laborSupplier, laborMarket_);
        }

        for (auto& firm : consumerFirms_) {
            consumer_firm::offer(firm.laborDemander);
        }
        for (auto& firm : capitalFirms_) {
            capital_firm::offer(firm.laborDemander);
        }

        for (auto& hhold : hholds_) {
            hhold::acceptOffer(hhold.laborSupplier);
        }

        for (auto& firm : consumerFirms_) {
            consumer_firm::registerMember(firm.laborDemander, firm.consumerGoodsSupplier);
        }
        for (auto& firm : capitalFirms_) {
            capital_firm::registerMember(firm.laborDemander, firm.capitalSupplier);
        }

        for (auto& hhold : hholds_) {
            hhold::recordRosterEntry(hhold.laborSupplier);
        }

        for (auto& firm : consumerFirms_) {
            consumer_firm::acceptResignation(firm.laborDemander);
        }
        for (auto& firm : capitalFirms_) {
            capital_firm::acceptResignation(firm.laborDemander);
        }
    }

    void runCapital() noexcept {
        for (auto& firm : capitalFirms_) {
            capital_firm::supplyCapital(firm.id, firm.capitalSupplier, capitalMarket_);
        }

        for (auto& firm : capitalFirms_) {
            capital_firm::purchaseCapital(firm.id, firm.capitalDemander, capitalMarket_);
        }
        for (auto& firm : consumerFirms_) {
            consumer_firm::purchaseCapital(firm.id, firm.capitalDemander, capitalMarket_);
        }

        for (auto& firm : capitalFirms_) {
            capital_firm::tradeCapital(firm.capitalSupplier);
        }

        for (auto& firm : capitalFirms_) {
            capital_firm::afterCapitalTrade(firm.capitalDemander);
        }
        for (auto& firm : consumerFirms_) {
            consumer_firm::afterCapitalTrade(firm.capitalDemander);
        }
    }

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