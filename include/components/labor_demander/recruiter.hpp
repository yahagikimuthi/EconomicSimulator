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

class [[nodiscard]] Offerer {
    template <typename T>
    using RefWrap = std::reference_wrapper<T>;

  public:
    Offerer(const HeadCount lastApplicants) : lastApplicantNum_{lastApplicants} {}

    void startWorking(const HeadCount offerPlan) { ledgerManager_.makeNewPage(offerPlan); }

    void offer(LaborRequest& request) {
        if (request.entryBox.empty()) return;

        std::ranges::view auto applicants{
            sortApplicants(ledgerManager_.remainOfferNum(), request.entryBox) |
            std::views::take(ledgerManager_.remainOfferNum().value())
        };

        for (auto&& entry : applicants) {
            entry.isOffer = true;
            offerApplicants_.add(entry);
            ledgerManager_.decrementRemainOfferNum();
        }
        ledgerManager_.addApplicant(HeadCount{request.entryBox.size()});
    }

    auto offerAcceptedApplicants() -> std::ranges::view auto {
        return offerApplicants_.offerAcceptedApplicants();
    }

    void reset() {
        lastApplicantNum_ = ledgerManager_.applicantNum();
        ledgerManager_.reset();
        offerApplicants_.clear();
    }

    auto lastApplicantNum() const -> HeadCount { return lastApplicantNum_; }

  private:
    class [[nodiscard]] {
      public:
        void makeNewPage(const HeadCount offerPlan) { offerPlan_ = offerPlan; }
        auto remainOfferNum() const -> HeadCount { return remainOfferNum_; }
        void decrementRemainOfferNum() { --remainOfferNum_; }
        void addApplicant(HeadCount add) { applicantNum_ += add; }
        auto applicantNum() const -> HeadCount { return applicantNum_; }
        void reset() {
            offerPlan_      = HeadCount{0.0};
            remainOfferNum_ = HeadCount{0.0};
            applicantNum_   = HeadCount{0.0};
        }

      private:
        HeadCount offerPlan_{0.0};
        HeadCount remainOfferNum_{0.0};
        HeadCount applicantNum_{0.0};
    } ledgerManager_;

    OfferApplicants offerApplicants_;
    HeadCount       lastApplicantNum_;
};

class [[nodiscard]] Recruiter {
  public:
    Recruiter(Offerer&& offerer) : offerer_{std::move(offerer)} {}
    void post(
        const AgentID id, const PostingInfo info, tbb::concurrent_vector<LaborRequest>& requestBox
    ) {
        if (info.offerNum == HeadCount{0.0}) return;
        isPosting_ = true;
        offerer_.startWorking(info.offerNum);
        auto it{requestBox.emplace_back(id, info.wage)};
        myRequest_ = *it;
    }

    void offer() {
        if (not isPosting_) return;
        offerer_.offer(*myRequest_);
    }

    void registerMember(AddRosterFn auto&& addRoster) {
        if (not isPosting_) return;

        HeadCount              employCnt{0.0};
        std::ranges::view auto acceptApplicants{offerer_.offerAcceptedApplicants()};
        for (LaborEntry& acceptApplicant : acceptApplicants) {
            acceptApplicant.rosterEntry = addRoster(acceptApplicant.hholdID, myRequest_->wage);
            ++employCnt;
        }
        ledgerManager_.addEmploying(employCnt);
    }

    void endStep() {
        myRequest_.reset();
        ledgerManager_.reset();
        offerer_.reset();
        isPosting_ = false;
    }

    auto lastApplicantNum() const -> HeadCount { return offerer_.lastApplicantNum(); }
    auto employing() const -> HeadCount { return ledgerManager_.employing(); }

  private:
    template <typename U>
    using RefWrapper = std::reference_wrapper<U>;

    std::optional<LaborRequest&> myRequest_{std::nullopt};
    class {
      public:
        void addEmploying(HeadCount add) { employing_ += add; }
        void reset() { employing_ = HeadCount{0.0}; }
        auto employing() const -> HeadCount { return employing_; }

      private:
        HeadCount employing_{0.0};
    } ledgerManager_;

    Offerer offerer_;
    bool    isPosting_{false};
};
}  // namespace abm::labor::demander