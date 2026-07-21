#include "core/engine.hpp"

#include <highfive/H5DataSet.hpp>
#include <highfive/H5File.hpp>
#include <string>

#include "config.hpp"
#include "strategies/common.hpp"
#include "strategies/orchestrator.hpp"
#include "world/message.hpp"

namespace core {
void Engine::run() {
    for (currentStep_ = 0; currentStep_ < totalStep_; ++currentStep_) {
        runLabor();
        runGoods();
        logging();
        reset();
    }
}

void Engine::runLabor() {
    using namespace orchestrator;
    for (Firm& firm : firms_) {
        labor::postLaborRequest(
            firm.index, firm.goods, firm.labor, laborRequestBox_, companyBoards_
        );
    }

    for (HHold& hhold : hholds_) {
        labor::jobEntry(hhold.index, hhold.labor, laborRequestBox_);
    }

    for (Firm& firm : firms_) {
        labor::offer(firm.labor);
    }

    for (HHold& hhold : hholds_) {
        labor::acceptOffer(hhold.labor);
    }

    for (Firm& firm : firms_) {
        labor::registerMember(firm.goods, firm.labor);
    }

    for (Firm& firm : firms_) {
        labor::endStep(firm.finance, firm.labor, dropBox_);
    }
    for (HHold& hhold : hholds_) {
        labor::endStep(hhold.finance, hhold.labor, dropBox_);
    }
}

void Engine::runGoods() {
    using namespace orchestrator;
    for (Firm& firm : firms_) {
        goods::postGoods(firm.goods, firm.labor, goodsEntryBox_);
    }

    for (HHold& hhold : hholds_) {
        goods::purchase(hhold.finance, hhold.goods, hhold.labor, goodsEntryBox_, currentStep_);
    }

    for (Firm& firm : firms_) {
        goods::trade(firm.goods);
    }

    for (HHold& hhold : hholds_) {
        goods::afterTrade(hhold.goods);
    }

    for (Firm& firm : firms_) {
        goods::endStep(firm.finance, firm.goods, dropBox_);
    }

    for (HHold& hhold : hholds_) {
        goods::endStep(hhold.finance, hhold.goods);
    }
}

void Engine::logging() {
    for (Firm& firm : firms_) {
        firm_finance::logging(dropBox_, firm.finance);
    }
    for (HHold& hhold : hholds_) {
        hhold_finance::logging(dropBox_, hhold.finance);
    }
    logger_.save(dropBox_, currentStep_);
    dropBox_.clear();
}

void Engine::reset() {
    laborRequestBox_.clear();
    goodsEntryBox_.clear();
}

void Logger::save(const world::CensusDropBox& dropBox, const int step) {
    namespace name = config::save_name;
    std::string     groupPath{"/step_" + std::to_string(step)};
    HighFive::Group group{file_.createGroup(groupPath)};

    auto create{[&group](std::string_view dataName, const std::vector<double>& data) -> void {
        group.createDataSet(static_cast<std::string>(dataName), data);
    }};

    create(name::firmAssets, dropBox.firmAssets_);
    create(name::postedEmployments, dropBox.postedEmployments_);
    create(name::employments, dropBox.employments_);
    create(name::prices, dropBox.prices_);
    create(name::supplies, dropBox.supplies_);
    create(name::markups, dropBox.markups_);
    create(name::inventories, dropBox.inventories_);
    create(name::householdAssets, dropBox.hholdAssets_);
    create(name::wages, dropBox.wages_);
}
}  // namespace core