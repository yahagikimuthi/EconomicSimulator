#pragma once

#include <tbb/concurrent_vector.h>
#include <highfive/H5DataSet.hpp>
#include <highfive/H5File.hpp>

#include "components/common.hpp"
#include "components/consumer_goods_demander.hpp"
#include "components/consumer_goods_supplier/consumer_goods_supplier.hpp"
#include "components/labor_demander/labor_demander.hpp"
#include "components/labor_supplier/labor_supplier.hpp"
#include "components/production_goods_demander.hpp"
#include "components/production_goods_supplier/production_goods_supplier.hpp"
#include "components/util.hpp"
#include "helper.hpp"
#include "world/message.hpp"

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

class [[nodiscard]] Logger {
  public:
    explicit Logger();
    auto isValid() const -> bool { return file_.isValid(); }
    void save(const CensusDropBox& dropBox, const Step step);

  private:
    HighFive::File file_;
};

class [[nodiscard]] Engine {
  public:
    explicit Engine(const int totalStep);

    void run();

  private:
    void runLabor();
    void runProductionGoods();
    void runConsumerGoods();
    void update();
    void logging();
    void reset();
    void check() const;

    Logger                logger_;
    std::vector<BtoCFirm> BtoCFirms_;
    std::vector<BtoBFirm> BtoBFirms_;
    std::vector<HHold>    hholds_;

    LaborMarket                                  laborMarket_;
    tbb::concurrent_vector<ProductionGoodsEntry> productionGoodsEntryBox_;
    tbb::concurrent_vector<ConsumerGoodsEntry>   consumerGoodsEntryBox_;

    CensusDropBox dropBox_;

    const Step totalStep_;
    Step       currentStep_{0};

    const helper::PCG32Seed seed_;
    detail::RandomGenerator rng_;
};
}  // namespace abm