#pragma once

#include <cstdint>
#include <ranges>
#include <vector>

#include "engine/agents.hpp"
#include "engine/logger.hpp"
#include "others/setting.hpp"
#include "others/util.hpp"
#include "system/capital.hpp"
#include "system/goods.hpp"
#include "system/labor.hpp"
#include "system/planning.hpp"
#include "values/date.hpp"
#include "world/base_goods.hpp"
#include "world/drop_box.hpp"
#include "world/labor.hpp"

namespace abm {
class Engine final {
  public:
    [[nodiscard]] explicit Engine(const int endStep)
        : seed_{generateSeed()}, rng_{{seed_.state, seed_.stream}}, endingDay_{endStep} {
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
            runPlanning();
            runLabor();
            runCapital();
            runGoods();
        }
    }

  private:
    void runPlanning() noexcept {
        using namespace planning;

        for (auto& firm : capitalFirms_) {
            plan(firm.finance, firm.laborDemander, firm.capitalDemander, firm.capitalSupplier);
        }
        for (auto& firm : goodsFirms_) {
            plan(firm.finance, firm.laborDemander, firm.capitalDemander, firm.goodsSupplier);
        }
        for (auto& hhold : hholds_) {
            plan(hhold.finance, hhold.goods);
        }
    }

    void runLabor() noexcept {
        using namespace labor;

        for (auto& firm : capitalFirms_) {
            request(firm.id, firm.laborDemander, laborMarket_);
        }
        for (auto& firm : goodsFirms_) {
            request(firm.id, firm.laborDemander, laborMarket_);
        }

        for (auto& hhold : hholds_) {
            entry(hhold.id, hhold.labor, laborMarket_);
        }

        for (auto& firm : capitalFirms_) {
            offer(firm.laborDemander);
        }
        for (auto& firm : goodsFirms_) {
            offer(firm.laborDemander);
        }

        for (auto& hhold : hholds_) {
            accept(hhold.labor);
        }

        for (auto& firm : capitalFirms_) {
            endRecruiting(firm.laborDemander, firm.capitalSupplier);
        }
        for (auto& firm : goodsFirms_) {
            endRecruiting(firm.laborDemander, firm.goodsSupplier);
        }

        for (auto& hhold : hholds_) {
            recordRosterEntry(hhold.finance, hhold.labor);
        }

        laborMarket_.clear();

        for (auto& firm : capitalFirms_) {
            payWage(firm.finance, firm.laborDemander);
        }
        for (auto& firm : goodsFirms_) {
            payWage(firm.finance, firm.laborDemander);
        }

        for (auto& hhold : hholds_) {
            work(hhold.finance, hhold.labor, today_);
        }
    }

    void runCapital() noexcept {
        using namespace capital;
        for (auto& firm : capitalFirms_) {
            entry(firm.id, firm.capitalSupplier, capitalMarket_);
        }

        for (const auto _ : std::views::indices(setting::repeatTransactionCnt)) {
            for (auto& firm : capitalFirms_) {
                request(firm.id, firm.finance, firm.capitalDemander, capitalMarket_);
            }
            for (auto& firm : goodsFirms_) {
                request(firm.id, firm.finance, firm.capitalDemander, capitalMarket_);
            }

            for (auto& firm : capitalFirms_) {
                trade(firm.finance, firm.capitalSupplier);
            }

            for (auto& firm : capitalFirms_) {
                afterTrade(firm.finance, firm.capitalDemander);
            }
            for (auto& firm : goodsFirms_) {
                afterTrade(firm.finance, firm.capitalDemander);
            }
        }

        capitalMarket_.clear();
    }

    void runGoods() noexcept {
        using namespace goods;
        for (auto& firm : goodsFirms_) {
            entry(firm.id, firm.goodsSupplier, goodsMarket_);
        }

        for (const auto _ : std::views::indices(setting::repeatTransactionCnt)) {
            for (auto& hhold : hholds_) {
                request(hhold.id, hhold.finance, hhold.goods, goodsMarket_);
            }

            for (auto& firm : goodsFirms_) {
                trade(firm.finance, firm.goodsSupplier);
            }

            for (auto& hhold : hholds_) {
                afterTrade(hhold.finance, hhold.goods);
            }
        }

        goodsMarket_.clear();
    }

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