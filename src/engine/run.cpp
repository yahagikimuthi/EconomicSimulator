#include "core/engine.hpp"

#include <entt/entt.hpp>
#include <strategies/common.hpp>
#include <strategies/goods_demander.hpp>
#include <strategies/goods_supplier.hpp>
#include <strategies/labor_demander.hpp>
#include <strategies/labor_supplier.hpp>
#include <string>

#include "components/common.hpp"
#include "components/goods_demander.hpp"
#include "components/goods_supplier.hpp"
#include "components/labor_demander.hpp"
#include "components/labor_supplier.hpp"
#include "config/sim_vars.hpp"
#include "strategies/orchestrator.hpp"
#include "world/message.hpp"

namespace core {
void Engine::run() {}

void Engine::runLabor() {
    auto firmView{registry_.view<
        agent_index::Component,
        goods_supplier::Component,
        labor_demander::Component,
        FirmTag>()};
    auto hholdView{registry_.view<agent_index::Component, labor_supplier::Component, HHoldTag>()};

    for (auto firm : firmView) {
        orchestrator::postLaborRequest(
            firmView.get<agent_index::Component>(firm),
            firmView.get<goods_supplier::Component>(firm),
            firmView.get<labor_demander::Component>(firm),
            laborRequestBox_
        );
    }

    for (auto hhold : hholdView) {
        orchestrator::jobEntry(
            hholdView.get<agent_index::Component>(hhold),
            hholdView.get<labor_supplier::Component>(hhold),
            laborRequestBox_
        );
    }

    for (auto firm : firmView) {
        orchestrator::offer(firmView.get<labor_demander::Component>(firm));
    }

    for (auto hhold : hholdView) {
        orchestrator::acceptOffer(hholdView.get<labor_supplier::Component>(hhold));
    }

    for (auto firm : firmView) {
        orchestrator::registerMember(
            firmView.get<goods_supplier::Component>(firm),
            firmView.get<labor_demander::Component>(firm)
        );
    }
}

void Engine::runGoods() {
    auto firmView{registry_.view<labor_demander::Component, goods_supplier::Component, FirmTag>()};
    auto hholdView{registry_.view<
        hhold_finance::Component,
        labor_supplier::Component,
        goods_demander::Component,
        HHoldTag>()};

    for (auto firm : firmView) {
        orchestrator::postGoods(
            firmView.get<goods_supplier::Component>(firm),
            firmView.get<labor_demander::Component>(firm),
            goodsEntryBox_
        );
    }

    for (auto hhold : hholdView) {
        orchestrator::purchase(
            hholdView.get<hhold_finance::Component>(hhold),
            hholdView.get<goods_demander::Component>(hhold),
            hholdView.get<labor_supplier::Component>(hhold),
            goodsEntryBox_
        );
    }

    for (auto firm : firmView) {
        orchestrator::trade(firmView.get<goods_supplier::Component>(firm));
    }
}

void Engine::update() {
    auto firmView{registry_.view<
        firm_finance::Component,
        labor_demander::Component,
        goods_supplier::Component,
        FirmTag>()};
    auto hholdView{registry_.view<
        hhold_finance::Component,
        labor_supplier::Component,
        goods_demander::Component,
        HHoldTag>()};

    for (auto firm : firmView) {
        orchestrator::updateAsset(
            firmView.get<firm_finance::Component>(firm),
            firmView.get<labor_demander::Component>(firm),
            firmView.get<goods_supplier::Component>(firm)
        );
    }

    for (auto hhold : hholdView) {
        orchestrator::updateAsset(
            hholdView.get<hhold_finance::Component>(hhold),
            hholdView.get<labor_supplier::Component>(hhold),
            hholdView.get<goods_demander::Component>(hhold)
        );
    }
}

void Engine::logging() {
    auto firmFinances{registry_.view<firm_finance::Component, FirmTag>()};
    auto hholdFinances{registry_.view<hhold_finance::Component, HHoldTag>()};
    auto laborDemanders{registry_.view<labor_demander::Component, FirmTag>()};
    auto laborSuppliers{registry_.view<labor_supplier::Component, HHoldTag>()};
    auto goodsSuppliers{registry_.view<goods_supplier::Component, FirmTag>()};

    for (auto firmFinance : firmFinances) {
        firm_finance::logging(dropBox_, firmFinances.get<firm_finance::Component>(firmFinance));
    }
    for (auto hholdFinance : hholdFinances) {
        hhold_finance::logging(dropBox_, hholdFinances.get<hhold_finance::Component>(hholdFinance));
    }
    for (auto laborDemander : laborDemanders) {
        labor_demander::logging(
            dropBox_, laborDemanders.get<labor_demander::Component>(laborDemander)
        );
    }
    for (auto laborSupplier : laborSuppliers) {
        labor_supplier::logging(
            dropBox_, laborSuppliers.get<labor_supplier::Component>(laborSupplier)
        );
    }
    for (auto goodsSupplier : goodsSuppliers) {
        goods_supplier::logging(
            dropBox_, goodsSuppliers.get<goods_supplier::Component>(goodsSupplier)
        );
    }
}

void Engine::reset() {
    auto laborDemanders{registry_.view<labor_demander::Component>()};
    auto laborSuppliers{registry_.view<labor_supplier::Component>()};
    auto goodsDemanders{registry_.view<goods_demander::Component>()};
    auto goodsSuppliers{registry_.view<goods_supplier::Component>()};

    for (auto laborDemander : laborDemanders) {
        labor_demander::reset(laborDemanders.get<labor_demander::Component>(laborDemander));
    }
    for (auto laborSupplier : laborSuppliers) {
        labor_supplier::reset(laborSuppliers.get<labor_supplier::Component>(laborSupplier));
    }
    for (auto goodsDemander : goodsDemanders) {
        goods_demander::reset(goodsDemanders.get<goods_demander::Component>(goodsDemander));
    }
    for (auto goodsSupplier : goodsSuppliers) {
        goods_supplier::reset(goodsSuppliers.get<goods_supplier::Component>(goodsSupplier));
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