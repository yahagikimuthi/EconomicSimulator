#pragma once

#include <tbb/concurrent_vector.h>
#include <cstddef>
#include <deque>
#include <functional>
#include <inplace_vector>
#include <iterator>
#include <memory>
#include <optional>
#include <ranges>
#include <utility>

#include "others/util.hpp"
#include "values/common.hpp"
#include "values/date.hpp"
#include "values/labor.hpp"
#include "world/base_goods.hpp"

namespace abm::labor {
class RosterEntry;
struct CompanyBoard final {
    explicit CompanyBoard(const AgentID Id, const Day WorkDay) noexcept
        : firmId{Id}, workDay{WorkDay} {}
    const AgentID firmId;
    const Day     workDay;
};

class Roster;
class RosterEntry final {
  public:
    explicit RosterEntry(
        const AgentID          Id,
        const Wage             Wage,
        CompanyBoard&          board,
        base_goods::Workspace& space,
        Roster&                roster
    ) noexcept
        : employeeId{Id}, wage{Wage}, companyBoard_{board}, workspace_{space}, roster_{roster} {
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
    void resign() noexcept;
    void disable() noexcept { isOccupied_ = false; }
    void payWage(const Money payment) noexcept { paidWage_ += payment; }

    [[nodiscard]] auto firmId() const noexcept -> AgentID { return companyBoard_.firmId; }
    [[nodiscard]] auto isOccupied() const noexcept -> bool { return isOccupied_; }
    [[nodiscard]] auto workDay() const noexcept -> Day { return companyBoard_.workDay; }
    [[nodiscard]] auto takeOutPaidWage() noexcept -> Money { return paidWage_; }

    const AgentID employeeId;
    const Wage    wage;

  private:
    CompanyBoard&          companyBoard_;
    base_goods::Workspace& workspace_;
    Roster&                roster_;
    Money                  paidWage_{0.0};
    bool                   isOccupied_{true};
};

class Roster final {
  public:
    explicit Roster() noexcept = default;

    [[nodiscard]] auto add(
        const AgentID id, const Wage wage, CompanyBoard& board, base_goods::Workspace& space
    ) noexcept -> RosterEntry& {
        if (empties_.empty()) return entries_.emplace_back(id, wage, board, space, *this);
        auto& newEntry = empties_.back().get();
        empties_.resize(  // 第二引数はコンパイルエラーを防止するためのダミー
            empties_.size() - 1UZ,
            newEntry
        );
        std::destroy_at(&newEntry);
        std::construct_at(&newEntry, id, wage, board, space, *this);
        return newEntry;
    }

    void resign(RosterEntry& resignation) noexcept {
        resignation.disable();
        empties_.emplace_back(std::ref(resignation));
    }

    [[nodiscard]] auto validEntries() noexcept
        -> auto = delete(
               "&RosterEntry::isOccupied = "
               "falseと途中でされると、rangesは遅延評価であるから、filterを施すことにより意図通りに"
               "動かなくなるため"
           );

    [[nodiscard]] auto validEntries() const noexcept -> auto {
        return std::as_const(entries_) | std::views::filter(&RosterEntry::isOccupied);
    }

    [[nodiscard]] auto rawEntries() noexcept -> auto { return std::ranges::subrange{entries_}; }

    [[nodiscard]] auto employeeCnt() const noexcept -> HeadCount {
        ASSERT(entries_.size() >= empties_.size());
        return HeadCount{entries_.size() - empties_.size()};
    }

  private:
    std::deque<RosterEntry>                      entries_;
    tbb::concurrent_vector<RefWrap<RosterEntry>> empties_;
};

inline void RosterEntry::resign() noexcept { roster_.resign(*this); }

class Request;
class Entry final {
  public:
    explicit Entry(const AgentID Id, const double power, const Request& req) noexcept
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
    explicit Request(const AgentID Id, const Wage Wage) noexcept : firmID{Id}, wage{Wage} {
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
    explicit Market() noexcept = default;

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