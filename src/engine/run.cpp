#include "core/engine.hpp"

#include <entt/entt.hpp>
#include <strategies/orchestrator.hpp>
#include <string>

#include "components/common.hpp"
#include "components/goods_demander.hpp"
#include "components/goods_supplier.hpp"
#include "components/labor_demander.hpp"
#include "components/labor_supplier.hpp"
#include "config/sim_vars.hpp"

namespace core {
void Engine::run() {}

void Engine::runLabor() {
    auto firmView{
        registry_
            .view<agent_index::Component, goods_supplier::Component, labor_demander::Component>()
    };
    auto hholdView{registry_.view<agent_index::Component, labor_supplier::Component>()};

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

void Engine::reset() {
    auto view{registry_.view<
        labor_demander::Component,
        labor_supplier::Component,
        goods_demander::Component,
        goods_supplier::Component>()};

    for (auto entity : view) {
        orchestrator::reset(
            view.get<labor_demander::Component>(entity),
            view.get<labor_supplier::Component>(entity),
            view.get<goods_demander::Component>(entity),
            view.get<goods_supplier::Component>(entity)
        );
    }

    laborRequestBox_.clear();
    goodsEntryBox_.clear();
    ++config::currentStep;
}

void Logger::save(const int step) {
    std::string     groupPath{"/step_" + std::to_string(step)};
    HighFive::Group group{file_.createGroup(groupPath)};

    group.createDataSet("prices", prices_);
}
}  // namespace core