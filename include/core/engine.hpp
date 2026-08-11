#pragma once

#include <tbb/concurrent_vector.h>
#include <cstdint>
#include <highfive/H5DataSet.hpp>
#include <highfive/H5File.hpp>

#include "components/common.hpp"
#include "components/consumer_goods_demander.hpp"
#include "components/consumer_goods_supplier.hpp"
#include "components/labor_demander/labor_demander.hpp"
#include "components/labor_supplier/labor_supplier.hpp"
#include "components/production_goods_demander.hpp"
#include "components/production_goods_supplier.hpp"
#include "helper.hpp"
#include "world/message.hpp"

struct BtoCFirm {  // NOLINT
    agent_index::Component  index;
    firm_finance::Component finance;
    LaborDemander           labor;
    ConsumerGoodsSupplier   consumerGoods;
    ProductionGoodsDemander productionGoods;
};

struct BtoBFirm {  // NOLINT
    agent_index::Component  index;
    firm_finance::Component finance;
    LaborDemander           labor;
    ProductionGoodsSupplier productionGoodsSupplier;
    ProductionGoodsDemander productionGoodsDemander;
};

struct HHold {  // NOLINT
    agent_index::Component   index;
    hhold_finance::Component finance;
    LaborSupplier            labor;
    ConsumerGoodsDemander    consumerGoods;
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

    auto makeSeed() -> std::uint64_t { return helper::makeSeed(masterRng_); }

    Logger                logger_;
    std::vector<BtoCFirm> BtoCFirms_;
    std::vector<BtoBFirm> BtoBFirms_;
    std::vector<HHold>    hholds_;

    tbb::concurrent_vector<LaborRequest>         laborRequestBox_;
    tbb::concurrent_vector<ProductionGoodsEntry> productionGoodsEntryBox_;
    tbb::concurrent_vector<ConsumerGoodsEntry>   consumerGoodsEntryBox_;

    CensusDropBox dropBox_;

    const Step totalStep_;
    Step       currentStep_{0};

    const helper::PCG32Seed seed_;
    pcg32                   masterRng_;
};