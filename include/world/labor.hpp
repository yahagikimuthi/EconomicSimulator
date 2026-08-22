#pragma once

#include <tbb/concurrent_vector.h>
#include <deque>
#include <functional>
#include <iterator>
#include <vector>

#include "core/assertion.hpp"
#include "core/forward.hpp"
#include "core/values/common.hpp"
#include "core/values/labor.hpp"
#include "util.hpp"
#include "world/common.hpp"

namespace abm {
struct CompanyBoard final {
    const AgentID                                firmId;
    const Market                                 firmType;
    std::deque<RosterEntry>                      roster;
    tbb::concurrent_vector<RefWrap<RosterEntry>> resignationBox;

    [[nodiscard]] constexpr CompanyBoard(const AgentID i, const Market type) noexcept
        : firmId{i}, firmType{type} {}
    void resign(RosterEntry& resignEntry) noexcept {
        resignationBox.emplace_back(std::ref(resignEntry));
    }
    constexpr auto addRoster(const AgentID id, const Wage wage, Workspace& workspace) noexcept
        -> RosterEntry&;
};

class RosterEntry final {
  public:
    [[nodiscard]] constexpr RosterEntry(
        const AgentID i, const Wage w, CompanyBoard& board, Workspace& space
    ) noexcept
        : hholdId{i}, wage{w}, companyBoard_{board}, workspace_{space} {
        ASSERT(w > Wage{0.0});
    }
    void addInput(const double productPower) noexcept;
    void resign() noexcept { companyBoard_.resign(*this); }

    [[nodiscard]] auto firmId() const -> AgentID { return companyBoard_.firmId; }
    [[nodiscard]] auto firmType() const -> Market { return companyBoard_.firmType; }

    const AgentID hholdId;
    const Wage    wage;
    bool          isOccupied{true};

  private:
    CompanyBoard& companyBoard_;
    Workspace&    workspace_;
};

constexpr auto CompanyBoard::addRoster(
    const AgentID id, const Wage wage, Workspace& workspace
) noexcept -> RosterEntry& {
    ASSERT(wage > Wage{0.0});
    return roster.emplace_back(id, wage, *this, workspace);
}

class LaborEntry final {
  public:
    [[nodiscard]] constexpr LaborEntry(
        const AgentID i, const double power, const LaborRequest& req
    ) noexcept
        : hholdID{i}, productPower{power}, request{req} {
        ASSERT(power > 0.0);
    }
    const AgentID hholdID;
    const double  productPower;

    void offer() noexcept { isOffer_ = true; }
    void accept() noexcept { isAccept_ = true; }
    void setRoster(RosterEntry& rosterEntry) noexcept { rosterEntry_ = rosterEntry; }

    [[nodiscard]] auto isOffer() const noexcept -> bool { return isOffer_; }
    [[nodiscard]] auto isAccept() const noexcept -> bool { return isAccept_; }
    [[nodiscard]] auto rosterEntry() const noexcept -> RosterEntry& {
        ASSERT(rosterEntry_);
        return *rosterEntry_;
    }

    const LaborRequest& request;

  private:
    std::optional<RosterEntry&> rosterEntry_{std::nullopt};
    bool                        isOffer_{false};
    bool                        isAccept_{false};
};

class LaborRequest final {
    using Entry = LaborEntry;

  public:
    [[nodiscard]] constexpr LaborRequest(const AgentID i, const Wage w) noexcept
        : firmID{i}, wage{w} {
        ASSERT(w > Wage{0.0});
    }
    [[nodiscard]] auto entry(const AgentID id, const double productPower) noexcept -> Entry& {
        ASSERT(productPower > 0.0);
        return *entryBox_.emplace_back(id, productPower, *this);
    }

    void packEntry(std::vector<RefWrap<Entry>>& out) noexcept {
        ASSERT(out.empty());
        for (Entry& e : entryBox_) out.emplace_back(std::ref(e));
    }

    const AgentID firmID;
    const Wage    wage;

  private:
    tbb::concurrent_vector<Entry> entryBox_;
};

class LaborMarket final {
    using Request = LaborRequest;

  public:
    [[nodiscard]] explicit constexpr LaborMarket(RandomGenerator& masterRng) noexcept
        : rng_{pcg32{masterRng.makeUint64(), masterRng.makeUint64()}} {}

    [[nodiscard]] auto request(const AgentID id, const Wage wage) noexcept -> Request& {
        ASSERT(wage > Wage{0.0});
        return *requestBox_.emplace_back(id, wage);
    }

    void pickRequest(std::vector<RefWrap<Request>>& out, const int n) noexcept {
        ASSERT(out.empty());
        if (n >= static_cast<int>(requestBox_.size())) {
            packAllRequest(out);
            return;
        }
        packPartRequest(out, n);
    }

    void clear() noexcept { requestBox_.clear(); }

  private:
    void packAllRequest(std::vector<RefWrap<Request>>& out) noexcept {
        for (Request& request : requestBox_) {
            out.emplace_back(std::ref(request));
        }
    }
    void packPartRequest(std::vector<RefWrap<Request>>& out, const int n) noexcept {
        rng_.sample(
            requestBox_ | std::views::transform([](Request& req) -> RefWrap<Request> {
                return std::ref(req);
            }),
            std::back_inserter(out),
            n
        );
    }

    tbb::concurrent_vector<LaborRequest> requestBox_;
    RandomGenerator                      rng_;
};
}  // namespace abm