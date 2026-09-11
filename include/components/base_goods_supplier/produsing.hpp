#pragma once

#include <algorithm>
#include <cassert>
#include <cmath>

#include "components/base_goods_supplier/common.hpp"
#include "others/setting.hpp"
#include "others/util.hpp"
#include "values/goods.hpp"
#include "values/labor.hpp"
#include "world/base_goods.hpp"
#include "world/drop_box.hpp"

namespace abm::base_goods::supplier {
class Producer final {
  public:
    explicit Producer(RandomGenerator& masterRng) noexcept
        : baseProductPower_{masterRng.random(setting::productPower)},
          capitalDepreciationRate_{masterRng.random(setting::capitalDepreciationRate)},
          capitalDistributionRate_{masterRng.random(setting::capitalDistributionRate)} {}

    [[nodiscard]] auto produce() noexcept -> GoodsQuantity {
        const auto workerInput = workspace_.takeout();
        assert(workerInput.isZeroOrMore());
        if (not workerInput.isZero()) lastWorkerInput_ = workerInput;

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

    [[nodiscard]] auto calcDesiredCapital(const GoodsQuantity requiresProduct
    ) const noexcept -> GoodsQuantity {
        const auto bottom =
            requiresProduct / (baseProductPower_ *
                               std::pow(lastWorkerInput_.value(), 1.0 - capitalDistributionRate_));
        const auto demand = std::pow(bottom.value(), 1.0 / capitalDistributionRate_);
        const auto out    = GoodsQuantity{demand} - capital_;
        return std::max(out, GoodsQuantity{0.0});
    }

    [[nodiscard]] auto calcDesiredEmploy(const HeadCount employee, const GoodsQuantity targetSupply)
        const noexcept -> HeadCount {
        assert(targetSupply.isZeroOrMore());
        const auto avgEmployeePower = avgWorkerPower(employee);
        const auto capital          = std::max(capital_.value(), global_setting::epsilon);
        const auto bottom =
            targetSupply / (baseProductPower_ * std::pow(capital, capitalDistributionRate_));
        const auto desiredLaborPower = std::pow(bottom.value(), 1.0 - capitalDistributionRate_);

        const auto out = desiredLaborPower / avgEmployeePower;

        assert(not std::isnan(out));
        return HeadCount{out} - employee;
    }

    [[nodiscard]] auto workspace() noexcept -> Workspace& { return workspace_; }

  private:
    [[nodiscard]] auto avgWorkerPower(const HeadCount employee) const noexcept -> double {
        assert(not lastWorkerInput_.isZero());
        if (employee.isZero()) return 1.0;
        return employee.value() / lastWorkerInput_.value();
    }

    Workspace     workspace_;
    GoodsQuantity lastWorkerInput_{1.0};
    const double  baseProductPower_;
    const double  capitalDepreciationRate_;
    const double  capitalDistributionRate_;
    GoodsQuantity capital_{0.0};
};

class ProducingSystem final {
  public:
    explicit ProducingSystem(RandomGenerator& masterRng) noexcept
        : producer_{masterRng}, inventory_{masterRng.random(setting::inventory)} {}

    [[nodiscard]] auto calcDesiredEmploy(
        const GoodsQuantity requiresSupply, const HeadCount employee
    ) const noexcept -> HeadCount {
        return producer_.calcDesiredEmploy(employee, requiresSupply);
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

    void logging(BaseGoodsDropBox auto& dropBox) noexcept { dropBox.inventories.add(inventory_); }

  private:
    Producer      producer_;
    GoodsQuantity inventory_;
};
}  // namespace abm::base_goods::supplier