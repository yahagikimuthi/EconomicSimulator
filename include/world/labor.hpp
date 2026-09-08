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
    explicit CompanyBoard(const AgentID Id) noexcept : firmId{Id} {}
    const AgentID firmId;
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
        assert(Wage.isPositive());
        assert(Id != companyBoard_.firmId);
    }
    // std::deque<RosterEntry>に対しstd::swapを施すと
    // entrantが持つ参照が無意味となる。
    // よって、代入演算子を明示的削除する。
    RosterEntry(const RosterEntry&) noexcept                    = default;
    auto operator=(const RosterEntry&) noexcept -> RosterEntry& = delete;
    RosterEntry(RosterEntry&&)                                  = delete;
    auto operator=(RosterEntry&&) noexcept -> RosterEntry&      = delete;
    ~RosterEntry() noexcept                                     = default;

    void addInput(const double productPower) noexcept {
        assert(isOccupied_);
        workspace_.addInput(productPower);
    }
    void resign() noexcept;
    void payWage(const Money payment) noexcept {
        assert(paidWage_.isZeroOrMore());
        assert(payment.isZeroOrMore());
        assert(isOccupied_);
        paidWage_ += payment;
    }

    [[nodiscard]] auto firmId() const noexcept -> AgentID { return companyBoard_.firmId; }
    [[nodiscard]] auto isOccupied() const noexcept -> bool { return isOccupied_; }
    [[nodiscard]] auto takeoutPaidWage() noexcept -> Money {
        const auto out = std::exchange(paidWage_, Money{0.0});
        assert(out.isZeroOrMore());
        return out;
    }

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
        assert(wage.isPositive());
        assert(id != board.firmId);

        sumWage_ += wage;

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
        empties_.emplace_back(std::ref(resignation));
        sumWage_ -= resignation.wage;
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
        assert(entries_.size() >= empties_.size());
        return HeadCount{entries_.size() - empties_.size()};
    }

    [[nodiscard]] auto sumWage() const noexcept -> Wage { return sumWage_; }

  private:
    std::deque<RosterEntry>                  entries_;
    tbb::concurrent_vector<Ref<RosterEntry>> empties_;
    Wage                                     sumWage_{0.0};
};

inline void RosterEntry::resign() noexcept {
    assert(paidWage_.isZero());
    isOccupied_ = false;
    roster_.resign(*this);
}

class Request;
class Entry final {
  public:
    explicit Entry(const AgentID Id, const double power, const Request& req) noexcept
        : entrantId{Id}, productPower{power}, request{req} {
        assert(power > 0.0);
    }
    // Request::entries() noexcept -> std::ranges::subrangeを呼び、それに対しstd::sortを施すと
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

    void offer() noexcept {
        assert(not isOffer_);
        isOffer_ = true;
    }
    void accept() noexcept {
        assert(isOffer_);
        assert(not isAccept_);
        isAccept_ = true;
    }
    void setRoster(RosterEntry& rosterEntry) noexcept {
        assert(isAccept_);
        assert(not rosterEntry_);
        rosterEntry_ = rosterEntry;
    }

    [[nodiscard]] auto isOffer() const noexcept -> bool { return isOffer_; }
    [[nodiscard]] auto isAccept() const noexcept -> bool { return isAccept_; }
    [[nodiscard]] auto takeoutRosterEntry() noexcept -> RosterEntry& {
        assert(isAccept_);
        assert(rosterEntry_);
        auto& out = *rosterEntry_;
        rosterEntry_.reset();
        return out;
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
        assert(Wage.isPositive());
    }
    [[nodiscard]] auto entry(const AgentID id, const double productPower) noexcept -> Entry& {
        assert(productPower > 0.0);
        assert(id != firmID);
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
        assert(wage.isPositive());
        return *requests_.emplace_back(id, wage);
    }

    template <std::size_t N>
    void pickRequest(
        const AgentID requestorId, std::inplace_vector<Ref<Request>, N>& out, RandomGenerator& rng
    ) noexcept {
        assert(out.empty());
        if (out.max_size() >= requests_.size())
            packAllRequest(requestorId, out);
        else
            packPartRequest(requestorId, out, rng);
    }

    void clear() noexcept { requests_.clear(); }

  private:
    template <std::size_t N>
    void packAllRequest(const AgentID id, std::inplace_vector<Ref<Request>, N>& out) {
        for (auto& req : requests_) {
            if (req.firmID != id) out.unchecked_emplace_back(std::ref(req));
        }
    }

    template <std::size_t N>
    void packPartRequest(
        const AgentID id, std::inplace_vector<Ref<Request>, N>& out, RandomGenerator& rng
    ) noexcept {
        rng.sample(
            requests_ | std::views::filter([id](const Request& req) noexcept -> bool {
                return req.firmID == id;
            }) | std::views::transform([](Request& req) noexcept -> Ref<Request> {
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
    assert(month.value() >= 1);
    return static_cast<MarketPhase>(month.value() - 1);
}
}  // namespace abm::labor

namespace abm {
using LaborMarket      = labor::Market;
using LaborMarketPhase = labor::MarketPhase;
}  // namespace abm