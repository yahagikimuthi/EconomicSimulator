#pragma once

#include <algorithm>
#include <cassert>

#include "components/base_goods_supplier/common.hpp"
#include "others/setting.hpp"
#include "others/util.hpp"
#include "values/goods.hpp"
#include "values/labor.hpp"
#include "values/math.hpp"
#include "world/base_goods.hpp"
#include "world/drop_box.hpp"

namespace abm::base_goods::supplier {
class CapitalManager final {
  public:
    explicit CapitalManager(const double depreciationRate, const double distributionRate) noexcept
        : depreciationRate_{depreciationRate}, distributionRate_{distributionRate} {}

    [[nodiscard]] auto produce() noexcept -> GoodsQuantity {
        const auto produce = calcProduceAmount();
        capital_ *= (1.0 - depreciationRate_);
        return produce;
    }

    [[nodiscard]] auto nextProducePlan() const noexcept -> GoodsQuantity {
        return calcProduceAmount();
    }

    [[nodiscard]] auto desiredCapital(const GoodsQuantity requiresSupply
    ) const noexcept -> GoodsQuantity {
        const auto desiredCapital = pow(requiresSupply, 1.0 / distributionRate_);
        return std::max(GoodsQuantity{0.0}, desiredCapital - capital_);
    }

    void addCapital(const GoodsQuantity add) noexcept { capital_ += add; }

  private:
    [[nodiscard]] auto calcProduceAmount() const noexcept -> GoodsQuantity {
        const auto capital = adjustedCapital();
        const auto out     = pow(capital, distributionRate_);
        assert(out.isZeroOrMore());
        return out;
    }

    [[nodiscard]] auto adjustedCapital() const noexcept -> GoodsQuantity {
        assert(capital_.isZeroOrMore());
        return std::max(GoodsQuantity{1.0}, capital_);
    }

    const double  depreciationRate_;
    const double  distributionRate_;
    GoodsQuantity capital_{0.0};
};

class WorkerManager final {
  public:
    explicit WorkerManager(const double distributionRate) noexcept
        : distributionRate_{distributionRate} {}

    [[nodiscard]] auto produce() noexcept -> GoodsQuantity {
        const auto workerInput = workspace_.takeout();
        const auto out         = pow(workerInput, distributionRate_);
        assert(out.isZeroOrMore());

        if (out.isPositive()) lastProduce_ = out;
        return out;
    }

    [[nodiscard]] auto nextProducePlan() const noexcept -> GoodsQuantity { return lastProduce_; }

    [[nodiscard]] auto desiredEmploy(const HeadCount employee, const GoodsQuantity requiresAmount)
        const noexcept -> HeadCount {
        assert(requiresAmount.isZeroOrMore());

        const auto requiresSumWorkerPower = pow(requiresAmount, 1.0 / distributionRate_);
        const auto avgProductPower        = calcAvgWorkerPower(employee);
        const auto out = (requiresSumWorkerPower / avgProductPower) - employee.value();
        return HeadCount{out};
    }

    [[nodiscard]] auto workspace() noexcept -> Workspace& { return workspace_; }

  private:
    [[nodiscard]] auto calcAvgWorkerPower(const HeadCount employee
    ) const noexcept -> GoodsQuantity {
        assert(lastProduce_.isPositive());
        if (employee.isZero()) return GoodsQuantity{1.0};
        const auto sumWorkerPower = pow(lastProduce_, 1.0 / distributionRate_);
        return sumWorkerPower / employee.value();
    }

    Workspace     workspace_;
    GoodsQuantity lastProduce_{1.0};
    const double  distributionRate_;
};

class Producer final {
  public:
    explicit Producer(RandomGenerator& masterRng) noexcept
        : Producer(masterRng, masterRng.random(setting::capitalDistributionRate)) {}

    [[nodiscard]] auto produce() noexcept -> GoodsQuantity {
        const auto out = capital_.produce() + worker_.produce();
        assert(out.isZeroOrMore());

        return productPower_ * out;
    }

    [[nodiscard]] auto desiredEmploy(const HeadCount employee, const GoodsQuantity requiresSupply)
        const noexcept -> HeadCount {
        assert(requiresSupply.isZeroOrMore());
        const auto capitalSupply  = capital_.nextProducePlan();
        const auto requiresWorker = requiresSupply / (productPower_ * capitalSupply);
        return worker_.desiredEmploy(employee, GoodsQuantity{requiresWorker});
    }

    [[nodiscard]] auto desiredCapital(const GoodsQuantity requiresSupply
    ) const noexcept -> GoodsQuantity {
        assert(requiresSupply.isZeroOrMore());
        const auto workerSupply    = worker_.nextProducePlan();
        const auto requiresCapital = requiresSupply / (productPower_ * workerSupply);
        return capital_.desiredCapital(GoodsQuantity{requiresCapital});
    }

    [[nodiscard]] auto workspace() noexcept -> Workspace& { return worker_.workspace(); }

    void addCapital(const GoodsQuantity add) noexcept { capital_.addCapital(add); }

  private:
    Producer(RandomGenerator& masterRng, const double capitalDistributionRate) noexcept
        : capital_{masterRng.random(setting::capitalDepreciationRate), capitalDistributionRate},
          worker_{1.0 - capitalDistributionRate},
          productPower_{masterRng.random(setting::productPower)} {}

    CapitalManager capital_;
    WorkerManager  worker_;
    const double   productPower_;
};

class ProducingSystem final {
  public:
    explicit ProducingSystem(RandomGenerator& masterRng) noexcept
        : producer_{masterRng}, inventory_{masterRng.random(setting::inventory)} {}

    [[nodiscard]] auto desiredEmploy(const GoodsQuantity requiresSupply, const HeadCount employee)
        const noexcept -> HeadCount {
        return producer_.desiredEmploy(employee, requiresSupply);
    }

    [[nodiscard]] auto desiredCapital(const GoodsQuantity requiresSupply
    ) const noexcept -> GoodsQuantity {
        return producer_.desiredCapital(requiresSupply);
    }

    void addCapital(const GoodsQuantity capital) noexcept { producer_.addCapital(capital); }

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

    void logging(BaseGoodsDropBox auto& dropBox) noexcept { dropBox.inventories.add(inventory_); }

  private:
    Producer      producer_;
    GoodsQuantity inventory_;
};
}  // namespace abm::base_goods::supplier