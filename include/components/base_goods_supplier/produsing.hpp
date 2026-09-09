#pragma once

#include <algorithm>
#include <cmath>

#include "components/base_goods_supplier/common.hpp"
#include "components/base_goods_supplier/employ_planner.hpp"
#include "others/setting.hpp"
#include "others/util.hpp"
#include "values/goods.hpp"
#include "values/labor.hpp"
#include "world/base_goods.hpp"

namespace abm::base_goods::supplier {
// TODO 生産関数を要検討
class Producer final {
  public:
    explicit Producer(RandomGenerator& masterRng) noexcept
        : baseProductPower_{masterRng.random(setting::productPower)},
          capitalDepreciationRate_{masterRng.random(setting::capitalDepreciationRate)},
          capitalDistributionRate_{masterRng.random(setting::capitalDistributionRate)} {}

    [[nodiscard]] auto produce() noexcept -> GoodsQuantity {
        const auto workerInput = workspace_.takeout();
        assert(workerInput.isZeroOrMore());
        lastWorkerInput_ = workerInput;

        const auto capitalInput = capital_;
        assert(capital_.isZeroOrMore());
        capital_ *= (1.0 - capitalDepreciationRate_);

        const auto input = baseProductPower_ *
                           std::pow(capitalInput.value(), capitalDistributionRate_) *
                           std::pow(workerInput.value(), 1.0 - capitalDistributionRate_);
        return GoodsQuantity{input};
    }

    void addProducingEquip(const GoodsQuantity capital) noexcept {
        assert(capital.isZeroOrMore());
        capital_ += capital;
    }

    [[nodiscard]] auto baseProductPower() const noexcept -> double { return baseProductPower_; }

    [[nodiscard]] auto calcDesiredCapital(const GoodsQuantity requiresSupply
    ) const noexcept -> GoodsQuantity {
        const auto bottom =
            requiresSupply / (baseProductPower_ *
                              std::pow(lastWorkerInput_.value(), 1.0 - capitalDistributionRate_));
        const auto demand = std::pow(bottom.value(), 1.0 / capitalDistributionRate_);
        const auto out    = GoodsQuantity{demand} - capital_;
        return std::max(out, GoodsQuantity{0.0});
    }

    [[nodiscard]] auto workspace() noexcept -> Workspace& { return workspace_; }

  private:
    Workspace     workspace_;
    GoodsQuantity lastWorkerInput_{0.0};
    const double  baseProductPower_;
    const double  capitalDepreciationRate_;
    const double  capitalDistributionRate_;
    GoodsQuantity capital_{0.0};
};

class ProducingSystem final {
  public:
    explicit ProducingSystem(RandomGenerator& masterRng) noexcept
        : employPlanner_{masterRng},
          producer_{masterRng},
          inventory_{masterRng.random(setting::inventory)} {}

    void acceptMediator(IMediator auto& mediator) noexcept {
        employPlanner_.acceptMediator(mediator);
    }

    [[nodiscard]] auto calcDesiredEmploy(
        const GoodsQuantity requiresSupply, const HeadCount employee
    ) noexcept -> HeadCount {
        return employPlanner_.plan(
            producer_.baseProductPower(), employee, requiresSupply - inventory_
        );
    }

    [[nodiscard]] auto calcDesiredCapital(const GoodsQuantity requiresSupply
    ) const noexcept -> GoodsQuantity {
        return producer_.calcDesiredCapital(requiresSupply);
    }

    void addProducingEquip(const GoodsQuantity capital) noexcept {
        producer_.addProducingEquip(capital);
    }

    [[nodiscard]] auto workspace() noexcept -> Workspace& { return producer_.workspace(); }

    [[nodiscard]] auto produce() noexcept -> GoodsQuantity {
        const auto out = producer_.produce() + inventory_;
        assert(inventory_.isZeroOrMore());
        inventory_ = GoodsQuantity{0.0};
        assert(out.isZeroOrMore());
        return out;
    }

    void listenTradeResult(const TradeResult& result) noexcept {
        inventory_ += result.unsoldAmount;
    }

    void reset() noexcept { employPlanner_.reset(); }

  private:
    EmployPlanner employPlanner_;
    Producer      producer_;
    GoodsQuantity inventory_;
};
}  // namespace abm::base_goods::supplier