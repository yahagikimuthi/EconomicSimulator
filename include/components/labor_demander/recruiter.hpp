#pragma once

#include <tbb/concurrent_vector.h>
#include <algorithm>
#include <cstddef>
#include <functional>
#include <optional>
#include <ranges>
#include <utility>
#include <vector>

#include "components/labor_demander/common.hpp"
#include "core/values/common.hpp"
#include "core/values/labor.hpp"
#include "world/message.hpp"

namespace abm::labor::demander {
inline auto sortApplicants(const HeadCount offer, LaborRequest::EntryBoxT entryBox)
    -> std::ranges::view auto {
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

class OfferApplicants {
    template <typename T>
    using RefWrap = std::reference_wrapper<T>;

  public:
    [[nodiscard]] OfferApplicants() = default;

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

struct OfferResult {
    const HeadCount offer;
    const HeadCount applicants;
};

class Ledger {
  public:
    [[nodiscard]] Ledger() = default;

    void makeNewPage(const HeadCount offerPlan) { remainOffer_ = offerPlan; }

    [[nodiscard]] auto remainOffer() const -> HeadCount { return remainOffer_; }

    void readOfferResult(const OfferResult& result) {
        remainOffer_ -= result.offer;
        applicants_ += result.applicants;
    }

    void addEmploy(const HeadCount add) { employ_ = add; }

    [[nodiscard]] auto publishResult() const -> RecruitResult {
        return {.applicants = applicants_, .employ = employ_};
    }

    void reset() {
        remainOffer_ = HeadCount{0.0};
        applicants_  = HeadCount{0.0};
        employ_      = HeadCount{0.0};
    }

  private:
    HeadCount offerPlan_{0.0};
    HeadCount remainOffer_{0.0};
    HeadCount applicants_{0.0};
    HeadCount employ_{0.0};
};

class Recruiter {
  public:
    [[nodiscard]] Recruiter() = default;

    void post(const AgentID id, const RecruitPlan& plan, LaborMarket& laborMarket) {
        if (not shouldPost(plan)) return;
        ledger_.makeNewPage(plan.offer);
        myRequest_ = laborMarket.request(id, plan.wage);
    }

    void offer() {
        if (not isPosting()) return;
        if (myRequest_->entryBox().empty()) return;

        std::ranges::view auto applicants{
            sortApplicants(ledger_.remainOffer(), myRequest_->entryBox()) |
            std::views::take(ledger_.remainOffer().value())
        };

        HeadCount offerCnt{0.0};
        for (auto& entry : applicants) {
            entry.isOffer = true;
            offerApplicants_.add(entry);
            ++offerCnt;
        }
        ledger_.readOfferResult(
            {.offer = offerCnt, .applicants = HeadCount{myRequest_->entryBox().size()}}
        );
    }

    template <AddRosterFn F>
    void registerMember(F&& addRoster) {
        if (not isPosting()) return;
        HeadCount              employCnt{0.0};
        std::ranges::view auto acceptApplicants{offerApplicants_.offerAcceptedApplicants()};
        for (LaborEntry& acceptApplicant : acceptApplicants) {
            acceptApplicant.rosterEntry =
                std::forward<F>(addRoster)(acceptApplicant.hholdID, myRequest_->wage);
            ++employCnt;
        }
        ledger_.addEmploy(employCnt);
    }

    [[nodiscard]] auto publishResult() const -> RecruitResult {
        if (not isPosting()) return {.applicants = HeadCount{0.0}, .employ = HeadCount{0.0}};
        return ledger_.publishResult();
    }

    void endStep() {
        if (not isPosting()) return;
        myRequest_.reset();
        ledger_.reset();
        offerApplicants_.clear();
    }

  private:
    [[nodiscard]] auto isPosting() const -> bool { return myRequest_.has_value(); }

    [[nodiscard]] auto shouldPost(const RecruitPlan& plan) const -> bool {
        return plan.offer > HeadCount{0.0};
    }

    std::optional<LaborRequest&> myRequest_{std::nullopt};
    Ledger                       ledger_;
    OfferApplicants              offerApplicants_;
};
}  // namespace abm::labor::demander