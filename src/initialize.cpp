#include "engine.hpp"

#include <cstdlib>
#include <highfive/H5DataSet.hpp>
#include <highfive/H5File.hpp>
#include <iostream>
#include <pcg_random.hpp>
#include <utility>

#include "components/common.hpp"
#include "components/consumer_goods_demander.hpp"
#include "components/consumer_goods_supplier/consumer_goods_supplier.hpp"
#include "components/labor_demander/labor_demander.hpp"
#include "components/labor_supplier/labor_supplier.hpp"
#include "components/production_goods_demander.hpp"
#include "components/production_goods_supplier/production_goods_supplier.hpp"
#include "core/setting.hpp"
#include "core/util.hpp"
#include "core/values/common.hpp"
#include "world/common.hpp"

namespace abm {
namespace {
[[nodiscard]] auto createFirmFinance(RandomGenerator& masterRng) noexcept -> FirmFinance {
    return FirmFinance{masterRng};
}

[[nodiscard]] auto createHHoldFinance(RandomGenerator& masterRng) noexcept -> HHoldFinance {
    return HHoldFinance{masterRng};
}

[[nodiscard]] auto createLaborDemander(
    RandomGenerator& masterRng, const AgentID id, const EMarket firmType
) noexcept -> LaborDemander {
    labor::CompanyBoard board{id, firmType};
    return LaborDemander{masterRng, std::move(board)};
}

[[nodiscard]] auto createLaborSupplier(RandomGenerator& masterRng) noexcept -> LaborSupplier {
    return LaborSupplier{masterRng};
}

[[nodiscard]] auto createConsumerGoodsDemander(RandomGenerator& masterRng
) noexcept -> ConsumerGoodsDemander {
    return ConsumerGoodsDemander{masterRng};
}

[[nodiscard]] auto createConsumerGoodsSupplier(RandomGenerator& masterRng
) noexcept -> ConsumerGoodsSupplier {
    return ConsumerGoodsSupplier{masterRng};
}

[[nodiscard]] auto createProductionGoodsDemander(RandomGenerator& masterRng
) noexcept -> ProductionGoodsDemander {
    return ProductionGoodsDemander{masterRng};
}

[[nodiscard]] auto createProductionGoodsSupplier(RandomGenerator& masterRng
) noexcept -> ProductionGoodsSupplier {
    return ProductionGoodsSupplier{masterRng};
}
}  // namespace

Engine::Engine(const int totalStep, const bool isAnalysis) noexcept
    : totalStep_{totalStep},
      seed_{generateSeed()},
      rng_{pcg32{seed_.state, seed_.stream}},
      isAnalysis_{isAnalysis} {
    if (not logger_.isValid()) {
        std::cerr << "can not create file\n";
        std::abort();
    }

    namespace cnt = setting::agent_count;

    bToCFirms_.reserve(cnt::bToCFirm);
    int agentId{};
    for (; agentId < cnt::bToCFirm; ++agentId) {
        bToCFirms_.emplace_back(BtoCFirm{
            .id            = AgentID{agentId},
            .finance       = createFirmFinance(rng_),
            .laborDemander = createLaborDemander(rng_, AgentID{agentId}, EMarket::ConsumerGoods),
            .consumerGoodsSupplier   = createConsumerGoodsSupplier(rng_),
            .productionGoodsDemander = createProductionGoodsDemander(rng_)
        });
    }
    for (BtoCFirm& firm : bToCFirms_) {
        firm.laborDemander.setMediator();
        firm.consumerGoodsSupplier.setMediator();
    }

    bToBFirms_.reserve(cnt::bToBFirm);
    for (; agentId < cnt::bToCFirm + cnt::bToBFirm; ++agentId) {
        bToBFirms_.emplace_back(BtoBFirm{
            .id            = AgentID{agentId},
            .finance       = createFirmFinance(rng_),
            .laborDemander = createLaborDemander(rng_, AgentID{agentId}, EMarket::ProductionGoods),
            .productionGoodsDemander = createProductionGoodsDemander(rng_),
            .productionGoodsSupplier = createProductionGoodsSupplier(rng_)
        });
    }
    for (BtoBFirm& firm : bToBFirms_) {
        firm.laborDemander.setMediator();
        firm.productionGoodsSupplier.setMediator();
    }

    hholds_.reserve(cnt::hhold);
    for (; agentId < cnt::bToCFirm + cnt::bToBFirm + cnt::hhold; ++agentId) {
        hholds_.emplace_back(HHold{
            .id                    = AgentID{agentId},
            .finance               = createHHoldFinance(rng_),
            .laborSupplier         = createLaborSupplier(rng_),
            .consumerGoodsDemander = createConsumerGoodsDemander(rng_)
        });
    }
}
}  // namespace abm