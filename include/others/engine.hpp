#pragma once

#include <pcg_random.hpp>
#include <vector>

#include "others/agents.hpp"
#include "others/logger.hpp"
#include "others/setting.hpp"
#include "others/util.hpp"
#include "system/capital.hpp"
#include "system/finance.hpp"
#include "system/goods.hpp"
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
            firm.capitalSupplier.setMediator();
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
            }
        }
    }

  private:
    void runYearly() noexcept {
        for (auto& firm : capitalFirms_) {
            finance::judgeYearlyAndMonthlyBudget(
                firm.finance, firm.laborDemander, firm.capitalDemander, firm.capitalSupplier
            );
        }
        for (auto& firm : consumerFirms_) {
            finance::judgeYearlyAndMonthlyBudget(
                firm.finance, firm.laborDemander, firm.capitalDemander, firm.goodsSupplier
            );
        }
        runYearlyLaborMarket();
    }

    void runYearlyLaborMarket() noexcept {
        using namespace labor;
        for (auto& firm : capitalFirms_) {
            demander::layOffs(firm.laborDemander);
        }
        for (auto& firm : capitalFirms_) {
            demander::layOffs(firm.laborDemander);
        }

        for (auto& firm : capitalFirms_) {
            demander::postRequest(firm.id, firm.laborDemander, laborMarket_);
        }
        for (auto& firm : consumerFirms_) {
            demander::postRequest(firm.id, firm.laborDemander, laborMarket_);
        }

        for (auto& hhold : hholds_) {
            supplier::entry(hhold.id, hhold.laborSupplier, laborMarket_);
        }

        for (auto& firm : capitalFirms_) {
            demander::offer(firm.laborDemander);
        }
        for (auto& firm : consumerFirms_) {
            demander::offer(firm.laborDemander);
        }

        for (auto& hhold : hholds_) {
            supplier::acceptOffer(hhold.laborSupplier);
        }

        for (auto& firm : capitalFirms_) {
            demander::endRecruiting(firm.laborDemander, firm.capitalSupplier);
        }
        for (auto& firm : consumerFirms_) {
            demander::endRecruiting(firm.laborDemander, firm.goodsSupplier);
        }

        for (auto& hhold : hholds_) {
            supplier::recordRosterEntry(hhold.laborSupplier);
        }
    }

    void runMonthlyCapitalMarket() noexcept {
        using namespace capital;
        for (auto& firm : capitalFirms_) {
            supplier::supply(firm.id, firm.capitalSupplier, capitalMarket_);
        }

        for (auto& firm : capitalFirms_) {
            demander::purchase(firm.id, firm.finance, firm.capitalDemander, capitalMarket_);
        }
        for (auto& firm : capitalFirms_) {
            demander::purchase(firm.id, firm.finance, firm.capitalDemander, capitalMarket_);
        }

        for (auto& firm : capitalFirms_) {
            supplier::trade(firm.finance, firm.capitalSupplier);
        }

        for (auto& firm : capitalFirms_) {
            demander::afterTarde(firm.finance, firm.capitalDemander);
        }
        for (auto& firm : consumerFirms_) {
            demander::afterTarde(firm.finance, firm.capitalDemander);
        }
    }

    void runDailyGoodsMarket() noexcept {
        using namespace goods;
        for (auto& firm : consumerFirms_) {
            supplier::supply(firm.id, firm.goodsSupplier, goodsMarket_);
        }
    }

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
    std::vector<GoodsFirm>   consumerFirms_;
    std::vector<CapitalFirm> capitalFirms_;

    LaborMarket     laborMarket_;
    BaseGoodsMarket goodsMarket_;
    BaseGoodsMarket capitalMarket_;
    CensusDropBox   dropBox_;

    const Date endingDay_;
    Date       today_{1U, 1U, 1U};
};
}  // namespace abm