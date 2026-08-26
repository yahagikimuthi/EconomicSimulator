#include "engine.hpp"

#include <tbb/concurrent_vector.h>
#include <algorithm>
#include <execution>
#include <highfive/H5DataSet.hpp>
#include <highfive/H5File.hpp>
#include <ranges>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "analysis/analysis.hpp"
#include "core/setting.hpp"
#include "core/values/common.hpp"
#include "orchestrator/consumer_goods.hpp"
#include "orchestrator/labor.hpp"
#include "orchestrator/production_goods.hpp"
#include "orchestrator/updates_loggings.hpp"
#include "world/common.hpp"

namespace abm {
namespace {
template <typename T>
    requires requires(T t) { t.finance.asset().value(); }
[[nodiscard]] auto calcSumAsset(const std::vector<T>& agents) noexcept -> double {
    return std::ranges::fold_left(
        agents | std::views::transform([](const T& agent) noexcept -> double {
            return agent.finance.asset().value();
        }),
        0.0,
        std::plus{}
    );
}

template <typename T, typename F>
    requires std::invocable<F, T&> and requires(std::vector<T>& agents, F&& f) {
        std::for_each(std::execution::par, agents.begin(), agents.end(), std::forward<F>(f));
    }
void forEach(std::vector<T>& agents, F&& f) {
    std::for_each(agents.begin(), agents.end(), std::forward<F>(f));
}
}  // namespace

void Engine::run() noexcept {
    for (currentStep_ = Step{0}; currentStep_ < totalStep_; ++currentStep_) {
        if (currentStep_ == Step{500}) {
            ignore();
        }
        runLabor();
        runProductionGoods();
        runConsumerGoods();
        endAllStep();
        reset();
    }
    if (isAnalysis_) {
        analysis::analysisData();
    }
}

void Engine::runLabor() noexcept {
    forEach(bToCFirms_, [&](BtoCFirm& firm) noexcept -> void {
        labor::adjustWorkforce(
            firm.id, firm.consumerGoodsSupplier, firm.laborDemander, laborMarket_
        );
    });

    forEach(bToBFirms_, [&](BtoBFirm& firm) noexcept -> void {
        labor::adjustWorkforce(
            firm.id, firm.productionGoodsSupplier, firm.laborDemander, laborMarket_
        );
    });

    forEach(hholds_, [&](HHold& hhold) noexcept -> void {
        labor::jobEntry(hhold.id, hhold.laborSupplier, laborMarket_);
    });

    forEach(bToCFirms_, [](BtoCFirm& firm) noexcept -> void { labor::offer(firm.laborDemander); });
    forEach(bToBFirms_, [](BtoBFirm& firm) noexcept -> void { labor::offer(firm.laborDemander); });

    forEach(hholds_, [](HHold& hhold) noexcept -> void {
        labor::acceptOffer(hhold.laborSupplier);
    });

    forEach(bToCFirms_, [](BtoCFirm& firm) noexcept -> void {
        labor::registerMember(firm.consumerGoodsSupplier, firm.laborDemander);
    });
    forEach(bToBFirms_, [](BtoBFirm& firm) noexcept -> void {
        labor::registerMember(firm.productionGoodsSupplier, firm.laborDemander);
    });

    forEach(hholds_, [](HHold& hhold) noexcept -> void {
        labor::recordRosterEntry(hhold.laborSupplier);
    });

    forEach(bToCFirms_, [](BtoCFirm& firm) noexcept -> void {
        labor::acceptResignation(firm.laborDemander);
    });
    forEach(bToBFirms_, [](BtoBFirm& firm) noexcept -> void {
        labor::acceptResignation(firm.laborDemander);
    });

    forEach(bToCFirms_, [&](BtoCFirm& firm) noexcept -> void {
        labor::endStep(firm.finance, firm.laborDemander, dropBox_);
    });
    forEach(bToBFirms_, [&](BtoBFirm& firm) noexcept -> void {
        labor::endStep(firm.finance, firm.laborDemander, dropBox_);
    });

    forEach(hholds_, [&](HHold& hhold) noexcept -> void {
        labor::endStep(hhold.finance, hhold.laborSupplier, government_, dropBox_);
    });
}

void Engine::runProductionGoods() noexcept {
    forEach(hholds_, [](HHold& hhold) noexcept -> void {
        production_goods::product(hhold.laborSupplier, EMarket::ProductionGoods);
    });

    forEach(bToBFirms_, [&](BtoBFirm& firm) noexcept -> void {
        production_goods::postGoods(
            firm.id, firm.productionGoodsSupplier, firm.laborDemander, productionGoodsMarket_
        );
    });

    forEach(bToCFirms_, [&](BtoCFirm& firm) noexcept -> void {
        production_goods::purchase(
            firm.id,
            firm.finance,
            firm.productionGoodsDemander,
            firm.consumerGoodsSupplier,
            productionGoodsMarket_
        );
    });

    forEach(bToBFirms_, [&](BtoBFirm& firm) noexcept -> void {
        production_goods::purchase(
            firm.id,
            firm.finance,
            firm.productionGoodsDemander,
            firm.productionGoodsSupplier,
            productionGoodsMarket_
        );
    });

    forEach(bToBFirms_, [](BtoBFirm& firm) noexcept -> void {
        production_goods::trade(firm.productionGoodsSupplier);
    });

    forEach(bToCFirms_, [](BtoCFirm& firm) noexcept -> void {
        production_goods::afterTrade(firm.productionGoodsDemander);
    });
    forEach(bToBFirms_, [](BtoBFirm& firm) noexcept -> void {
        production_goods::afterTrade(firm.productionGoodsDemander);
    });

    forEach(bToBFirms_, [&](BtoBFirm& firm) noexcept -> void {
        production_goods::endStep(
            firm.finance,
            firm.productionGoodsSupplier,
            firm.productionGoodsDemander,
            government_,
            dropBox_
        );
    });

    forEach(bToCFirms_, [](BtoCFirm& firm) noexcept -> void {
        production_goods::endStep(
            firm.finance, firm.consumerGoodsSupplier, firm.productionGoodsDemander
        );
    });
}

void Engine::runConsumerGoods() noexcept {
    forEach(hholds_, [](HHold& hhold) noexcept -> void {
        consumer_goods::product(hhold.laborSupplier, EMarket::ConsumerGoods);
    });

    forEach(bToCFirms_, [&](BtoCFirm& firm) noexcept -> void {
        consumer_goods::postGoods(
            firm.consumerGoodsSupplier, firm.laborDemander, consumerGoodsMarket_
        );
    });

    forEach(hholds_, [&](HHold& hhold) noexcept -> void {
        consumer_goods::purchase(
            hhold.finance, hhold.consumerGoodsDemander, consumerGoodsMarket_, currentStep_
        );
    });

    forEach(bToCFirms_, [](BtoCFirm& firm) noexcept -> void {
        consumer_goods::trade(firm.consumerGoodsSupplier);
    });

    forEach(hholds_, [](HHold& hhold) noexcept -> void {
        consumer_goods::afterTrade(hhold.consumerGoodsDemander);
    });

    forEach(bToCFirms_, [&](BtoCFirm& firm) noexcept -> void {
        consumer_goods::endStep(firm.finance, firm.consumerGoodsSupplier, government_, dropBox_);
    });

    forEach(hholds_, [](HHold& hhold) noexcept -> void {
        consumer_goods::endStep(hhold.finance, hhold.consumerGoodsDemander);
    });
}

void Engine::endAllStep() noexcept {
    forEach(bToCFirms_, [&](BtoCFirm& firm) noexcept -> void {
        firm_finance::endAllStep(firm.finance, government_, dropBox_);
    });

    forEach(bToBFirms_, [&](BtoBFirm& firm) noexcept -> void {
        firm_finance::endAllStep(firm.finance, government_, dropBox_);
    });

    forEach(hholds_, [&](HHold& hhold) noexcept -> void {
        hhold_finance::endAllStep(hhold.finance, hhold.laborSupplier, government_, dropBox_);
    });

    logger_.save(dropBox_, currentStep_);
}

void Engine::reset() noexcept {
    dropBox_.clear();
    laborMarket_.clear();
    productionGoodsMarket_.clear();
    consumerGoodsMarket_.clear();
    government_.reset();
}

void Logger::save(const CensusDropBox& dropBox, const Step step) noexcept {
    namespace name = setting::save_name;
    auto groupPath = std::string{"/step_" + std::to_string(step.value())};
    auto group     = HighFive::Group{file_.createGroup(groupPath)};

    auto create =
        [&group](std::string_view dataName, const std::vector<double>& data) noexcept -> void {
        group.createDataSet(static_cast<std::string>(dataName), data);
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