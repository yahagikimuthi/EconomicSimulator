#include "abm.hpp"

#include <algorithm>
#include <execution>
#include <highfive/H5DataSet.hpp>
#include <highfive/H5File.hpp>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "analysis/analysis.hpp"
#include "core/setting.hpp"
#include "orchestrator/consumer_goods.hpp"
#include "orchestrator/labor.hpp"
#include "orchestrator/production_goods.hpp"
#include "orchestrator/updates_loggings.hpp"
#include "world/common.hpp"

namespace abm {
namespace {
template <typename T>
    requires requires(T t) { t.finance.asset().value(); }
[[nodiscard]] auto calcSumAsset(std::vector<T>& agents) noexcept -> double {
    return std::ranges::fold_left(agents, 0.0, [](const double acc, const T& agent) -> double {
        return acc + agent.finance.asset().value();
    });
}

template <typename T, typename F>
    requires std::invocable<F, T&> and requires(std::vector<T>& agents, F&& f) {
        std::for_each(std::execution::par, agents.begin(), agents.end(), std::forward<F>(f));
    }
void forEach(std::vector<T>& agents, F f) {
    std::for_each(std::execution::par, agents.begin(), agents.end(), (f));
}
}  // namespace

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
    forEach(bToCFirms_, [&](BtoCFirm& firm) -> void {
        labor::adjustWorkforce(
            firm.id, firm.consumerGoodsSupplier, firm.laborDemander, laborMarket_
        );
    });

    forEach(bToBFirms_, [&](BtoBFirm& firm) -> void {
        labor::adjustWorkforce(
            firm.id, firm.productionGoodsSupplier, firm.laborDemander, laborMarket_
        );
    });

    forEach(hholds_, [&](HHold& hhold) -> void {
        labor::jobEntry(hhold.id, hhold.laborSupplier, laborMarket_);
    });

    forEach(bToCFirms_, [](BtoCFirm& firm) -> void { labor::offer(firm.laborDemander); });
    forEach(bToBFirms_, [](BtoBFirm& firm) -> void { labor::offer(firm.laborDemander); });

    forEach(hholds_, [](HHold& hhold) -> void { labor::acceptOffer(hhold.laborSupplier); });

    forEach(bToCFirms_, [](BtoCFirm& firm) -> void {
        labor::registerMember(firm.consumerGoodsSupplier, firm.laborDemander);
    });
    forEach(bToBFirms_, [](BtoBFirm& firm) -> void {
        labor::registerMember(firm.productionGoodsSupplier, firm.laborDemander);
    });

    forEach(hholds_, [](HHold& hhold) -> void { labor::recordRosterEntry(hhold.laborSupplier); });

    forEach(bToCFirms_, [](BtoCFirm& firm) -> void {
        labor::acceptResignation(firm.laborDemander);
    });
    forEach(bToBFirms_, [](BtoBFirm& firm) -> void {
        labor::acceptResignation(firm.laborDemander);
    });

    forEach(bToCFirms_, [&](BtoCFirm& firm) -> void {
        labor::endStep(firm.finance, firm.laborDemander, dropBox_);
    });
    forEach(bToBFirms_, [&](BtoBFirm& firm) -> void {
        labor::endStep(firm.finance, firm.laborDemander, dropBox_);
    });

    forEach(hholds_, [&](HHold& hhold) -> void {
        labor::endStep(hhold.finance, hhold.laborSupplier, dropBox_);
    });
}

void Engine::runProductionGoods() noexcept {
    forEach(hholds_, [](HHold& hhold) -> void {
        production_goods::product(hhold.laborSupplier, EMarket::productionGoods);
    });

    forEach(bToBFirms_, [&](BtoBFirm& firm) -> void {
        production_goods::postGoods(
            firm.id, firm.productionGoodsSupplier, firm.laborDemander, productionGoodsMarket_
        );
    });

    forEach(bToCFirms_, [&](BtoCFirm& firm) -> void {
        production_goods::purchase(
            firm.id,
            firm.finance,
            firm.productionGoodsDemander,
            firm.consumerGoodsSupplier,
            productionGoodsMarket_
        );
    });

    forEach(bToBFirms_, [&](BtoBFirm& firm) -> void {
        production_goods::purchase(
            firm.id,
            firm.finance,
            firm.productionGoodsDemander,
            firm.productionGoodsSupplier,
            productionGoodsMarket_
        );
    });

    forEach(bToBFirms_, [](BtoBFirm& firm) -> void {
        production_goods::trade(firm.productionGoodsSupplier);
    });

    forEach(bToCFirms_, [](BtoCFirm& firm) -> void {
        production_goods::afterTrade(firm.productionGoodsDemander);
    });
    forEach(bToBFirms_, [](BtoBFirm& firm) -> void {
        production_goods::afterTrade(firm.productionGoodsDemander);
    });

    forEach(bToBFirms_, [&](BtoBFirm& firm) -> void {
        production_goods::endStep(
            firm.finance, firm.productionGoodsSupplier, firm.productionGoodsDemander
        );
        production_goods::endStep(firm.finance, firm.productionGoodsSupplier, dropBox_);
    });

    forEach(bToCFirms_, [](BtoCFirm& firm) -> void {
        production_goods::endStep(
            firm.finance, firm.consumerGoodsSupplier, firm.productionGoodsDemander
        );
    });
}

void Engine::runConsumerGoods() noexcept {
    forEach(hholds_, [](HHold& hhold) -> void {
        consumer_goods::product(hhold.laborSupplier, EMarket::consumerGoods);
    });

    forEach(bToCFirms_, [&](BtoCFirm& firm) -> void {
        consumer_goods::postGoods(
            firm.consumerGoodsSupplier, firm.laborDemander, consumerGoodsMarket_
        );
    });

    forEach(hholds_, [&](HHold& hhold) -> void {
        consumer_goods::purchase(
            hhold.finance, hhold.consumerGoodsDemander, consumerGoodsMarket_, currentStep_
        );
    });

    forEach(bToCFirms_, [](BtoCFirm& firm) -> void {
        consumer_goods::trade(firm.consumerGoodsSupplier);
    });

    forEach(hholds_, [](HHold& hhold) -> void {
        consumer_goods::afterTrade(hhold.consumerGoodsDemander);
    });

    forEach(bToCFirms_, [&](BtoCFirm& firm) -> void {
        consumer_goods::endStep(firm.finance, firm.consumerGoodsSupplier, dropBox_);
    });

    forEach(hholds_, [](HHold& hhold) -> void {
        consumer_goods::endStep(hhold.finance, hhold.consumerGoodsDemander);
    });
}

void Engine::logging() {
    forEach(bToCFirms_, [&](BtoCFirm& firm) -> void {
        firm_finance::logging(dropBox_, firm.finance);
    });

    forEach(bToBFirms_, [&](BtoBFirm& firm) -> void {
        firm_finance::logging(dropBox_, firm.finance);
    });

    forEach(hholds_, [&](HHold& hhold) -> void {
        hhold_finance::logging(dropBox_, hhold.finance);
    });

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

    auto create{
        [&group](std::string_view dataName, const tbb::concurrent_vector<double>& data) -> void {
            std::vector<double> vec(data.begin(), data.end());
            group.createDataSet(static_cast<std::string>(dataName), vec);
        }
    };

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