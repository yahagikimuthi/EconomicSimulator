#include "core/engine.hpp"

#include <strategies/common.hpp>
#include <strategies/goods_demander.hpp>
#include <strategies/goods_supplier.hpp>
#include <strategies/labor_demander.hpp>
#include <strategies/labor_supplier.hpp>
#include <string>

#include "config.hpp"
#include "strategies/orchestrator.hpp"
#include "world/message.hpp"

namespace core {
void Engine::run() {
    runLabor();
    runGoods();
    update();
    logging();
    reset();
}

void Engine::runLabor() {
    for (Firm& firm : firms_) {
        orchestrator::postLaborRequest(firm.index, firm.goods, firm.labor, laborRequestBox_);
    }

    for (HHold& hhold : hholds_) {
        orchestrator::jobEntry(hhold.index, hhold.labor, laborRequestBox_);
    }

    for (Firm& firm : firms_) {
        orchestrator::offer(firm.labor);
    }

    for (HHold& hhold : hholds_) {
        orchestrator::acceptOffer(hhold.labor);
    }

    for (Firm& firm : firms_) {
        orchestrator::registerMember(firm.goods, firm.labor);
    }
}

void Engine::runGoods() {
    for (Firm& firm : firms_) {
        orchestrator::postGoods(firm.goods, firm.labor, goodsEntryBox_);
    }

    for (HHold& hhold : hholds_) {
        orchestrator::purchase(hhold.finance, hhold.goods, hhold.labor, goodsEntryBox_);
    }

    for (Firm& firm : firms_) {
        orchestrator::trade(firm.goods);
    }

    for (HHold& hhold : hholds_) {
        orchestrator::afterTrade(hhold.goods);
    }
}

void Engine::update() {
    for (Firm& firm : firms_) {
        orchestrator::updateAsset(firm.finance, firm.labor, firm.goods);
    }
    for (HHold& hhold : hholds_) {
        orchestrator::updateAsset(hhold.finance, hhold.labor, hhold.goods);
    }
}

void Engine::logging() {
    dropBox_.clear();
    for (Firm& firm : firms_) {
        firm_finance::logging(dropBox_, firm.finance);
        labor_demander::logging(dropBox_, firm.labor);
        goods_supplier::logging(dropBox_, firm.goods);
    }
    for (HHold& hhold : hholds_) {
        hhold_finance::logging(dropBox_, hhold.finance);
        labor_supplier::logging(dropBox_, hhold.labor);
    }
}

void Engine::reset() {
    for (Firm& firm : firms_) {
        labor_demander::reset(firm.labor);
        goods_supplier::reset(firm.goods);
    }
    for (HHold& hhold : hholds_) {
        labor_supplier::reset(hhold.labor);
        goods_demander::reset(hhold.goods);
    }

    laborRequestBox_.clear();
    goodsEntryBox_.clear();
    ++config::currentStep;
}

void Logger::save(const world::CensusDropBox& dropBox, const int step) {
    std::string     groupPath{"/step_" + std::to_string(step)};
    HighFive::Group group{file_.createGroup(groupPath)};

    group.createDataSet("prices", dropBox.prices_);
}
}  // namespace core