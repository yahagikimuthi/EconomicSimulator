#pragma once

#include <tbb/concurrent_vector.h>
#include <cstdint>
#include <highfive/H5DataSet.hpp>
#include <highfive/H5File.hpp>
#include <random>

#include "components/common.hpp"
#include "components/consumer_goods_demander.hpp"
#include "components/consumer_goods_supplier/consumer_goods_supplier.hpp"
#include "components/labor_demander/labor_demander.hpp"
#include "components/labor_supplier/labor_supplier.hpp"
#include "components/production_goods_demander.hpp"
#include "components/production_goods_supplier/production_goods_supplier.hpp"
#include "config.hpp"
#include "util.hpp"
#include "world/common.hpp"
#include "world/goods.hpp"
#include "world/labor.hpp"

namespace abm {

struct BtoCFirm {  // NOLINT
    AgentIndex              index;
    FirmFinance             finance;
    LaborDemander           labor;
    ConsumerGoodsSupplier   consumerGoods;
    ProductionGoodsDemander productionGoods;
};

struct BtoBFirm {  // NOLINT
    AgentIndex              index;
    FirmFinance             finance;
    LaborDemander           labor;
    ProductionGoodsSupplier productionGoodsSupplier;
    ProductionGoodsDemander productionGoodsDemander;
};

struct HHold {  // NOLINT
    AgentIndex            index;
    HHoldFinance          finance;
    LaborSupplier         labor;
    ConsumerGoodsDemander consumerGoods;
};

class Logger {
  public:
    [[nodiscard]] explicit Logger();
    auto isValid() const noexcept -> bool { return file_.isValid(); }
    void save(const CensusDropBox& dropBox, const Step step);

  private:
    HighFive::File file_;
};

struct PCG32Seed {
    const std::uint64_t state;
    const std::uint64_t stream;
};

class Engine {
    template <typename T>
    using TBBVec = tbb::concurrent_vector<T>;

  public:
    [[nodiscard]] explicit Engine(const int totalStep) noexcept;

    void run() noexcept;

  private:
    void runLabor() noexcept;
    void runProductionGoods() noexcept;
    void runConsumerGoods() noexcept;
    void update() noexcept;
    void logging() noexcept;
    void reset() noexcept;
    void check() const noexcept;

    Logger                logger_;
    std::vector<BtoCFirm> BtoCFirms_;
    std::vector<BtoBFirm> BtoBFirms_;
    std::vector<HHold>    hholds_;

    LaborMarket                  laborMarket_;
    TBBVec<ProductionGoodsEntry> productionGoodsEntryBox_;
    TBBVec<ConsumerGoodsEntry>   consumerGoodsEntryBox_;

    CensusDropBox dropBox_;

    const Step totalStep_;
    Step       currentStep_{0};

    const PCG32Seed seed_;
    RandomGenerator rng_;

    [[nodiscard]] static auto generateSeed() noexcept -> PCG32Seed {
        if (not config::setting::useRuntimeRandomSeed)
            return {
                .state = config::setting::fixedSeedState, .stream = config::setting::fixedSeedStream
            };

        auto       rd     = std::random_device{};
        const auto state  = std::uint64_t{(static_cast<std::uint64_t>(rd()) << 32) | rd()};
        const auto stream = std::uint64_t{(static_cast<std::uint64_t>(rd()) << 32) | rd()};
        return {.state = state, .stream = stream};
    }
};
}  // namespace abm