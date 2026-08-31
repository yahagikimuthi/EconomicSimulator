#pragma once

#include "components/base_goods_supplier/common.hpp"
#include "components/base_goods_supplier/employ_planner.hpp"
#include "others/setting.hpp"
#include "others/util.hpp"
#include "values/goods.hpp"
#include "values/labor.hpp"
#include "values/math.hpp"
#include "world/base_goods.hpp"
#include "world/common.hpp"

namespace abm::base_goods::supplier {
// TODO 生産関数を要検討
class Producer final {
  public:
    [[nodiscard]] explicit Producer(RandomGenerator& masterRng) noexcept
        : baseProductPower_{masterRng.random(setting::productPower)},
          producerGoodsEfficiency_{masterRng.random(setting::producerGoodsEfficiency)},
          producerGoodsDepreciationRate_{masterRng.random(setting::producerGoodsDepreciationRate)} {
    }

    [[nodiscard]] auto produce() noexcept -> GoodsQuantity {
        const auto workerInput = workspace_.totalInput();
        ASSERT(workerInput >= GoodsQuantity{0.0});

        workspace_.resetInput();
        const auto capitalEquipInput = capital_ * producerGoodsEfficiency_;
        ASSERT(capital_ >= GoodsQuantity{0.0});
        capital_ *= (1.0 - producerGoodsDepreciationRate_);

        const auto input = baseProductPower_ * min(workerInput, capitalEquipInput);
        return input;
    }

    void addProducingEquip(const GoodsQuantity capital) noexcept {
        ASSERT(capital >= GoodsQuantity{0.0});
        capital_ += capital;
    }

    [[nodiscard]] auto baseProductPower() const noexcept -> double { return baseProductPower_; }

    [[nodiscard]] auto calcDesiredCapital(const GoodsQuantity requiresSupply
    ) const noexcept -> GoodsQuantity {
        return max(
            (requiresSupply / (baseProductPower_ * producerGoodsEfficiency_)) - capital_,
            GoodsQuantity{0.0}
        );
    }

    [[nodiscard]] auto workspace() noexcept -> Workspace& { return workspace_; }

  private:
    Workspace     workspace_;
    const double  baseProductPower_;
    const double  producerGoodsEfficiency_;
    const double  producerGoodsDepreciationRate_;
    GoodsQuantity capital_{0.0};
};

class ProducingSystem final {
  public:
    [[nodiscard]] explicit ProducingSystem(RandomGenerator& masterRng) noexcept
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
        ASSERT(inventory_ >= GoodsQuantity{0.0});
        inventory_ = GoodsQuantity{0.0};
        ASSERT(out >= GoodsQuantity{0.0});
        return out;
    }

    void listenTradeResult(const TradeResult& result) noexcept {
        inventory_ += result.unsoldAmount;
    }

    void reset(CensusDropBox& dropBox) noexcept {
        dropBox.inventories.emplace_back(inventory_.value());
        employPlanner_.reset();
    }

  private:
    EmployPlanner employPlanner_;
    Producer      producer_;
    GoodsQuantity inventory_;
};
}  // namespace abm::base_goods::supplier