#include "core/engine.hpp"

#include <algorithm>
#include <highfive/H5DataSet.hpp>
#include <highfive/H5File.hpp>
#include <ranges>
#include <string>
#include <string_view>
#include <vector>

#include "orchestrator/consumer_goods.hpp"
#include "orchestrator/labor.hpp"
#include "orchestrator/updates_loggings.hpp"
#include "setting.hpp"
#include "world/common.hpp"

namespace abm {
namespace {
void foo() {}
}  // namespace

auto Engine::calcSumAsset() const noexcept -> double {
    const auto firmAsset = std::ranges::fold_left(
        bToCFirms_ | std::views::transform([](const BtoCFirm& firm) -> double {
            return firm.finance.asset().value();
        }),
        0.0,
        std::plus{}
    );
    const auto hholdAsset = std::ranges::fold_left(
        hholds_ | std::views::transform([](const HHold& hhold) -> double {
            return hhold.finance.asset().value();
        }),
        0.0,
        std::plus{}
    );
    return firmAsset + hholdAsset;
}

void Engine::run() {
    for (currentStep_ = Step{0}; currentStep_ < totalStep_; ++currentStep_) {
        if (currentStep_ == Step{600}) {
            foo();
        }
        runLabor();
        runConsumerGoods();
        logging();
        check();
        reset();
    }
}

void Engine::runLabor() noexcept {
    for (BtoCFirm& firm : bToCFirms_) {
        labor::adjustWorkforce(
            firm.index, firm.consumerGoodsSupplier, firm.laborDemander, laborMarket_
        );
    }

    for (HHold& hhold : hholds_) {
        labor::jobEntry(hhold.index, hhold.laborSupplier, laborMarket_);
    }

    for (BtoCFirm& firm : bToCFirms_) {
        labor::offer(firm.laborDemander);
    }

    for (HHold& hhold : hholds_) {
        labor::acceptOffer(hhold.laborSupplier);
    }

    for (BtoCFirm& firm : bToCFirms_) {
        labor::registerMember(firm.consumerGoodsSupplier, firm.laborDemander);
    }

    for (HHold& hhold : hholds_) {
        labor::recordRosterEntry(hhold.laborSupplier);
    }

    for (BtoCFirm& firm : bToCFirms_) {
        labor::acceptResignation(firm.laborDemander);
    }

    for (BtoCFirm& firm : bToCFirms_) {
        labor::endStep(firm.finance, firm.laborDemander, dropBox_);
    }
    for (HHold& hhold : hholds_) {
        labor::endStep(hhold.finance, hhold.laborSupplier, dropBox_);
    }
}
/*
void Engine::runProductionGoods() noexcept {
    for (HHold& hhold : hholds_) {
        production_goods::product(hhold.laborSupplier, Market::productionGoods);
    }

    for (BtoBFirm& firm : bToBFirms_) {
        production_goods::postGoods(
            firm.index, firm.productionGoodsSupplier, firm.laborDemander, productionGoodsMarket_
        );
    }

    for (BtoBFirm& firm : bToBFirms_) {
        production_goods::purchase(
            firm.index,
            firm.finance,
            firm.productionGoodsDemander,
            firm.laborDemander,
            productionGoodsMarket_
        );
    }

    for (BtoCFirm& firm : bToCFirms_) {
        production_goods::purchase(
            firm.index,
            firm.finance,
            firm.productionGoodsDemander,
            firm.laborDemander,
            productionGoodsMarket_
        );
    }

    for (BtoBFirm& firm : bToBFirms_) {
        production_goods::trade(firm.productionGoodsSupplier);
    }

    for (BtoBFirm& firm : bToBFirms_) {
        production_goods::afterTrade(firm.productionGoodsDemander);
    }

    for (BtoCFirm& firm : bToCFirms_) {
        production_goods::afterTrade(firm.productionGoodsDemander);
    }

    for (BtoBFirm& firm : bToBFirms_) {
        production_goods::endStep(
            firm.finance, firm.productionGoodsSupplier, firm.productionGoodsDemander
        );
    }
}
*/

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
            hhold.finance,
            hhold.consumerGoodsDemander,
            hhold.laborSupplier,
            consumerGoodsMarket_,
            currentStep_
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
//399ステップ目で失敗
void Engine::check() const noexcept {
    ASSERT(calcSumAsset() > 0.0);
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