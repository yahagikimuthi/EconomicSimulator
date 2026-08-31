#pragma once

#include <vector>

#include "others/agents.hpp"
#include "others/logger.hpp"
#include "others/util.hpp"
#include "system/begining.hpp"
#include "system/capital.hpp"
#include "system/labor.hpp"
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
            for (auto& hhold : hholds_) {
                labor::supplier::work(hhold.laborSupplier);
            }
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
            begining::beginingMonth(
                firm.finance, firm.laborDemander, firm.consumerGoodsSupplier, firm.capitalDemander
            );
        }
        for (auto& firm : capitalFirms_) {
            begining::beginingYear(
                firm.finance, firm.laborDemander, firm.capitalDemander, firm.capitalSupplier
            );
        }
        runLabor();
        runCapital();
        runConsumerGoods();
    }

    void runMonthlyPhase() noexcept {
        for (auto& firm : consumerFirms_) {
            begining::beginingMonth(
                firm.finance, firm.laborDemander, firm.consumerGoodsSupplier, firm.capitalDemander
            );
        }
        for (auto& firm : capitalFirms_) {
            begining::beginingMonth(
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
        using namespace labor;
        for (auto& firm : consumerFirms_) {
            demander::layOffs(firm.laborDemander);
            demander::postLaborRequest(firm.id, firm.laborDemander, laborMarket_);
        }
        for (auto& firm : capitalFirms_) {
            demander::layOffs(firm.laborDemander);
            demander::postLaborRequest(firm.id, firm.laborDemander, laborMarket_);
        }

        for (auto& hhold : hholds_) {
            supplier::laborEntry(hhold.id, hhold.laborSupplier, laborMarket_);
        }

        for (auto& firm : consumerFirms_) {
            demander::offer(firm.laborDemander);
        }
        for (auto& firm : capitalFirms_) {
            demander::offer(firm.laborDemander);
        }

        for (auto& hhold : hholds_) {
            supplier::acceptOffer(hhold.laborSupplier);
        }

        for (auto& firm : consumerFirms_) {
            demander::registerMember(firm.laborDemander, firm.consumerGoodsSupplier);
        }
        for (auto& firm : capitalFirms_) {
            demander::registerMember(firm.laborDemander, firm.capitalSupplier);
        }

        for (auto& hhold : hholds_) {
            supplier::recordRosterEntry(hhold.laborSupplier);
        }

        for (auto& firm : consumerFirms_) {
            demander::acceptResignation(firm.laborDemander);
        }
        for (auto& firm : capitalFirms_) {
            demander::acceptResignation(firm.laborDemander);
        }
    }

    void runCapital() noexcept {
        using namespace capital;
        for (auto& firm : capitalFirms_) {
            supplier::supplyCapital(firm.id, firm.capitalSupplier, capitalMarket_);
        }

        for (auto& firm : capitalFirms_) {
            demander::purchaseCapital(firm.id, firm.capitalDemander, capitalMarket_);
        }
        for (auto& firm : consumerFirms_) {
            demander::purchaseCapital(firm.id, firm.capitalDemander, capitalMarket_);
        }

        for (auto& firm : capitalFirms_) {
            supplier::tradeCapital(firm.capitalSupplier);
        }

        for (auto& firm : capitalFirms_) {
            supplier::afterCapitalTrade(firm.capitalDemander);
        }
        for (auto& firm : consumerFirms_) {
            supplier::afterCapitalTrade(firm.capitalDemander);
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