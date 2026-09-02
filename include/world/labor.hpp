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
#include "values/date.hpp"
#include "values/labor.hpp"
#include "world/base_goods.hpp"

namespace abm::labor {
class RosterEntry;
struct CompanyBoard final {
    CompanyBoard(const AgentID Id) noexcept : firmId{Id} {}
    void resign(RosterEntry& resignEntry) noexcept {
        resignationBox.emplace_back(std::ref(resignEntry));
    }
    [[nodiscard]] auto addRoster(
        const AgentID id, const Wage wage, base_goods::Workspace& workspace
    ) noexcept -> RosterEntry&;

    const AgentID                                firmId;
    std::deque<RosterEntry>                      roster;
    tbb::concurrent_vector<RefWrap<RosterEntry>> resignationBox;
};

class RosterEntry final {
  public:
    RosterEntry(
        const AgentID Id, const Wage Wage, CompanyBoard& board, base_goods::Workspace& space
    ) noexcept
        : employeeId{Id}, wage{Wage}, companyBoard_{board}, workspace_{space} {
        ASSERT(Wage.isPositive());
        ASSERT(Id != companyBoard_.firmId);
    }
    // std::deque<RosterEntry>に対しstd::swapを施すと
    // entrantが持つ参照が無意味となる。
    // よって、代入演算子を明示的削除する。
    RosterEntry(const RosterEntry&) noexcept                    = default;
    auto operator=(const RosterEntry&) noexcept -> RosterEntry& = delete;
    RosterEntry(RosterEntry&&)                                  = delete;
    auto operator=(RosterEntry&&) noexcept -> RosterEntry&      = delete;
    ~RosterEntry() noexcept                                     = default;

    void addInput(const double productPower) noexcept { workspace_.addInput(productPower); }
    void resign() noexcept { companyBoard_.resign(*this); }
    void disable() noexcept { isOccupied_ = false; }

    [[nodiscard]] auto firmId() const noexcept -> AgentID { return companyBoard_.firmId; }
    [[nodiscard]] auto isOccupied() const noexcept -> bool { return isOccupied_; }

    const AgentID employeeId;
    const Wage    wage;

  private:
    CompanyBoard&          companyBoard_;
    base_goods::Workspace& workspace_;
    bool                   isOccupied_{true};
};

inline auto CompanyBoard::addRoster(
    const AgentID id, const Wage wage, base_goods::Workspace& workspace
) noexcept -> RosterEntry& {
    ASSERT(wage.isPositive());
    return roster.emplace_back(id, wage, *this, workspace);
}

class Request;
class Entry final {
  public:
    Entry(const AgentID Id, const double power, const Request& req) noexcept
        : entrantId{Id}, productPower{power}, request{req} {
        ASSERT(power > 0.0);
    }
    // Request::entries() -> std::ranges::subrangeを呼び、それに対しstd::sortを施すと
    // entrantが持つ参照が無効化してしまう。
    // std::sortはstd::swapを内部で行い、そのコンセプトはコピー及びムーブ構築が可能であること。
    // よって、代入演算子を明示的削除する。
    Entry(const Entry&)                             = default;
    auto operator=(const Entry&) noexcept -> Entry& = delete;
    Entry(Entry&&)                                  = delete;
    auto operator=(Entry&&) noexcept -> Entry&      = delete;
    ~Entry() noexcept                               = default;

    const AgentID entrantId;
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
    Request(const AgentID Id, const Wage Wage) noexcept : firmID{Id}, wage{Wage} {
        ASSERT(Wage.isPositive());
    }
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
    void pickRequest(
        const AgentID                             requestorId,
        std::inplace_vector<RefWrap<Request>, N>& out,
        RandomGenerator&                          rng
    ) noexcept {
        ASSERT(out.empty());
        if (out.max_size() >= requests_.size())
            packAllRequest(requestorId, out);
        else
            packPartRequest(requestorId, out, rng);
    }

    void clear() noexcept { requests_.clear(); }

  private:
    template <std::size_t N>
    void packAllRequest(const AgentID id, std::inplace_vector<RefWrap<Request>, N>& out) {
        for (auto& req : requests_) {
            if (req.firmID == id) continue;
            out.unchecked_emplace_back(std::ref(req));
        }
    }

    template <std::size_t N>
    void packPartRequest(
        const AgentID id, std::inplace_vector<RefWrap<Request>, N>& out, RandomGenerator& rng
    ) noexcept {
        rng.sample(
            requests_ | std::views::filter([id](const Request& req) noexcept -> bool {
                return req.firmID == id;
            }) | std::views::transform([](Request& req) noexcept -> RefWrap<Request> {
                return std::ref(req);
            }),
            std::back_inserter(out),
            out.max_size()
        );
    }

    tbb::concurrent_vector<Request> requests_;
};

enum class MarketPhase : char {
    RequestAndLayOffs,
    Entry,
    Offer,
    Accept,
    EndRecruiting,
    RecordRosterEntry
};

[[nodiscard]] constexpr auto toMarketPhase(const Month month) noexcept -> MarketPhase {
    return static_cast<MarketPhase>(month.value());
}
}  // namespace abm::labor

namespace abm {
using LaborMarket      = labor::Market;
using LaborMarketPhase = labor::MarketPhase;
}  // namespace abm