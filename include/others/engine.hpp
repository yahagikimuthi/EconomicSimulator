#pragma once

#include <pcg_random.hpp>
#include <vector>

#include "others/agents.hpp"
#include "others/logger.hpp"
#include "others/setting.hpp"
#include "others/util.hpp"
#include "system/begining.hpp"
#include "system/capital.hpp"
#include "system/labor.hpp"
#include "values/date.hpp"
#include "world/base_goods.hpp"
#include "world/drop_box.hpp"
#include "world/labor.hpp"

namespace abm {
class Engine final {
  public:
    [[nodiscard]] explicit Engine(const Date endingDay)
        : seed_{generateSeed()}, rng_{pcg32{seed_.state, seed_.stream}}, endingDay_{endingDay} {
        namespace cnt = setting::agent_count;
        auto id       = 0;
        capitalFirms_.reserve(cnt::capitalFirm);
        for (; id < cnt::capitalFirm; ++id) {
            capitalFirms_.emplace_back(AgentID{id}, rng_);
        }
        for (auto& firm : capitalFirms_) {
            firm.laborDemander.setMediator();
            firm.goodsSupplier.setMediator();
        }

        consumerFirms_.reserve(cnt::consumerFirm + 5);
        for (; id < cnt::capitalFirm + cnt::consumerFirm; ++id) {
            consumerFirms_.emplace_back(AgentID{id}, rng_);
        }
        for (auto& firm : consumerFirms_) {
            firm.laborDemander.setMediator();
            firm.goodsSupplier.setMediator();
        }

        hholds_.reserve(cnt::hhold + 10);
        for (; id < cnt::capitalFirm + cnt::consumerFirm + cnt::hhold; ++id) {
            hholds_.emplace_back(AgentID{id}, rng_);
        }
    }

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
        for (auto& hhold : hholds_) {
            labor::supplier::work(hhold.laborSupplier);
        }
        for (auto& firm : consumerFirms_) {
            begining::beginingMonth(
                firm.finance, firm.laborDemander, firm.goodsSupplier, firm.capitalDemander
            );
        }
        for (auto& firm : capitalFirms_) {
            begining::beginingYear(
                firm.finance, firm.laborDemander, firm.capitalDemander, firm.goodsSupplier
            );
        }
        runLabor();
        runCapital();
        runGoods();
    }

    void runMonthlyPhase() noexcept {
        for (auto& hhold : hholds_) {
            labor::supplier::work(hhold.laborSupplier);
        }
        for (auto& firm : consumerFirms_) {
            begining::beginingMonth(
                firm.finance, firm.laborDemander, firm.goodsSupplier, firm.capitalDemander
            );
        }
        for (auto& firm : capitalFirms_) {
            begining::beginingMonth(
                firm.finance, firm.laborDemander, firm.capitalDemander, firm.goodsSupplier
            );
        }
        runCapital();
        runGoods();
    }

    void runDailyPhase() noexcept {
        for (auto& hhold : hholds_) {
            labor::supplier::work(hhold.laborSupplier);
        }
        runCapital();
        runGoods();
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
            demander::registerMember(firm.laborDemander, firm.goodsSupplier);
        }
        for (auto& firm : capitalFirms_) {
            demander::registerMember(firm.laborDemander, firm.goodsSupplier);
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
            supplier::supplyCapital(firm.id, firm.goodsSupplier, capitalMarket_);
        }

        for (auto& firm : capitalFirms_) {
            demander::purchaseCapital(firm.id, firm.finance, firm.capitalDemander, capitalMarket_);
        }
        for (auto& firm : consumerFirms_) {
            demander::purchaseCapital(firm.id, firm.finance, firm.capitalDemander, capitalMarket_);
        }

        for (auto& firm : capitalFirms_) {
            supplier::tradeCapital(firm.finance, firm.goodsSupplier);
        }

        for (auto& firm : capitalFirms_) {
            supplier::afterCapitalTrade(firm.finance, firm.capitalDemander);
        }
        for (auto& firm : consumerFirms_) {
            supplier::afterCapitalTrade(firm.finance, firm.capitalDemander);
        }
    }

    void runGoods() noexcept {}

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

    std::vector<HHold> hholds_;
    std::vector<Firm>  consumerFirms_;
    std::vector<Firm>  capitalFirms_;

    LaborMarket     laborMarket_;
    BaseGoodsMarket goodsMarket_;
    BaseGoodsMarket capitalMarket_;
    CensusDropBox   dropBox_;

    const Date endingDay_;
    Date       today_{1U, 1U, 1U};
};
}  // namespace abm