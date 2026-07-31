#pragma once

#include <tbb/concurrent_vector.h>
#include <concepts>
#include <functional>
#include <pcg_random.hpp>
#include <ranges>
#include <vector>

#include "config.hpp"
#include "core/base.hpp"
#include "core/forward.hpp"
#include "helper.hpp"
#include "world/message.hpp"

namespace labor_supplier::internal {
inline void pickSample(
    tbb::concurrent_vector<world::LaborRequest>&              requestBox,
    std::vector<std::reference_wrapper<world::LaborRequest>>& sampleRequests,
    pcg32&                                                    rng,
    const int sampleCnt = config::labor_supplier::jobSampleCnt
) {
    const std::size_t k{std::min(static_cast<std::size_t>(sampleCnt), requestBox.size())};
    sampleRequests.clear();

    if (requestBox.size() <= static_cast<std::size_t>(sampleCnt)) {
        for (world::LaborRequest& request : requestBox)
            sampleRequests.emplace_back(std::ref(request));
        return;
    }

    std::ranges::sample(requestBox, std::back_inserter(sampleRequests), static_cast<int>(k), rng);
}

inline void sortSample(
    std::vector<std::reference_wrapper<world::LaborRequest>>& sortRequests,
    const int entryCnt = config::labor_supplier::jobEntryCnt
) {
    const std::size_t k{std::min(static_cast<std::size_t>(entryCnt), sortRequests.size())};
    std::ranges::partial_sort(
        sortRequests,
        sortRequests.begin() + static_cast<int>(k),
        std::ranges::greater{},
        [](const std::reference_wrapper<world::LaborRequest>& requestRef) -> double {
            return requestRef.get().wage.value();
        }
    );
}
}  // namespace labor_supplier::internal

namespace labor_supplier {
class JobHunter {
  public:
    JobHunter(pcg32& masterRng);

    template <typename F1, typename F2>
        requires requires(F1 isAligned, F2 makeEntrySheet, world::LaborRequest& request) {
            { isAligned(request) } -> std::same_as<bool>;
            {
                makeEntrySheet(request)
            } -> std::same_as<tbb::concurrent_vector<world::LaborEntry>::iterator>;
        }
    void entry(
        const F1                                     isAligned,
        const F2                                     makeEntrySheet,
        tbb::concurrent_vector<world::LaborRequest>& requestBox,
        const int                                    entryCnt = config::labor_supplier::jobEntryCnt
    ) PRE(entryCnt > 0) {
        using Request = world::LaborRequest;
        isPosting_    = true;
        static thread_local std::vector<std::reference_wrapper<Request>> sampleRequests;
        internal::pickSample(requestBox, sampleRequests, rng_);
        internal::sortSample(sampleRequests);
        std::ranges::view auto alignedRequests{
            sampleRequests |
            std::views::filter([&](const std::reference_wrapper<Request> req) -> bool {
                return isAligned(req.get());
            }) |
            std::views::take(entryCnt)
        };
        for (const auto requestRef : alignedRequests) {
            auto& request = requestRef.get();
            auto  it{makeEntrySheet(request)};
            myEntries_.emplace_back(&*it);
        }
    }
    void accept() {
        if (not isPosting_) return;
        for (SafePtr<world::LaborEntry> myEntry : myEntries_) {
            if (not myEntry->isOffer) continue;
            myEntry->isAccept = true;
            acceptedEntry_    = myEntry;
        }
    }
    void endStep() { myEntries_.clear(), acceptedEntry_ = nullptr, isPosting_ = false; }
    auto acceptedEntry() const -> SafePtr<world::LaborEntry> { return acceptedEntry_; }

  private:
    mutable pcg32                           rng_;
    std::vector<SafePtr<world::LaborEntry>> myEntries_;
    SafePtr<world::LaborEntry>              acceptedEntry_{nullptr};
    bool                                    isPosting_{false};
};

class Employment {
  public:
    Employment(pcg32& masterRng);
    auto isEmployed() const -> bool { return rosterEntry_.hasValue(); }
    void startWorking(const SafePtr<world::RosterEntry> rosterEntry) {
        resign();
        rosterEntry_ = rosterEntry;
    }
    auto contractFirmId() const -> AgentID {
        return isEmployed() ? rosterEntry_->companyBoard.firmId : AgentID{-1};
    }
    auto wage() const -> Wage POST(wage : wage >= Wage{0.0}) {
        return isEmployed() ? rosterEntry_->wage : Wage{0.0};
    }
    void executeProduct() {
        if (not isEmployed()) return;
        auto& workspace = rosterEntry_->workspace;
        workspace.totalLaborInput += workspace.firmProductPower * productPower_;
    }
    auto productPower() const -> double { return productPower_; }
    void updateStatus() {
        if (not isEmployed()) return;
        if (not rosterEntry_->isOccupied) rosterEntry_ = nullptr;
    }

  private:
    void resign() {
        if (not isEmployed()) return;
        rosterEntry_->companyBoard.resignationBox.emplace_back(rosterEntry_);
    }

    SafePtr<world::RosterEntry> rosterEntry_{nullptr};
    const double                productPower_;
};

class LaborSupplier {
  public:
    LaborSupplier(pcg32& masterRng);
    void entry(const AgentID id, tbb::concurrent_vector<world::LaborRequest>& requestBox) {
        updateRosterEntry();
        if (not shouldSearchJob()) return;
        if (requestBox.empty()) return;

        using Request = world::LaborRequest;
        const auto isAligned{[&](const Request& req) -> bool {
            if (req.firmID == employment_.contractFirmId()) return false;
            if (req.wage < employment_.wage()) return false;
            return true;
        }};
        const auto makeEntrySheet{[&](Request& req) -> auto {
            return req.entryBox.emplace_back(id, employment_.productPower(), req);
        }};
        jobHunter_.entry(isAligned, makeEntrySheet, requestBox);
    }
    void accept() { jobHunter_.accept(); }
    void recordRosterEntry() {
        const SafePtr<world::LaborEntry> acceptedEntry{jobHunter_.acceptedEntry()};
        if (not acceptedEntry) return;
        employment_.startWorking(acceptedEntry->rosterEntry);
    }
    void endStep(world::CensusDropBox& dropBox) {
        dropBox.wages.emplace_back(wage().value());
        jobHunter_.endStep();
    }
    void product() { employment_.executeProduct(); }
    auto wage() const -> Wage POST(wage : wage >= Wage{0.0}) { return employment_.wage(); }
    auto isEligibleRequest(const world::LaborRequest& request) const -> bool {
        if (request.firmID == employment_.contractFirmId()) return false;
        if (request.wage < employment_.wage()) return false;
        return true;
    }

  private:
    void updateRosterEntry() { employment_.updateStatus(); }
    auto shouldSearchJob() const -> bool {
        if (not employment_.isEmployed()) return true;
        return helper::rand(rng_) < jobSearchThreshold_;
    }

    mutable pcg32 rng_;
    JobHunter     jobHunter_;
    Employment    employment_;
    const double  jobSearchThreshold_;
};
}  // namespace labor_supplier