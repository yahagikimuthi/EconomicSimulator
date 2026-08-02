#include "core/engine.hpp"

#include <concepts>
#include <highfive/H5DataSet.hpp>
#include <highfive/H5File.hpp>
#include <print>
#include <string>

#include "config.hpp"
#include "orchestrator/goods.hpp"
#include "orchestrator/labor.hpp"
#include "orchestrator/updates_loggings.hpp"
#include "world/message.hpp"

namespace {
template <typename Agent>
    requires requires(Agent agent) {
        { agent.finance.asset() } -> std::same_as<Money>;
    }
[[nodiscard]] auto sumAsset(const std::vector<Agent>& agents) {
    return std::ranges::fold_left(agents, 0.0, [](const double acc, const Agent& agent) -> double {
        return acc + agent.finance.asset().value();
    });
}
}  // namespace

namespace core {
void Engine::run() {
    for (currentStep_ = Step{0}; currentStep_ < totalStep_; ++currentStep_) {
        runLabor();
        runGoods();
        logging();
        check();
        reset();
    }
}

void Engine::runLabor() {
    std::println("労働市場開始");
    std::println("{}", sumAsset(firms_) + sumAsset(hholds_));
    for (Firm& firm : firms_) {
        labor::adjustWorkforce(firm.index, firm.goods, firm.labor, laborRequestBox_);
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

    for (HHold& hhold : hholds_) {
        labor::recordRosterEntry(hhold.labor);
    }

    for (Firm& firm : firms_) {
        labor::acceptResignation(firm.labor);
    }

    for (Firm& firm : firms_) {
        labor::endStep(firm.finance, firm.labor, dropBox_);
    }
    for (HHold& hhold : hholds_) {
        labor::endStep(hhold.finance, hhold.labor, dropBox_);
    }
    std::println("労働市場終了");
    std::println("{}", sumAsset(firms_) + sumAsset(hholds_));
}

void Engine::runGoods() {
    std::println("財市場開始");
    std::println("{}", sumAsset(firms_) + sumAsset(hholds_));
    for (HHold& hhold : hholds_) {
        goods::product(hhold.labor);
    }

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
    std::println("財市場終了");
    std::println("{}", sumAsset(firms_) + sumAsset(hholds_));
}

void Engine::logging() {
    for (Firm& firm : firms_) {
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
    goodsEntryBox_.clear();
}

void Engine::check() const {}

void Logger::save(const world::CensusDropBox& dropBox, const Step step) {
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
}  // namespace core