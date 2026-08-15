#pragma once

#include <tbb/concurrent_vector.h>
#include <algorithm>
#include <cstddef>
#include <functional>
#include <optional>
#include <ranges>
#include <vector>

#include "components/labor_demander/util.hpp"
#include "core/values/common.hpp"
#include "core/values/labor.hpp"
#include "world/message.hpp"

namespace abm::labor::demander {
inline auto sortApplicants(const HeadCount offer, tbb::concurrent_vector<LaborEntry>& entryBox)
    -> auto {
    using EntryRef = std::reference_wrapper<LaborEntry>;
    static thread_local std::vector<EntryRef> applicants;
    applicants.clear();
    const std::size_t k{std::min(entryBox.size(), static_cast<std::size_t>(offer.value()))};
    for (LaborEntry& entry : entryBox) applicants.emplace_back(std::ref(entry));
    const bool isOver{entryBox.size() > static_cast<std::size_t>(offer.value())};

    const auto toRawRef{[](EntryRef entryRef) -> LaborEntry& { return entryRef.get(); }};
    if (not isOver) return applicants | std::views::transform(toRawRef);

    std::ranges::nth_element(
        applicants,
        applicants.begin() + static_cast<int>(k),
        std::ranges::greater{},
        [](const EntryRef entryRef) -> double { return entryRef.get().productPower; }
    );
    return applicants | std::views::transform(toRawRef);
}

class [[nodiscard]] OfferApplicants {
    template <typename T>
    using RefWrap = std::reference_wrapper<T>;

  public:
    OfferApplicants() = default;

    void add(LaborEntry& entry) { applicants_.emplace_back(std::ref(entry)); }
    void clear() { applicants_.clear(); }
    auto offerAcceptedApplicants() -> std::ranges::view auto {
        return applicants_ | std::views::transform([](RefWrap<LaborEntry> ref) -> LaborEntry& {
                   return ref.get();
               }) |
               std::views::filter(&LaborEntry::isAccept);
    }

  private:
    std::vector<RefWrap<LaborEntry>> applicants_;
};

class [[nodiscard]] Ledger {
  public:
    Ledger() = default;

    void makeNewPage(const HeadCount offerPlan) {
        offerPlan_   = offerPlan;
        wasPageMade_ = true;
    }
    auto remainOffer() const -> HeadCount { return remainOffer_; }
    auto applicants() const -> HeadCount { return applicants_; }
    void countUpOffer() { --remainOffer_; }
    void addApplicants(const HeadCount add) { applicants_ += add; }
    void addEmploy(const HeadCount add) { employ_ = add; }
    auto publishResult() const -> std::optional<RecruitmentResult> {
        if (not wasPageMade_) return std::nullopt;
        return RecruitmentResult{.applicants = applicants_, .employ = employ_};
    }
    void reset() {
        if (not wasPageMade_) return;
        offerPlan_   = HeadCount{0.0};
        remainOffer_ = HeadCount{0.0};
        applicants_  = HeadCount{0.0};
        employ_      = HeadCount{0.0};
        wasPageMade_ = false;
    }

  private:
    HeadCount offerPlan_{0.0};
    HeadCount remainOffer_{0.0};
    HeadCount applicants_{0.0};
    HeadCount employ_{0.0};
    bool      wasPageMade_{false};
};

class [[nodiscard]] Recruiter {
  public:
    Recruiter() = default;

    void post(
        const AgentID id, const RecruitPlan& plan, tbb::concurrent_vector<LaborRequest>& requestBox
    ) {
        if (plan.offer == HeadCount{0.0}) return;
        isPosting_ = true;
        auto it{requestBox.emplace_back(id, plan.wage)};
        myRequest_ = *it;
        ledger_.makeNewPage(plan.offer);
    }

    void offer() {
        if (not isPosting_) return;
        if (myRequest_->entryBox.empty()) return;

        std::ranges::view auto applicants{
            sortApplicants(ledger_.remainOffer(), myRequest_->entryBox) |
            std::views::take(ledger_.remainOffer().value())
        };

        for (auto& entry : applicants) {
            entry.isOffer = true;
            offerApplicants_.add(entry);
            ledger_.countUpOffer();
        }
        ledger_.addApplicants(HeadCount{myRequest_->entryBox.size()});
    }

    void registerMember(AddRosterFn auto&& addRoster) {
        if (not isPosting_) return;

        HeadCount              employCnt{0.0};
        std::ranges::view auto acceptApplicants{offerApplicants_.offerAcceptedApplicants()};
        for (LaborEntry& acceptApplicant : acceptApplicants) {
            acceptApplicant.rosterEntry = addRoster(acceptApplicant.hholdID, myRequest_->wage);
            ++employCnt;
        }
        ledger_.addEmploy(employCnt);
    }

    auto publishResult() const -> std::optional<RecruitmentResult> {
        return ledger_.publishResult();
    }

    void endStep() {
        myRequest_.reset();
        ledger_.reset();
        offerApplicants_.clear();
        isPosting_ = false;
    }

  private:
    std::optional<LaborRequest&> myRequest_{std::nullopt};
    Ledger                       ledger_;
    OfferApplicants              offerApplicants_;
    bool                         isPosting_{false};
};
}  // namespace abm::labor::demander