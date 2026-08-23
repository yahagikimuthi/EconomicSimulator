#include "abm.hpp"

#include <highfive/H5DataSet.hpp>
#include <highfive/H5File.hpp>
#include <print>
#include <string>
#include <string_view>
#include <vector>

#include "analysis/analysis.hpp"
#include "core/setting.hpp"
#include "orchestrator/consumer_goods.hpp"
#include "orchestrator/labor.hpp"
#include "orchestrator/production_goods.hpp"
#include "orchestrator/updates_loggings.hpp"
#include "world/common.hpp"

namespace abm {

template <typename T>
    requires requires(T t) { t.finance.asset().value(); }
[[nodiscard]] constexpr auto calcSumAsset(std::vector<T>& agents) noexcept -> double {
    return std::ranges::fold_left(agents, 0.0, [](const double acc, const T& agent) -> double {
        return acc + agent.finance.asset().value();
    });
}

void Engine::run() {
    for (currentStep_ = Step{0}; currentStep_ < totalStep_; ++currentStep_) {
        runLabor();
        runProductionGoods();
        runConsumerGoods();
        logging();
        reset();
    }
    if (isAnalysis_) {
        analysis::analysisData();
    }
}

void Engine::runLabor() noexcept {
    const auto bToBs  = calcSumAsset(bToBFirms_);
    const auto bToCs  = calcSumAsset(bToCFirms_);
    const auto hholds = calcSumAsset(hholds_);

    std::println("ステップ: {}", currentStep_.value());
    std::println("{}", bToBs + bToCs + hholds);
    std::println();

    for (BtoCFirm& firm : bToCFirms_) {
        labor::adjustWorkforce(
            firm.index, firm.consumerGoodsSupplier, firm.laborDemander, laborMarket_
        );
    }
    for (BtoBFirm& firm : bToBFirms_) {
        labor::adjustWorkforce(
            firm.index, firm.productionGoodsSupplier, firm.laborDemander, laborMarket_
        );
    }

    for (HHold& hhold : hholds_) {
        labor::jobEntry(hhold.index, hhold.laborSupplier, laborMarket_);
    }

    for (BtoCFirm& firm : bToCFirms_) {
        labor::offer(firm.laborDemander);
    }
    for (BtoBFirm& firm : bToBFirms_) {
        labor::offer(firm.laborDemander);
    }

    for (HHold& hhold : hholds_) {
        labor::acceptOffer(hhold.laborSupplier);
    }

    for (BtoCFirm& firm : bToCFirms_) {
        labor::registerMember(firm.consumerGoodsSupplier, firm.laborDemander);
    }
    for (BtoBFirm& firm : bToBFirms_) {
        labor::registerMember(firm.productionGoodsSupplier, firm.laborDemander);
    }

    for (HHold& hhold : hholds_) {
        labor::recordRosterEntry(hhold.laborSupplier);
    }

    for (BtoCFirm& firm : bToCFirms_) {
        labor::acceptResignation(firm.laborDemander);
    }
    for (BtoBFirm& firm : bToBFirms_) {
        labor::acceptResignation(firm.laborDemander);
    }

    for (BtoCFirm& firm : bToCFirms_) {
        labor::endStep(firm.finance, firm.laborDemander, dropBox_);
    }
    for (BtoBFirm& firm : bToBFirms_) {
        labor::endStep(firm.finance, firm.laborDemander, dropBox_);
    }

    for (HHold& hhold : hholds_) {
        labor::endStep(hhold.finance, hhold.laborSupplier, dropBox_);
    }
}

void Engine::runProductionGoods() noexcept {
    for (HHold& hhold : hholds_) {
        production_goods::product(hhold.laborSupplier, Market::productionGoods);
    }

    for (BtoBFirm& firm : bToBFirms_) {
        production_goods::postGoods(
            firm.index, firm.productionGoodsSupplier, firm.laborDemander, productionGoodsMarket_
        );
    }

    for (BtoCFirm& firm : bToCFirms_) {
        production_goods::purchase(
            firm.index,
            firm.finance,
            firm.productionGoodsDemander,
            firm.consumerGoodsSupplier,
            productionGoodsMarket_
        );
    }
    for (BtoBFirm& firm : bToBFirms_) {
        production_goods::purchase(
            firm.index,
            firm.finance,
            firm.productionGoodsDemander,
            firm.productionGoodsSupplier,
            productionGoodsMarket_
        );
    }

    for (BtoBFirm& firm : bToBFirms_) {
        production_goods::trade(firm.productionGoodsSupplier);
    }

    for (BtoCFirm& firm : bToCFirms_) {
        production_goods::afterTrade(firm.productionGoodsDemander);
    }
    for (BtoBFirm& firm : bToBFirms_) {
        production_goods::afterTrade(firm.productionGoodsDemander);
    }

    for (BtoBFirm& firm : bToBFirms_) {
        production_goods::endStep(
            firm.finance, firm.productionGoodsSupplier, firm.productionGoodsDemander
        );
    }
    for (BtoCFirm& firm : bToCFirms_) {
        production_goods::endStep(
            firm.finance, firm.consumerGoodsSupplier, firm.productionGoodsDemander
        );
    }
}

void Engine::runConsumerGoods() noexcept {
    for (HHold& hhold : hholds_) {
        consumer_goods::product(hhold.laborSupplier, Market::consumerGoods);
    }

    for (BtoCFirm& firm : bToCFirms_) {
        consumer_goods::postGoods(
            firm.consumerGoodsSupplier, firm.laborDemander, consumerGoodsMarket_
        );
    }

    for (HHold& hhold : hholds_) {
        consumer_goods::purchase(
            hhold.finance, hhold.consumerGoodsDemander, consumerGoodsMarket_, currentStep_
        );
    }

    for (BtoCFirm& firm : bToCFirms_) {
        consumer_goods::trade(firm.consumerGoodsSupplier);
    }

    for (HHold& hhold : hholds_) {
        consumer_goods::afterTrade(hhold.consumerGoodsDemander);
    }

    for (BtoCFirm& firm : bToCFirms_) {
        consumer_goods::endStep(firm.finance, firm.consumerGoodsSupplier, dropBox_);
    }

    for (HHold& hhold : hholds_) {
        consumer_goods::endStep(hhold.finance, hhold.consumerGoodsDemander);
    }
}

void Engine::logging() {
    for (BtoCFirm& firm : bToCFirms_) {
        firm_finance::logging(dropBox_, firm.finance);
    }

    for (BtoBFirm& firm : bToBFirms_) {
        firm_finance::logging(dropBox_, firm.finance);
    }

    for (HHold& hhold : hholds_) {
        hhold_finance::logging(dropBox_, hhold.finance);
    }
    logger_.save(dropBox_, currentStep_);
}

void Engine::reset() noexcept {
    dropBox_.clear();
    laborMarket_.clear();
    productionGoodsMarket_.clear();
    consumerGoodsMarket_.clear();
}

void Logger::save(const CensusDropBox& dropBox, const Step step) {
    namespace name = setting::save_name;
    auto groupPath = std::string{"/step_" + std::to_string(step.value())};
    auto group     = HighFive::Group{file_.createGroup(groupPath)};

    auto create{[&group](std::string_view dataName, const std::vector<double>& data) -> void {
        group.createDataSet(static_cast<std::string>(dataName), data);
    }};

    create(name::firmAssets, dropBox.firmAssets);
    create(name::postedEmployments, dropBox.postedEmployments);
    create(name::postedWages, dropBox.postedWages);
    create(name::employments, dropBox.employments);
    create(name::sumWages, dropBox.sumWages);
    create(name::prices, dropBox.prices);
    create(name::supplies, dropBox.supplies);
    create(name::markups, dropBox.markups);
    create(name::inventories, dropBox.inventories);
    create(name::householdAssets, dropBox.hholdAssets);
    create(name::wages, dropBox.wages);
}
}  // namespace abm