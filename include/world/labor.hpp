#pragma once

#include <tbb/concurrent_vector.h>
#include <deque>
#include <functional>

#include "core/values/common.hpp"
#include "core/values/labor.hpp"
#include "world/common.hpp"

namespace abm {

class RosterEntry;
class Workspace;
struct [[nodiscard]] CompanyBoard {
    const AgentID                                               firmId;
    const Market                                                firmType;
    std::deque<RosterEntry>                                     roster;
    tbb::concurrent_vector<std::reference_wrapper<RosterEntry>> resignationBox;

    CompanyBoard(const AgentID Id, const Market FirmType) : firmId{Id}, firmType{FirmType} {}
    void resign(RosterEntry& resignEntry) { resignationBox.emplace_back(std::ref(resignEntry)); }
    auto addRoster(const AgentID id, const Wage wage, Workspace& workspace) -> RosterEntry&;
};

class [[nodiscard]] RosterEntry {
  public:
    RosterEntry(const AgentID Id, const Wage Wage, CompanyBoard& CompanyBoard, Workspace& Workspace)
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

    LaborEntry(const AgentID Id, const double ProductPower, const LaborRequest& Request)
        : hholdID{Id}, productPower{ProductPower}, request{Request} {}
};

struct [[nodiscard]] LaborRequest {
    template <typename T>
    using RefWrap = std::reference_wrapper<T>;

    LaborRequest(const AgentID Id, const Wage Wage) : firmID{Id}, wage{Wage} {}
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

class [[nodiscard]] LaborMarket {
  public:
    using RequestBoxT = std::ranges::subrange<
        std::ranges::iterator_t<tbb::detail::d1::concurrent_vector<
            abm::LaborRequest,
            tbb::detail::d1::cache_aligned_allocator<abm::LaborRequest>>&>,
        std::ranges::sentinel_t<tbb::detail::d1::concurrent_vector<
            abm::LaborRequest,
            tbb::detail::d1::cache_aligned_allocator<abm::LaborRequest>>&>,
        (std::ranges::sized_range<tbb::detail::d1::concurrent_vector<abm::LaborRequest>&> ||
         std::sized_sentinel_for<
             std::ranges::sentinel_t<tbb::detail::d1::concurrent_vector<
                 abm::LaborRequest,
                 tbb::detail::d1::cache_aligned_allocator<abm::LaborRequest>>&>,
             std::ranges::iterator_t<tbb::detail::d1::concurrent_vector<
                 abm::LaborRequest,
                 tbb::detail::d1::cache_aligned_allocator<abm::LaborRequest>>&>>)
            ? std::ranges::subrange_kind::sized
            : std::ranges::subrange_kind::unsized>;

    LaborMarket() = default;
    auto requestBox() -> RequestBoxT { return std::ranges::subrange{requestBox_}; }
    auto request(const AgentID id, const Wage wage) -> LaborRequest& {
        return *requestBox_.emplace_back(id, wage);
    }
    void clear() { requestBox_.clear(); }

  private:
    tbb::concurrent_vector<LaborRequest> requestBox_;
};
}  // namespace abm