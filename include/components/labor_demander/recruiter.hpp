#pragma once

#include <tbb/concurrent_vector.h>
#include <algorithm>
#include <cstddef>
#include <functional>
#include <memory>
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

struct OfferResult {
    const HeadCount offer;
    const HeadCount applicants;
};

class [[nodiscard]] Ledger {
  public:
    Ledger() = default;

    void makeNewPage(const HeadCount offerPlan) {
        remainOffer_ = offerPlan;
        wasPageMade_ = true;
    }

    auto remainOffer() const -> HeadCount { return wasPageMade_ ? remainOffer_ : HeadCount{0.0}; }

    void readOfferResult(const OfferResult& result) {
        if (not wasPageMade_) return;
        remainOffer_ -= result.offer;
        applicants_ += result.applicants;
    }

    void addEmploy(const HeadCount add) {
        if (not wasPageMade_) return;
        employ_ = add;
    }

    auto publishResult() const -> std::optional<RecruitResult> {
        if (not wasPageMade_) return std::nullopt;
        return RecruitResult{.applicants = applicants_, .employ = employ_};
    }

    void reset() {
        if (not wasPageMade_) return;
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

class [[nodiscard]] MarketMediator {
  public:
    MarketMediator(const AgentID id, LaborMarket& market)
        : id_{id}, nullRequest_{id_, Wage{1.0}}, market_{market} {}

    auto entry(const RecruitPlan& plan) -> LaborRequest& {
        if (plan.offer > HeadCount{0.0}) return market_.request(id_, plan.wage);
        std::destroy_at(&nullRequest_);
        std::construct_at(&nullRequest_, id_, plan.wage);
        return nullRequest_;
    }

  private:
    const AgentID id_;
    LaborRequest  nullRequest_;
    LaborMarket&  market_;
};

class [[nodiscard]] Recruiter {
  public:
    Recruiter() = default;

    void post(const AgentID id, const RecruitPlan& plan, LaborMarket& laborMarket) {
        ledger_.makeNewPage(plan.offer);
        myRequest_ = laborMarket.request(id, plan.wage);
    }

    void offer() {
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
        HeadCount              employCnt{0.0};
        std::ranges::view auto acceptApplicants{offerApplicants_.offerAcceptedApplicants()};
        for (LaborEntry& acceptApplicant : acceptApplicants) {
            acceptApplicant.rosterEntry =
                std::forward<F>(addRoster)(acceptApplicant.hholdID, myRequest_->wage);
            ++employCnt;
        }
        ledger_.addEmploy(employCnt);
    }

    auto publishResult() const -> std::optional<RecruitResult> { return ledger_.publishResult(); }

    void endStep() {
        myRequest_.reset();
        ledger_.reset();
        offerApplicants_.clear();
    }

  private:
    std::optional<LaborRequest&> myRequest_{std::nullopt};
    Ledger                       ledger_;
    OfferApplicants              offerApplicants_;
};
}  // namespace abm::labor::demander