#pragma once

#include <tbb/concurrent_vector.h>
#include <deque>
#include <functional>
#include <iterator>
#include <span>
#include <vector>

#include "core/forward.hpp"
#include "core/values/common.hpp"
#include "core/values/labor.hpp"
#include "util.hpp"
#include "world/common.hpp"

namespace abm {
struct CompanyBoard final {
    const AgentID                                               firmId;
    const Market                                                firmType;
    std::deque<RosterEntry>                                     roster;
    tbb::concurrent_vector<std::reference_wrapper<RosterEntry>> resignationBox;

    [[nodiscard]] CompanyBoard(const AgentID i, const Market type) noexcept
        : firmId{i}, firmType{type} {}
    void resign(RosterEntry& resignEntry) { resignationBox.emplace_back(std::ref(resignEntry)); }
    auto addRoster(const AgentID id, const Wage wage, Workspace& workspace) -> RosterEntry&;
};

class RosterEntry final {
  public:
    [[nodiscard]] RosterEntry(const AgentID i, const Wage w, CompanyBoard& board, Workspace& space)
        : hholdId{i}, wage{w}, companyBoard_{board}, workspace_{space} {}
    void addInput(const double productPower) noexcept;
    void resign() { companyBoard_.resign(*this); }

    [[nodiscard]] auto firmId() const -> AgentID { return companyBoard_.firmId; }
    [[nodiscard]] auto firmType() const -> Market { return companyBoard_.firmType; }

    const AgentID hholdId;
    const Wage    wage;
    bool          isOccupied{true};

  private:
    CompanyBoard& companyBoard_;
    Workspace&    workspace_;
};

inline auto CompanyBoard::addRoster(const AgentID id, const Wage wage, Workspace& workspace)
    -> RosterEntry& {
    return roster.emplace_back(id, wage, *this, workspace);
}

struct LaborEntry final {
    const AgentID hholdID;
    const double  productPower;

    bool isOffer{false};
    bool isAccept{false};

    std::optional<RosterEntry&> rosterEntry{std::nullopt};
    const LaborRequest&         request;

    [[nodiscard]] LaborEntry(const AgentID i, const double power, const LaborRequest& req)
        : hholdID{i}, productPower{power}, request{req} {}
};

struct LaborRequest final {
    [[nodiscard]] LaborRequest(const AgentID i, const Wage w) : firmID{i}, wage{w} {}
    auto entry(const AgentID id, const double productPower) -> LaborEntry& {
        auto it = entryBox_.emplace_back(id, productPower, *this);
        references_.emplace_back(std::ref(*it));
        return *it;
    }

    auto entryBox() -> std::span<RefWrap<LaborEntry>> { return references_; }

    const AgentID firmID;
    const Wage    wage;

  private:
    tbb::concurrent_vector<LaborEntry> entryBox_;
    std::vector<RefWrap<LaborEntry>>   references_;
};

class LaborMarket final {
    using Request = LaborRequest;

  public:
    [[nodiscard]] explicit LaborMarket(RandomGenerator& masterRng) noexcept
        : rng_{pcg32{masterRng.makeUint64(), masterRng.makeUint64()}} {}
    [[nodiscard]] auto request(const AgentID id, const Wage wage) -> Request& {
        auto it = requestBox_.emplace_back(id, wage);
        references_.emplace_back(std::ref(*it));
        return *it;
    }
    void pickRequest(std::vector<RefWrap<Request>>& out, const int n) {
        ASSERT(out.empty());
        ASSERT(requestBox_.size() == references_.size());
        if (n > static_cast<int>(requestBox_.size())) {
            packAllRequest(out);
            return;
        }
    }

    void clear() { requestBox_.clear(), references_.clear(); }

  private:
    void packAllRequest(std::vector<RefWrap<Request>>& out) {
        for (RefWrap<Request> request : references_) {
            out.emplace_back(request);
        }
    }
    void packPartRequest(std::vector<RefWrap<Request>>& out, const int n) {
        rng_.sample(
            requestBox_ | std::views::transform([](Request& req) -> RefWrap<Request> {
                return std::ref(req);
            }),
            std::back_inserter(out),
            n
        );
    }

    tbb::concurrent_vector<LaborRequest> requestBox_;
    std::vector<RefWrap<LaborRequest>>   references_;
    RandomGenerator                      rng_;
};
}  // namespace abm