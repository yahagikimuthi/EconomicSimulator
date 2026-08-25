#pragma once

#include <tbb/concurrent_vector.h>
#include <cstdint>
#include <filesystem>
#include <highfive/H5DataSet.hpp>
#include <highfive/H5File.hpp>
#include <random>
#include <vector>

#include "components/common.hpp"
#include "components/consumer_goods_demander.hpp"
#include "components/consumer_goods_supplier/consumer_goods_supplier.hpp"
#include "components/government.hpp"
#include "components/labor_demander/labor_demander.hpp"
#include "components/labor_supplier/labor_supplier.hpp"
#include "components/production_goods_demander.hpp"
#include "components/production_goods_supplier/production_goods_supplier.hpp"
#include "core/setting.hpp"
#include "core/util.hpp"
#include "core/values/common.hpp"
#include "world/common.hpp"
#include "world/goods.hpp"
#include "world/labor.hpp"

namespace abm {
struct BtoCFirm final {  // NOLINT
    const AgentID           id;
    FirmFinance             finance;
    LaborDemander           laborDemander;
    ConsumerGoodsSupplier   consumerGoodsSupplier;
    ProductionGoodsDemander productionGoodsDemander;
};

struct BtoBFirm final {  // NOLINT
    const AgentID           id;
    FirmFinance             finance;
    LaborDemander           laborDemander;
    ProductionGoodsDemander productionGoodsDemander;
    ProductionGoodsSupplier productionGoodsSupplier;
};

struct HHold final {  // NOLINT
    const AgentID         id;
    HHoldFinance          finance;
    LaborSupplier         laborSupplier;
    ConsumerGoodsDemander consumerGoodsDemander;
};

class Logger final {
  public:
    [[nodiscard]] explicit Logger() noexcept
        : file_{[]() -> HighFive::File {
              namespace fs        = std::filesystem;
              const auto filepath = static_cast<std::string>(setting::simulationResultOutputPath);
              const auto path     = fs::path{filepath};
              if (path.has_parent_path()) fs::create_directories(path.parent_path());
              return HighFive::File{
                  filepath,
                  HighFive::File::ReadWrite | HighFive::File::Create | HighFive::File::Truncate
              };
          }()} {}

    [[nodiscard]] auto isValid() const noexcept -> bool { return file_.isValid(); }

    void save(const CensusDropBox& dropBox, Step step) noexcept;

  private:
    HighFive::File file_;
};

struct PCG32Seed final {
    const std::uint64_t state;
    const std::uint64_t stream;
};

class Engine final {
    template <typename T>
    using TBBVec = tbb::concurrent_vector<T>;

  public:
    [[nodiscard]] explicit Engine(const int totalStep, const bool isAnalysis) noexcept;

    void run() noexcept;

  private:
    void runLabor() noexcept;
    void runProductionGoods() noexcept;
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

    const Step totalStep_;
    Step       currentStep_{0};

    const PCG32Seed seed_;
    RandomGenerator rng_;

    LaborMarket           laborMarket_;
    ProductionGoodsMarket productionGoodsMarket_;
    ConsumerGoodsMarket   consumerGoodsMarket_;

    bool isAnalysis_;
};
}  // namespace abm