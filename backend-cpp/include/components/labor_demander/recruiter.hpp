#pragma once

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <functional>
#include <optional>
#include <ranges>
#include <vector>

#include "components/labor_demander/common.hpp"
#include "others/util.hpp"
#include "values/common.hpp"
#include "values/labor.hpp"

namespace abm::labor::demander::recruiter {
class OfferApplicants final {
  public:
    explicit OfferApplicants() noexcept = default;

    void add(Entry& entry) noexcept { applicants_.emplace_back(std::ref(entry)); }
    void clear() noexcept { applicants_.clear(); }
    auto offerAcceptedApplicants() noexcept -> auto {
        return applicants_ |
               std::views::transform([](Ref<Entry> ref) noexcept -> Entry& { return ref.get(); }) |
               std::views::filter(&Entry::isAccept);
    }

  private:
    std::vector<Ref<Entry>> applicants_;
};

class Ledger final {
  public:
    explicit Ledger() noexcept = default;

    void makeNewPage(const HeadCount offerPlan) noexcept {
        assert(offerPlan.isZeroOrMore());
        offerPlan_ = offerPlan;
    }

    [[nodiscard]] auto offerPlan() const noexcept -> HeadCount {
        assert(offerPlan_.isZeroOrMore());
        return offerPlan_;
    }

    void addApplicantsCnt(const HeadCount applicant) noexcept {
        assert(applicant.isZeroOrMore());
        assert(applicants_.isZero());
        applicants_ += applicant;
    }

    void addEmployCnt(const HeadCount employ) noexcept {
        assert(employ.isZeroOrMore());
        assert(employ_.isZero());
        employ_ += employ;
    }

    [[nodiscard]] auto publishResult() const noexcept -> RecruitResult {
        assert(applicants_.isZeroOrMore());
        assert(employ_.isZeroOrMore());
        return {.applicants = applicants_, .employ = employ_};
    }

    void reset() noexcept {
        assert(offerPlan_.isZeroOrMore());
        assert(applicants_.isZeroOrMore());
        assert(employ_.isZeroOrMore());

        offerPlan_ = applicants_ = employ_ = HeadCount{0.0};
    }

  private:
    HeadCount offerPlan_{0.0};
    HeadCount applicants_{0.0};
    HeadCount employ_{0.0};
};

class Recruiter final {
  public:
    explicit Recruiter() noexcept = default;

    void post(const AgentID id, const RecruitPlan& plan, Market& laborMarket) noexcept {
        assert(plan.wage.isZeroOrMore());
        if (not shouldPost(plan)) return;
        ledger_.makeNewPage(plan.offer);
        myRequest_ = laborMarket.request(id, plan.wage);
    }

    void offer() noexcept {
        if (not isPosting()) return;
        auto entries = myRequest_->entries();
        if (entries.empty()) return;
        HeadCount{entries.size()} < ledger_.offerPlan() ? offerAll() : offerPart();
    }

    [[nodiscard]] auto endRecruiting(AddRosterFn auto&& addRoster) noexcept -> RecruitResult {
        if (not isPosting()) return {.applicants = HeadCount{0.0}, .employ = HeadCount{0.0}};
        auto employCnt        = HeadCount{0.0};
        auto acceptApplicants = offerApplicants_.offerAcceptedApplicants();
        for (auto& acceptApplicant : acceptApplicants) {
            acceptApplicant.setRoster(addRoster(acceptApplicant.entrantId, myRequest_->wage));
            ++employCnt;
        }
        ledger_.addEmployCnt(employCnt);
        return ledger_.publishResult();
    }

    void reset() noexcept {
        myRequest_.reset();
        ledger_.reset();
        offerApplicants_.clear();
    }

  private:
    [[nodiscard]] auto isPosting() const noexcept -> bool { return myRequest_.has_value(); }

    void offerAll() noexcept {
        auto entries = myRequest_->entries();
        for (auto& entry : entries) {
            entry.offer();
            offerApplicants_.add(entry);
        }
        const auto applicant = HeadCount{entries.size()};
        ledger_.addApplicantsCnt(applicant);
    }

    void offerPart() noexcept {
        auto entries         = packEntry();
        auto offerApplicants = sortApplicants(ledger_.offerPlan(), entries) |
                               std::views::take(ledger_.offerPlan().value());

        auto offerCnt = HeadCount{0.0};
        for (auto entryRef : offerApplicants) {
            auto& entry = entryRef.get();
            entry.offer();
            offerApplicants_.add(entry);
            ++offerCnt;
        }
        ledger_.addApplicantsCnt(HeadCount{entries.size()});
    }

    [[nodiscard]] auto packEntry() noexcept -> std::span<Ref<Entry>> {
        assert(myRequest_);
        static thread_local auto refs = std::vector<Ref<Entry>>{};
        refs.clear();
        auto entries = myRequest_->entries();
        refs.reserve(entries.size());
        for (auto& entry : entries) refs.emplace_back(std::ref(entry));
        return refs;
    }

    [[nodiscard]] static auto shouldPost(const RecruitPlan& plan) noexcept -> bool {
        return plan.offer.isPositive() and plan.employ.isPositive() and plan.wage.isPositive();
    }

    [[nodiscard]] static auto sortApplicants(
        const HeadCount offer, const std::span<Ref<Entry>> entryBox
    ) noexcept -> std::span<Ref<Entry>> {
        assert(offer.isZeroOrMore());

        const auto k      = std::min(entryBox.size(), static_cast<std::size_t>(offer.value()));
        const auto isOver = entryBox.size() > static_cast<std::size_t>(offer.value());

        if (not isOver) return entryBox;

        std::ranges::nth_element(
            entryBox,
            entryBox.begin() + static_cast<int>(k),
            std::ranges::greater{},
            [](const Ref<Entry> entryRef) noexcept -> double { return entryRef.get().productPower; }
        );
        return entryBox;
    }

    std::optional<Request&> myRequest_{std::nullopt};
    Ledger                  ledger_;
    OfferApplicants         offerApplicants_;
};
}  // namespace abm::labor::demander::recruiter

namespace abm::labor::demander {
using Recruiter = recruiter::Recruiter;
}