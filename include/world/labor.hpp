#pragma once

#include <tbb/concurrent_vector.h>
#include <deque>
#include <functional>
#include <iterator>
#include <optional>
#include <ranges>
#include <vector>

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
    [[nodiscard]] RosterEntry(
        const AgentID i, const Wage w, CompanyBoard& board, base_goods::Workspace& space
    ) noexcept
        : hholdId{i}, wage{w}, companyBoard_{board}, workspace_{space} {
        ASSERT(w > Wage{0.0});
        ASSERT(i != companyBoard_.firmId);
    }
    void addInput(const double productPower) noexcept;
    void resign() noexcept { companyBoard_.resign(*this); }
    void disable() noexcept { isOccupied_ = false; }

    [[nodiscard]] auto firmId() const noexcept -> AgentID { return companyBoard_.firmId; }
    [[nodiscard]] auto firmType() const noexcept -> EMarket { return companyBoard_.firmType; }
    [[nodiscard]] auto isOccupied() const noexcept -> bool { return isOccupied_; }

    const AgentID hholdId;
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

class LaborRequest;
class LaborEntry final {
    using Request = LaborRequest;

  public:
    [[nodiscard]] LaborEntry(const AgentID i, const double power, const Request& req) noexcept
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

    const Request& request;

  private:
    std::optional<RosterEntry&> rosterEntry_{std::nullopt};
    bool                        isOffer_{false};
    bool                        isAccept_{false};
};

class LaborRequest final {
    using Entry = LaborEntry;

  public:
    [[nodiscard]] LaborRequest(const AgentID i, const Wage w) noexcept : firmID{i}, wage{w} {
        ASSERT(w > Wage{0.0});
    }
    [[nodiscard]] auto entry(const AgentID id, const double productPower) noexcept -> Entry& {
        ASSERT(productPower > 0.0);
        ASSERT(id != firmID);
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
    [[nodiscard]] LaborMarket() noexcept = default;

    [[nodiscard]] auto request(const AgentID id, const Wage wage) noexcept -> Request& {
        ASSERT(wage > Wage{0.0});
        return *requestBox_.emplace_back(id, wage);
    }

    void pickRequest(
        std::vector<RefWrap<Request>>& out, const int n, RandomGenerator& rng
    ) noexcept {
        ASSERT(out.empty());
        if (n >= static_cast<int>(requestBox_.size())) {
            packAllRequest(out);
            return;
        }
        packPartRequest(out, n, rng);
    }

    void clear() noexcept { requestBox_.clear(); }

  private:
    void packAllRequest(std::vector<RefWrap<Request>>& out) noexcept {
        for (Request& request : requestBox_) {
            out.emplace_back(std::ref(request));
        }
    }
    void packPartRequest(
        std::vector<RefWrap<Request>>& out, const int n, RandomGenerator& rng
    ) noexcept {
        rng.sample(
            requestBox_ | std::views::transform([](Request& req) noexcept -> RefWrap<Request> {
                return std::ref(req);
            }),
            std::back_inserter(out),
            n
        );
    }

    tbb::concurrent_vector<LaborRequest> requestBox_;
};
}  // namespace abm::labor

namespace abm {
using LaborMarket = labor::LaborMarket;
}