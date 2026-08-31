#pragma once

#include <algorithm>
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
    OfferApplicants() noexcept = default;

    void add(Entry& entry) noexcept { applicants_.emplace_back(std::ref(entry)); }
    void clear() noexcept { applicants_.clear(); }
    auto offerAcceptedApplicants() noexcept -> std::ranges::view auto {
        return applicants_ | std::views::transform([](RefWrap<Entry> ref) noexcept -> Entry& {
                   return ref.get();
               }) |
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
    Ledger() = default;

    void makeNewPage(const HeadCount offerPlan) noexcept {
        ASSERT(offerPlan >= HeadCount{0.0});
        remainOffer_ = offerPlan;
    }

    [[nodiscard]] auto remainOffer() const noexcept -> HeadCount {
        ASSERT(remainOffer_ >= HeadCount{0.0});
        return remainOffer_;
    }

    void readOfferResult(const OfferResult& result) noexcept {
        ASSERT(result.offer >= HeadCount{0.0});
        ASSERT(result.applicants >= HeadCount{0.0});

        remainOffer_ -= result.offer;
        applicants_ += result.applicants;
    }

    void readEmployResult(const EmployResult add) noexcept {
        ASSERT(add.employ >= HeadCount{0.0});
        employ_ += add.employ;
    }

    [[nodiscard]] auto publishResult() const -> RecruitResult {
        return {.applicants = applicants_, .employ = employ_};
    }

    void reset() noexcept {
        ASSERT(remainOffer_ >= HeadCount{0.0});
        ASSERT(applicants_ >= HeadCount{0.0});
        ASSERT(employ_ >= HeadCount{0.0});

        remainOffer_ = HeadCount{0.0};
        applicants_  = HeadCount{0.0};
        employ_      = HeadCount{0.0};
    }

  private:
    HeadCount remainOffer_{0.0};
    HeadCount applicants_{0.0};
    HeadCount employ_{0.0};
};

class Recruiter final {
  public:
    Recruiter() noexcept = default;

    void post(const AgentID id, const RecruitPlan& plan, Market& laborMarket) noexcept {
        isActive_ = true;
        ASSERT(plan.wage >= Wage{0.0});
        if (not shouldPost(plan)) return;
        ledger_.makeNewPage(plan.offer);
        myRequest_ = laborMarket.request(id, plan.wage);
    }

    void offer() noexcept {
        if (not isPosting()) return;
        auto entries = myRequest_->entries();
        if (entries.empty()) return;
        if (HeadCount{entries.size()} < ledger_.remainOffer()) {
            offerAll();
            return;
        }

        auto applicants = sortApplicants(ledger_.remainOffer(), entries) |
                          std::views::take(ledger_.remainOffer().value());

        auto offerCnt = HeadCount{0.0};
        for (auto entryRef : applicants) {
            auto& entry = entryRef.get();
            entry.offer();
            offerApplicants_.add(entry);
            ++offerCnt;
        }

        ledger_.readOfferResult({.offer = offerCnt, .applicants = HeadCount{entries.size()}});
    }

    void registerMember(AddRosterFn auto&& addRoster) noexcept {
        if (not isPosting()) return;
        auto employCnt        = HeadCount{0.0};
        auto acceptApplicants = offerApplicants_.offerAcceptedApplicants();
        for (auto& acceptApplicant : acceptApplicants) {
            acceptApplicant.setRoster(addRoster(acceptApplicant.entrantId, myRequest_->wage));
            ++employCnt;
        }
        ledger_.readEmployResult({.employ = employCnt});
    }

    [[nodiscard]] auto publishResult() const noexcept -> std::optional<RecruitResult> {
        if (not isActive_) return std::nullopt;
        return ledger_.publishResult();
    }

    void reset() noexcept {
        myRequest_.reset();
        ledger_.reset();
        offerApplicants_.clear();
        isActive_ = false;
    }

  private:
    [[nodiscard]] auto isPosting() const noexcept -> bool { return myRequest_.has_value(); }

    void offerAll() noexcept {
        auto entries = myRequest_->entries();
        for (auto& entry : entries) entry.offer();
        const auto applicant = HeadCount{entries.size()};
        ledger_.readOfferResult({.offer = applicant, .applicants = applicant});
    }

    void offerPart() noexcept {
        std::vector<RefWrap<Entry>> {}
    }

    [[nodiscard]] auto packEntry() noexcept -> std::span<RefWrap<Entry>> {
        ASSERT(myRequest_);
        static thread_local auto box = std::vector<RefWrap<Entry>>{};
        box.clear();
        myRequest_->packEntry(box);
        return box;
    }

    [[nodiscard]] static auto shouldPost(const RecruitPlan& plan) noexcept -> bool {
        return plan.offer > HeadCount{0.0};
    }

    [[nodiscard]] static auto sortApplicants(
        const HeadCount offer, const std::span<RefWrap<Entry>> entryBox
    ) noexcept -> std::span<RefWrap<Entry>> {
        ASSERT(offer >= HeadCount{0.0});

        const auto k      = std::min(entryBox.size(), static_cast<std::size_t>(offer.value()));
        const auto isOver = entryBox.size() > static_cast<std::size_t>(offer.value());

        if (not isOver) return entryBox;

        std::ranges::nth_element(
            entryBox,
            entryBox.begin() + static_cast<int>(k),
            std::ranges::greater{},
            [](const RefWrap<Entry> entryRef) noexcept -> double {
                return entryRef.get().productPower;
            }
        );
        return entryBox;
    }

    std::optional<Request&> myRequest_{std::nullopt};
    Ledger                  ledger_;
    OfferApplicants         offerApplicants_;
    bool                    isActive_{false};
};
}  // namespace abm::labor::demander::recruiter

namespace abm::labor::demander {
using Recruiter = recruiter::Recruiter;
}