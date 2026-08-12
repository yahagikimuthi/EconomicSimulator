#include "core/engine.hpp"

#include <highfive/H5DataSet.hpp>
#include <highfive/H5File.hpp>
#include <string>
#include <string_view>

#include "config.hpp"
#include "orchestrator/consumer_goods.hpp"
#include "orchestrator/labor.hpp"
#include "orchestrator/production_goods.hpp"
#include "orchestrator/updates_loggings.hpp"
#include "world/message.hpp"

namespace abm {
void foo() {}
void Engine::run() {
    for (currentStep_ = Step{0}; currentStep_ < totalStep_; ++currentStep_) {
        if (currentStep_.value() == 600) {
            foo();
        }
        runLabor();
        runConsumerGoods();
        logging();
        check();
        reset();
    }
}

void Engine::runLabor() {
    for (BtoCFirm& firm : BtoCFirms_) {
        labor::adjustWorkforce(firm.index, firm.consumerGoods, firm.labor, laborRequestBox_);
    }

    for (HHold& hhold : hholds_) {
        labor::jobEntry(hhold.index, hhold.labor, laborRequestBox_);
    }

    for (BtoCFirm& firm : BtoCFirms_) {
        labor::offer(firm.labor);
    }

    for (HHold& hhold : hholds_) {
        labor::acceptOffer(hhold.labor);
    }

    for (BtoCFirm& firm : BtoCFirms_) {
        labor::registerMember(firm.consumerGoods, firm.labor);
    }

    for (HHold& hhold : hholds_) {
        labor::recordRosterEntry(hhold.labor);
    }

    for (BtoCFirm& firm : BtoCFirms_) {
        labor::acceptResignation(firm.labor);
    }

    for (BtoCFirm& firm : BtoCFirms_) {
        labor::endStep(firm.finance, firm.labor, dropBox_);
    }
    for (HHold& hhold : hholds_) {
        labor::endStep(hhold.finance, hhold.labor, dropBox_);
    }
}

void Engine::runProductionGoods() {
    for (HHold& hhold : hholds_) {
        production_goods::product(hhold.labor, Market::productionGoods);
    }

    for (BtoBFirm& firm : BtoBFirms_) {
        production_goods::postGoods(
            firm.index, firm.productionGoodsSupplier, firm.labor, productionGoodsEntryBox_
        );
    }

    for (BtoBFirm& firm : BtoBFirms_) {
        production_goods::purchase(
            firm.index,
            firm.finance,
            firm.productionGoodsDemander,
            firm.labor,
            productionGoodsEntryBox_
        );
    }

    for (BtoCFirm& firm : BtoCFirms_) {
        production_goods::purchase(
            firm.index, firm.finance, firm.productionGoods, firm.labor, productionGoodsEntryBox_
        );
    }

    for (BtoBFirm& firm : BtoBFirms_) {
        production_goods::trade(firm.productionGoodsSupplier);
    }

    for (BtoBFirm& firm : BtoBFirms_) {
        production_goods::afterTrade(firm.productionGoodsDemander);
    }

    for (BtoCFirm& firm : BtoCFirms_) {
        production_goods::afterTrade(firm.productionGoods);
    }

    for (BtoBFirm& firm : BtoBFirms_) {
        production_goods::endStep(firm.finance, firm.productionGoodsDemander);
    }
}

void Engine::runConsumerGoods() {
    for (HHold& hhold : hholds_) {
        consumer_goods::product(hhold.labor, Market::consumerGoods);
    }

    for (BtoCFirm& firm : BtoCFirms_) {
        consumer_goods::postGoods(firm.consumerGoods, firm.labor, consumerGoodsEntryBox_);
    }

    for (HHold& hhold : hholds_) {
        consumer_goods::purchase(
            hhold.finance, hhold.consumerGoods, hhold.labor, consumerGoodsEntryBox_, currentStep_
        );
    }

    for (BtoCFirm& firm : BtoCFirms_) {
        consumer_goods::trade(firm.consumerGoods);
    }

    for (HHold& hhold : hholds_) {
        consumer_goods::afterTrade(hhold.consumerGoods);
    }

    for (BtoCFirm& firm : BtoCFirms_) {
        consumer_goods::endStep(firm.finance, firm.consumerGoods, dropBox_);
    }

    for (HHold& hhold : hholds_) {
        consumer_goods::endStep(hhold.finance, hhold.consumerGoods);
    }
}

void Engine::logging() {
    for (BtoCFirm& firm : BtoCFirms_) {
        firm_finance::logging(dropBox_, firm.finance);
    }
    for (HHold& hhold : hholds_) {
        hhold_finance::logging(dropBox_, hhold.finance);
    }
    logger_.save(dropBox_, currentStep_);
}

void Engine::reset() {
    dropBox_.clear();
    laborRequestBox_.clear();
    productionGoodsEntryBox_.clear();
    consumerGoodsEntryBox_.clear();
}

void Engine::check() const {}

void Logger::save(const CensusDropBox& dropBox, const Step step) {
    namespace name = config::save_name;
    std::string     groupPath{"/step_" + std::to_string(step.value())};
    HighFive::Group group{file_.createGroup(groupPath)};

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