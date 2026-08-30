#pragma once

#include <cstdint>
#include <random>
#include <vector>

#include "components/capital_demander.hpp"
#include "components/capital_supplier/capital_supplier.hpp"
#include "components/consumer_goods_demander.hpp"
#include "components/consumer_goods_supplier/consumer_goods_supplier.hpp"
#include "components/finance/finance.hpp"
#include "components/government.hpp"
#include "components/labor_demander/labor_demander.hpp"
#include "components/labor_supplier/labor_supplier.hpp"
#include "others/setting.hpp"
#include "others/util.hpp"
#include "values/common.hpp"
#include "values/date.hpp"
#include "world/capital.hpp"
#include "world/common.hpp"
#include "world/consumer_goods.hpp"
#include "world/labor.hpp"

namespace abm {
struct BtoCFirm final {  // NOLINT
    const AgentID         id;
    FirmFinance           finance;
    LaborDemander         laborDemander;
    ConsumerGoodsSupplier consumerGoodsSupplier;
    CapitalDemander       capitalDemander;
};

struct BtoBFirm final {  // NOLINT
    const AgentID   id;
    FirmFinance     finance;
    LaborDemander   laborDemander;
    CapitalDemander capitalDemander;
    CapitalSupplier capitalSupplier;
};

struct HHold final {  // NOLINT
    const AgentID         id;
    HHoldFinance          finance;
    LaborSupplier         laborSupplier;
    ConsumerGoodsDemander consumerGoodsDemander;
};

struct PCG32Seed final {
    const std::uint64_t state;
    const std::uint64_t stream;
};

class Engine final {
  public:
    [[nodiscard]] explicit Engine(const unsigned int totalStep, const bool isAnalysis) noexcept;

    void run() noexcept;

  private:
    void runLabor() noexcept;
    void runCapital() noexcept;
    void runConsumerGoods() noexcept;
    void endAllStep() noexcept;
    void reset() noexcept;

    [[nodiscard]] static constexpr auto generateSeed() noexcept -> PCG32Seed {
        if constexpr (not setting::useRuntimeRandomSeed) {
            return {.state = setting::fixedSeedState, .stream = setting::fixedSeedStream};
        }
        auto       rd     = std::random_device{};
        const auto state  = std::uint64_t{(static_cast<std::uint64_t>(rd()) << 32) | rd()};
        const auto stream = std::uint64_t{(static_cast<std::uint64_t>(rd()) << 32) | rd()};
        return {.state = state, .stream = stream};
    }

    Logger                logger_;
    std::vector<BtoCFirm> bToCFirms_;
    std::vector<BtoBFirm> bToBFirms_;
    std::vector<HHold>    hholds_;
    Government            government_;

    CensusDropBox dropBox_;

    const Date endDate_;
    Date       today_{1U, 1U, 1U};

    const PCG32Seed seed_;
    RandomGenerator rng_;

    LaborMarket         laborMarket_;
    CapitalMarket       capitalMarket_;
    ConsumerGoodsMarket consumerGoodsMarket_;

    bool isAnalysis_;
};
}  // namespace abm