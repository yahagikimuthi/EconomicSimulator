#pragma once

#include <tbb/concurrent_vector.h>
#include <deque>
#include <functional>
#include <span>
#include <vector>

#include "core/values/common.hpp"
#include "core/values/labor.hpp"
#include "util.hpp"
#include "world/common.hpp"

namespace abm {

class RosterEntry;
class Workspace;
struct CompanyBoard {
    const AgentID                                               firmId;
    const Market                                                firmType;
    std::deque<RosterEntry>                                     roster;
    tbb::concurrent_vector<std::reference_wrapper<RosterEntry>> resignationBox;

    [[nodiscard]] CompanyBoard(const AgentID Id, const Market FirmType) noexcept
        : firmId{Id}, firmType{FirmType} {}
    void resign(RosterEntry& resignEntry) { resignationBox.emplace_back(std::ref(resignEntry)); }
    auto addRoster(const AgentID id, const Wage wage, Workspace& workspace) -> RosterEntry&;
};

class RosterEntry {
  public:
    [[nodiscard]] RosterEntry(
        const AgentID Id, const Wage Wage, CompanyBoard& CompanyBoard, Workspace& Workspace
    )
        : hholdId{Id}, wage{Wage}, companyBoard{CompanyBoard}, workspace{Workspace} {}
    void addInput(const double productPower);
    void resign() { companyBoard.resign(*this); }
    auto firmId() const -> AgentID { return companyBoard.firmId; }
    auto firmType() const -> Market { return companyBoard.firmType; }

    const AgentID hholdId;
    const Wage    wage;
    bool          isOccupied{true};

  private:
    CompanyBoard& companyBoard;
    Workspace&    workspace;
};

inline auto CompanyBoard::addRoster(const AgentID id, const Wage wage, Workspace& workspace)
    -> RosterEntry& {
    return roster.emplace_back(id, wage, *this, workspace);
}

struct LaborRequest;
struct LaborEntry {
    const AgentID hholdID;
    const double  productPower;

    bool isOffer{false};
    bool isAccept{false};

    std::optional<RosterEntry&> rosterEntry{std::nullopt};
    const LaborRequest&         request;

    [[nodiscard]] LaborEntry(
        const AgentID Id, const double ProductPower, const LaborRequest& Request
    )
        : hholdID{Id}, productPower{ProductPower}, request{Request} {}
};

struct LaborRequest {
    [[nodiscard]] LaborRequest(const AgentID Id, const Wage Wage) : firmID{Id}, wage{Wage} {}
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

class LaborMarket {
  public:
    [[nodiscard]] LaborMarket() noexcept = default;
    auto requestBox() -> std::span<RefWrap<LaborRequest>> { return references_; }
    auto request(const AgentID id, const Wage wage) -> LaborRequest& {
        auto it = requestBox_.emplace_back(id, wage);
        references_.emplace_back(std::ref(*it));
        return *it;
    }
    void clear() { requestBox_.clear(); }

  private:
    tbb::concurrent_vector<LaborRequest> requestBox_;
    std::vector<RefWrap<LaborRequest>>   references_;
};
}  // namespace abm