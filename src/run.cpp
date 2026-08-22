#include "abm.hpp"

#include <algorithm>
#include <highfive/H5DataSet.hpp>
#include <highfive/H5File.hpp>
#include <print>
#include <ranges>
#include <string>
#include <string_view>
#include <vector>

#include "analysis/analysis.hpp"
#include "core/setting.hpp"
#include "orchestrator/consumer_goods.hpp"
#include "orchestrator/labor.hpp"
#include "orchestrator/updates_loggings.hpp"
#include "world/common.hpp"

namespace abm {
auto Engine::calcSumAsset() const noexcept -> long double {
    const long double firmAsset = std::ranges::fold_left(
        bToCFirms_ | std::views::transform([](const BtoCFirm& firm) -> long double {
            return firm.finance.asset().value();
        }),
        0LL,
        std::plus{}
    );
    const long double hholdAsset = std::ranges::fold_left(
        hholds_ | std::views::transform([](const HHold& hhold) -> long double {
            return hhold.finance.asset().value();
        }),
        0LL,
        std::plus{}
    );
    return firmAsset + hholdAsset;
}
//! 546ステップ目が明らかにおかしい
void Engine::run() {
    std::println("{}", calcSumAsset());
    for (currentStep_ = Step{0}; currentStep_ < totalStep_; ++currentStep_) {
        runLabor();
        runConsumerGoods();
        logging();
        reset();
    }
    if (isAnalysis_) {
        analysis::analysisData();
    }
}

void Engine::runLabor() noexcept {
    const auto totalCost =
        std::ranges::fold_left(bToCFirms_, 0.0, [](double acc, BtoCFirm& firm) -> double {
            return acc + firm.laborDemander.sumWage().value();
        });
    const auto sumWage =
        std::ranges::fold_left(hholds_, 0.0, [](double acc, HHold& hhold) -> double {
            return acc + hhold.laborSupplier.wage().value();
        });

    std::println("ステップ: {}", currentStep_.value());
    std::println("企業が支払う総賃金: {}", totalCost);
    std::println("家計が受ける装賃金: {}", sumWage);

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
// 399ステップ目で失敗
void Engine::check() const noexcept { ASSERT(calcSumAsset() > 0.0); }

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