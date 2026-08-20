#include "core/engine.hpp"

#include <highfive/H5DataSet.hpp>
#include <highfive/H5File.hpp>
#include <string>
#include <string_view>
#include <vector>

#include "orchestrator/consumer_goods.hpp"
#include "orchestrator/labor.hpp"
#include "orchestrator/production_goods.hpp"
#include "orchestrator/updates_loggings.hpp"
#include "setting.hpp"
#include "world/common.hpp"

namespace abm {
void Engine::run() {
    for (currentStep_ = Step{0}; currentStep_ < totalStep_; ++currentStep_) {
        runLabor();
        runConsumerGoods();
        logging();
        check();
        reset();
    }
}

void Engine::runLabor() noexcept {
    for (BtoCFirm& firm : bToCFirms_) {
        labor::adjustWorkforce(firm.index, firm.consumerGoods, firm.labor, laborMarket_);
    }

    for (HHold& hhold : hholds_) {
        labor::jobEntry(hhold.index, hhold.labor, laborMarket_);
    }

    for (BtoCFirm& firm : bToCFirms_) {
        labor::offer(firm.labor);
    }

    for (HHold& hhold : hholds_) {
        labor::acceptOffer(hhold.labor);
    }

    for (BtoCFirm& firm : bToCFirms_) {
        labor::registerMember(firm.consumerGoods, firm.labor);
    }

    for (HHold& hhold : hholds_) {
        labor::recordRosterEntry(hhold.labor);
    }

    for (BtoCFirm& firm : bToCFirms_) {
        labor::acceptResignation(firm.labor);
    }

    for (BtoCFirm& firm : bToCFirms_) {
        labor::endStep(firm.finance, firm.labor);
    }
    for (HHold& hhold : hholds_) {
        labor::endStep(hhold.finance, hhold.labor, dropBox_);
    }
}

void Engine::runProductionGoods() noexcept {
    for (HHold& hhold : hholds_) {
        production_goods::product(hhold.labor, Market::productionGoods);
    }

    for (BtoBFirm& firm : bToBFirms_) {
        production_goods::postGoods(
            firm.index, firm.productionGoodsSupplier, firm.labor, productionGoodsMarket_
        );
    }

    for (BtoBFirm& firm : bToBFirms_) {
        production_goods::purchase(
            firm.index,
            firm.finance,
            firm.productionGoodsDemander,
            firm.labor,
            productionGoodsMarket_
        );
    }

    for (BtoCFirm& firm : bToCFirms_) {
        production_goods::purchase(
            firm.index, firm.finance, firm.productionGoods, firm.labor, productionGoodsMarket_
        );
    }

    for (BtoBFirm& firm : bToBFirms_) {
        production_goods::trade(firm.productionGoodsSupplier);
    }

    for (BtoBFirm& firm : bToBFirms_) {
        production_goods::afterTrade(firm.productionGoodsDemander);
    }

    for (BtoCFirm& firm : bToCFirms_) {
        production_goods::afterTrade(firm.productionGoods);
    }

    for (BtoBFirm& firm : bToBFirms_) {
        production_goods::endStep(firm.productionGoodsDemander);
    }
}

void Engine::runConsumerGoods() noexcept {
    for (HHold& hhold : hholds_) {
        consumer_goods::product(hhold.labor, Market::consumerGoods);
    }

    for (BtoCFirm& firm : bToCFirms_) {
        consumer_goods::postGoods(firm.consumerGoods, firm.labor, consumerGoodsMarket_);
    }

    for (HHold& hhold : hholds_) {
        consumer_goods::purchase(
            hhold.finance, hhold.consumerGoods, hhold.labor, consumerGoodsMarket_, currentStep_
        );
    }

    for (BtoCFirm& firm : bToCFirms_) {
        consumer_goods::trade(firm.consumerGoods);
    }

    for (HHold& hhold : hholds_) {
        consumer_goods::afterTrade(hhold.consumerGoods);
    }

    for (BtoCFirm& firm : bToCFirms_) {
        consumer_goods::endStep(firm.consumerGoods);
    }

    for (HHold& hhold : hholds_) {
        consumer_goods::endStep(hhold.consumerGoods);
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

void Engine::check() const noexcept {}

void Logger::save(const CensusDropBox& dropBox, const Step step) {
    namespace name = setting::save_name;
    auto groupPath = std::string{"/step_" + std::to_string(step.value())};
    auto group     = HighFive::Group{file_.createGroup(groupPath)};

    auto create{[&group](std::string_view dataName, const std::vector<double>& data) -> void {
        group.createDataSet(static_cast<std::string>(dataName), data);
    }};

    create(name::firmAssets, dropBox.firmAssets);
    create(name::postedEmployments, dropBox.postedEmployments);
    create(name::employments, dropBox.employments);
    create(name::prices, dropBox.prices);
    create(name::supplies, dropBox.supplies);
    create(name::markups, dropBox.markups);
    create(name::inventories, dropBox.inventories);
    create(name::householdAssets, dropBox.hholdAssets);
    create(name::wages, dropBox.wages);
}
}  // namespace abm