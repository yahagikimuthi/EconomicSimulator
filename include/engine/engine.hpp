#pragma once

#include <algorithm>
#include <cstdint>
#include <functional>
#include <print>
#include <ranges>
#include <vector>

#include "components/government.hpp"
#include "engine/agents.hpp"
#include "engine/labor_engine.hpp"
#include "engine/logger.hpp"
#include "others/setting.hpp"
#include "others/util.hpp"
#include "system/capital.hpp"
#include "system/end_month.hpp"
#include "system/goods.hpp"
#include "system/planning.hpp"
#include "world/base_goods.hpp"
#include "world/drop_box.hpp"

namespace abm::engine {
class Engine final {
  public:
    [[nodiscard]] explicit Engine(const int endStep)
        : seed_{generateSeed()},
          rng_{{seed_.state, seed_.stream}},
          endMonth_{endStep},
          laborEngine_{dropBox_.labor} {
        namespace cnt = global_setting::agent_count;

        capitalFirms_.reserve(cnt::capitalFirm);
        for (const auto _ : std::views::indices(cnt::capitalFirm)) capitalFirms_.emplace_back(rng_);

        goodsFirms_.reserve(cnt::goodsFirm);
        for (const auto _ : std::views::indices(cnt::goodsFirm)) goodsFirms_.emplace_back(rng_);

        hholds_.reserve(cnt::hhold);
        for (const auto _ : std::views::indices(cnt::hhold)) hholds_.emplace_back(rng_);
    }

    void run() noexcept {
        for (const auto month : std::views::indices(endMonth_)) {
            if (month % global_setting::monthInYear == 0) {
                runJanuaryPlanning();
                laborEngine_.run(capitalFirms_, goodsFirms_, hholds_);
            } else
                runStandardPlanning();
            runCapital();
            runGoods();
            runEndMonth();
            logger_.save(dropBox_, month);
            dropBox_.clear();

            std::println("{}", calcSumAsset());
        }
    }

  private:
    void runJanuaryPlanning() noexcept {
        using namespace planning;

        for (auto& firm : capitalFirms_) {
            planJanuary(
                firm.finance, firm.laborDemander, firm.capitalDemander, firm.capitalSupplier
            );
        }

        for (auto& firm : goodsFirms_) {
            planJanuary(firm.finance, firm.laborDemander, firm.capitalDemander, firm.goodsSupplier);
        }

        for (auto& hhold : hholds_) {
            planJanuary(hhold.finance, hhold.goods);
        }
    }

    void runStandardPlanning() noexcept {
        using namespace planning;

        for (auto& firm : capitalFirms_) {
            planStandard(
                firm.finance, firm.laborDemander, firm.capitalDemander, firm.capitalSupplier
            );
        }

        for (auto& firm : goodsFirms_) {
            planStandard(
                firm.finance, firm.laborDemander, firm.capitalDemander, firm.goodsSupplier
            );
        }

        for (auto& hhold : hholds_) {
            planStandard(hhold.finance, hhold.goods);
        }
    }

    void runCapital() noexcept {
        using namespace capital;
        for (auto& firm : capitalFirms_) {
            entry(firm.id, firm.capitalSupplier, capitalMarket_);
        }

        for (auto& firm : capitalFirms_) {
            request(firm.id, firm.finance, firm.capitalDemander, capitalMarket_);
        }
        for (auto& firm : goodsFirms_) {
            request(firm.id, firm.finance, firm.capitalDemander, capitalMarket_);
        }

        for (auto& firm : capitalFirms_) {
            trade(firm.finance, firm.capitalSupplier, government_);
        }

        for (auto& firm : capitalFirms_) {
            afterTrade(firm.finance, firm.capitalDemander, firm.capitalSupplier);
        }
        for (auto& firm : goodsFirms_) {
            afterTrade(firm.finance, firm.capitalDemander, firm.goodsSupplier);
        }

        capitalMarket_.clear();

        for (auto& firm : capitalFirms_) {
            logging(dropBox_.capital, firm.capitalSupplier);
        }
    }

    void runGoods() noexcept {
        using namespace goods;
        for (auto& firm : goodsFirms_) {
            entry(firm.id, firm.goodsSupplier, goodsMarket_);
        }

        for (auto& hhold : hholds_) {
            request(hhold.id, hhold.finance, hhold.goods, goodsMarket_);
        }

        for (auto& firm : goodsFirms_) {
            trade(firm.finance, firm.goodsSupplier, government_);
        }

        for (auto& hhold : hholds_) {
            afterTrade(hhold.finance, hhold.goods);
        }

        goodsMarket_.clear();

        for (auto& firm : goodsFirms_) {
            logging(dropBox_.goods, firm.goodsSupplier);
        }
    }

    void runEndMonth() noexcept {
        using namespace end_month;
        for (auto& firm : capitalFirms_) {
            payWage(firm.finance, firm.laborDemander, government_);
        }
        for (auto& firm : goodsFirms_) {
            payWage(firm.finance, firm.laborDemander, government_);
        }

        for (auto& hhold : hholds_) {
            workAndReceiveWage(hhold.finance, hhold.labor);
        }

        for (auto& firm : capitalFirms_) {
            finalizeAccounts(firm.finance, government_);
        }
        for (auto& firm : goodsFirms_) {
            finalizeAccounts(firm.finance, government_);
        }

        for (auto& hhold : hholds_) {
            provideUnemploymentBenefit(hhold.finance, hhold.labor, government_);
        }
    }

    [[nodiscard]] static auto calcSumAssert(const auto& agents) noexcept -> double {
        return std::ranges::fold_left(
            agents | std::views::transform([](const auto& agent) noexcept -> double {
                return agent.finance.asset().value();
            }),
            0.0,
            std::plus{}
        );
    }

    [[nodiscard]] auto calcSumAsset() noexcept -> double {
        return calcSumAssert(capitalFirms_) + calcSumAssert(goodsFirms_) + calcSumAssert(hholds_);
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

    const int endMonth_;

    Logger logger_;

    std::vector<CapitalFirm> capitalFirms_;
    std::vector<GoodsFirm>   goodsFirms_;
    std::vector<HHold>       hholds_;
    Government               government_;

    CensusDropBox dropBox_;
    LaborEngine   laborEngine_;
    CapitalMarket capitalMarket_;
    GoodsMarket   goodsMarket_;
};
}  // namespace abm::engine

namespace abm {
using Engine = engine::Engine;
}