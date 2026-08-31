#pragma once

#include <tbb/concurrent_vector.h>
#include <cstddef>
#include <deque>
#include <functional>
#include <inplace_vector>
#include <iterator>
#include <optional>
#include <ranges>

#include "others/util.hpp"
#include "values/common.hpp"
#include "values/labor.hpp"
#include "world/common.hpp"

namespace abm::base_goods {
class Workspace;
}

namespace abm::labor {
class RosterEntry;
struct CompanyBoard final {
    const AgentID                                firmId;
    const EMarket                                firmType;
    std::deque<RosterEntry>                      roster;
    tbb::concurrent_vector<RefWrap<RosterEntry>> resignationBox;

    [[nodiscard]] CompanyBoard(const AgentID i, const EMarket type) noexcept
        : firmId{i}, firmType{type} {}
    void resign(RosterEntry& resignEntry) noexcept {
        resignationBox.emplace_back(std::ref(resignEntry));
    }
    auto addRoster(const AgentID id, const Wage wage, base_goods::Workspace& workspace) noexcept
        -> RosterEntry&;
};

class RosterEntry final {
  public:
    RosterEntry(
        const AgentID i, const Wage w, CompanyBoard& board, base_goods::Workspace& space
    ) noexcept
        : employeeId{i}, wage{w}, companyBoard_{board}, workspace_{space} {
        ASSERT(w > Wage{0.0});
        ASSERT(i != companyBoard_.firmId);
    }
    void addInput(const double productPower) noexcept;
    void resign() noexcept { companyBoard_.resign(*this); }
    void disable() noexcept { isOccupied_ = false; }

    [[nodiscard]] auto firmId() const noexcept -> AgentID { return companyBoard_.firmId; }
    [[nodiscard]] auto firmType() const noexcept -> EMarket { return companyBoard_.firmType; }
    [[nodiscard]] auto isOccupied() const noexcept -> bool { return isOccupied_; }

    const AgentID employeeId;
    const Wage    wage;

  private:
    CompanyBoard&          companyBoard_;
    base_goods::Workspace& workspace_;
    bool                   isOccupied_{true};
};

auto CompanyBoard::addRoster(
    const AgentID id, const Wage wage, base_goods::Workspace& workspace
) noexcept -> RosterEntry& {
    ASSERT(wage > Wage{0.0});
    return roster.emplace_back(id, wage, *this, workspace);
}

class Request;
class Entry final {
  public:
    Entry(const AgentID i, const double power, const Request& req) noexcept
        : requestorId{i}, productPower{power}, request{req} {
        ASSERT(power > 0.0);
    }
    const AgentID requestorId;
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

    const Request& request;

  private:
    std::optional<RosterEntry&> rosterEntry_{std::nullopt};
    bool                        isOffer_{false};
    bool                        isAccept_{false};
};

class Request final {
  public:
    Request(const AgentID i, const Wage w) noexcept : firmID{i}, wage{w} { ASSERT(w > Wage{0.0}); }
    [[nodiscard]] auto entry(const AgentID id, const double productPower) noexcept -> Entry& {
        ASSERT(productPower > 0.0);
        ASSERT(id != firmID);
        return *entries_.emplace_back(id, productPower, *this);
    }

    [[nodiscard]] auto entries() noexcept -> auto { return std::ranges::subrange{entries_}; }

    const AgentID firmID;
    const Wage    wage;

  private:
    tbb::concurrent_vector<Entry> entries_;
};

class Market final {
  public:
    Market() noexcept = default;

    [[nodiscard]] auto request(const AgentID id, const Wage wage) noexcept -> Request& {
        ASSERT(wage.isPositive());
        return *requests_.emplace_back(id, wage);
    }

    template <std::size_t N>
    void pickRequest(std::inplace_vector<RefWrap<Request>, N>& out, RandomGenerator& rng) noexcept {
        ASSERT(out.empty());
        if (out.max_size() >= requests_.size()) {
            packAllRequest(out);
        }
        packPartRequest(out, rng);
    }

    void clear() noexcept { requests_.clear(); }

  private:
    void packAllRequest(std::inplace_vector<RefWrap<Request>, 1UZ>& out) {
        for (auto& req : requests_) out.unchecked_emplace_back(std::ref(req));
    }

    template <std::size_t N>
    void packPartRequest(
        std::inplace_vector<RefWrap<Request>, N>& out, RandomGenerator& rng
    ) noexcept {
        rng.sample(
            requests_ | std::views::transform([](Request& req) noexcept -> RefWrap<Request> {
                return std::ref(req);
            }),
            std::back_inserter(out),
            out.max_size()
        );
    }

    tbb::concurrent_vector<Request> requests_;
};
}  // namespace abm::labor

namespace abm {
using LaborMarket = labor::Market;
}