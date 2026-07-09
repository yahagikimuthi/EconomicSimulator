#pragma once

#include <tbb/concurrent_vector.h>
#include <functional>

#include "components/labor_demander.hpp"
#include "core/forward.hpp"

namespace labor_demander {
struct PostJobView {
    PostJobView(Component& comp) : comp_{comp} {}

    void setPlan(const double wage, const int employ) {
        comp_.plan_.wage_ = wage, comp_.plan_.employ_ = employ;
    }
    void setMyRequest(const tbb::concurrent_vector<world::LaborRequest>::iterator it) {  // NOLINT
        comp_.posting_.myRequest_ = it;
    }
    [[nodiscard]] auto getLog() const -> std::tuple<double, double, double> {
        const auto& log = comp_.log_;
        return {log.wage_, log.targetEmploy_, log.actualEmploy_};
    }
    [[nodiscard]] auto wageAdjustVol() const -> double {
        return comp_.parameter_.wageAdjustmentVolatility_;
    }
    [[nodiscard]] auto employAdjustVol() const -> double {
        return comp_.parameter_.employAdjustmentVolatility_;
    }
    [[nodiscard]] auto lastTargetEmploy() const -> double { return comp_.log_.targetEmploy_; }

  private:
    Component& comp_;
};

void postJob(
    const int                                    id,
    const bool                                   isSold,
    tbb::concurrent_vector<world::LaborRequest>& requestBox,
    PostJobView                                  view
);

struct OfferApplicantsView {
    OfferApplicantsView(Component& comp) : comp_{comp} {}

    [[nodiscard]] auto getEmploy() const -> int { return comp_.plan_.employ_; }

    [[nodiscard]] auto getMyRequest() const
        -> tbb::concurrent_vector<world::LaborRequest>::iterator {
        return comp_.posting_.myRequest_;
    }

    void clearOfferVec() { comp_.posting_.offerApplicants_.clear(); }
    void recordOffer(world::LaborEntry& entry) {
        comp_.posting_.offerApplicants_.emplace_back(entry);
    }

  private:
    Component& comp_;
};

void offerApplicants(OfferApplicantsView view);

struct RegisterMemberView {
    RegisterMemberView(Component& comp) : comp_{comp} {};

    [[nodiscard]] auto getMyRequest() const
        -> tbb::concurrent_vector<world::LaborRequest>::iterator {
        return comp_.posting_.myRequest_;
    }
    [[nodiscard]] auto getOfferApplicants() const
        -> std::vector<std::reference_wrapper<world::LaborEntry>> {
        return comp_.posting_.offerApplicants_;
    }

    [[nodiscard]] auto getTargetEmploy() const -> int { return comp_.plan_.employ_; }

    void setLog(const double wage, const int targetEmploy, const int actualEmploy) {
        auto& log = comp_.log_;
        log.wage_ = wage, log.targetEmploy_ = targetEmploy, log.actualEmploy_ = actualEmploy;
    }

  private:
    Component& comp_;
};

[[nodiscard]] auto registerMember(RegisterMemberView view) -> double;
}  // namespace labor_demander