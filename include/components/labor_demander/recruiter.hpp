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
#include "util.hpp"
#include "world/labor.hpp"

namespace abm::labor::demander::recruiter {
class OfferApplicants final {
  public:
    [[nodiscard]] OfferApplicants() = default;

    void add(Entry& entry) { applicants_.emplace_back(std::ref(entry)); }
    void clear() { applicants_.clear(); }
    auto offerAcceptedApplicants() -> std::ranges::view auto {
        return applicants_ |
               std::views::transform([](RefWrap<Entry> ref) -> Entry& { return ref.get(); }) |
               std::views::filter(&Entry::isAccept);
    }

  private:
    std::vector<RefWrap<Entry>> applicants_;
};

struct OfferResult final {
    const HeadCount offer;
    const HeadCount applicants;
};

struct EmployResult final {
    const HeadCount employ;
};

class Ledger final {
  public:
    [[nodiscard]] Ledger() = default;

    void makeNewPage(const HeadCount offerPlan) { remainOffer_ = offerPlan; }

    [[nodiscard]] auto remainOffer() const -> HeadCount { return remainOffer_; }

    void readOfferResult(const OfferResult& result) {
        remainOffer_ -= result.offer;
        applicants_ += result.applicants;
    }

    void readEmployResult(const EmployResult add) { employ_ += add.employ; }

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

class Recruiter final {
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

        auto applicants = sortApplicants(ledger_.remainOffer(), myRequest_->entryBox()) |
                          std::views::take(ledger_.remainOffer().value());

        auto offerCnt = HeadCount{0.0};
        for (RefWrap<LaborEntry> entryRef : applicants) {
            auto& entry   = entryRef.get();
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
        auto employCnt        = HeadCount{0.0};
        auto acceptApplicants = offerApplicants_.offerAcceptedApplicants();
        for (auto& acceptApplicant : acceptApplicants) {
            acceptApplicant.rosterEntry =
                std::forward<F>(addRoster)(acceptApplicant.hholdID, myRequest_->wage);
            ++employCnt;
        }
        ledger_.readEmployResult({.employ = employCnt});
    }

    [[nodiscard]] auto publishResult() const -> RecruitResult {
        if (not isPosting()) return {.applicants = HeadCount{0.0}, .employ = HeadCount{0.0}};
        return ledger_.publishResult();
    }

    void reset() {
        if (not isPosting()) return;
        myRequest_.reset();
        ledger_.reset();
        offerApplicants_.clear();
    }

  private:
    [[nodiscard]] auto isPosting() const -> bool { return myRequest_.has_value(); }

    [[nodiscard]] static auto shouldPost(const RecruitPlan& plan) -> bool {
        return plan.offer > HeadCount{0.0};
    }

    [[nodiscard]] static auto sortApplicants(
        const HeadCount offer, const std::span<RefWrap<Entry>> entryBox
    ) -> std::span<RefWrap<Entry>> {
        const auto k{std::min(entryBox.size(), static_cast<std::size_t>(offer.value()))};
        const auto isOver{entryBox.size() > static_cast<std::size_t>(offer.value())};

        if (not isOver) return entryBox;

        std::ranges::nth_element(
            entryBox,
            entryBox.begin() + static_cast<int>(k),
            std::ranges::greater{},
            [](const RefWrap<Entry> entryRef) -> double { return entryRef.get().productPower; }
        );
        return entryBox;
    }

    std::optional<LaborRequest&> myRequest_{std::nullopt};
    Ledger                       ledger_;
    OfferApplicants              offerApplicants_;
};
}  // namespace abm::labor::demander::recruiter

namespace abm::labor::demander {
using Recruiter = recruiter::Recruiter;
}