#pragma once

#include <cstddef>
#include <functional>
#include <memory>
#include <utility>
#include <vector>

#include "core/base.hpp"
#include "core/values/common.hpp"
#include "core/values/labor.hpp"
#include "world/message.hpp"

namespace abm::labor::demander::human_resource {
class EmptyRosterPool {
    template <typename T>
    using RefWrap = std::reference_wrapper<T>;

  public:
    [[nodiscard]] EmptyRosterPool() = default;
    [[nodiscard]] auto size() const -> std::size_t { return pool_.size(); }
    [[nodiscard]] auto empty() const -> bool { return size() == 0UZ; }

    auto popBackEntry() -> RosterEntry& {
        ASSERT(not empty());
        RosterEntry& back = pool_.back().get();
        pool_.pop_back();
        return back;
    }
    void add(RosterEntry& entry) { pool_.emplace_back(std::ref(entry)); }

  private:
    std::vector<RefWrap<RosterEntry>> pool_;
};

class HumanResource {
  public:
    [[nodiscard]] HumanResource(CompanyBoard&& companyBoard)
        : companyBoard_{std::move(companyBoard)} {}

    auto addRoster(const AgentID id, const Wage wage, Workspace& workspace)
        -> RosterEntry& PRE(id >= AgentID{0}) PRE(wage > Wage{0.0}) {
        sumWage_ += wage;
        if (emptyRosterPool_.empty()) return companyBoard_.addRoster(id, wage, workspace);
        RosterEntry* newRoster = &emptyRosterPool_.popBackEntry();
        ASSERT(newRoster != nullptr);
        std::destroy_at(newRoster);
        std::construct_at(newRoster, id, wage, companyBoard_, workspace);
        return *newRoster;
    }

    void acceptResignation() {
        auto& resignationBox = companyBoard_.resignationBox;
        for (RosterEntry& resignEntry : resignationBox) {
            ASSERT(resignEntry.isOccupied);
            resignEntry.isOccupied = false;
            emptyRosterPool_.add(resignEntry);
            sumWage_ -= resignEntry.wage;
        }
        resignationBox.clear();
    }

    void layOffs(const HeadCount layOffsCnt) PRE(layOffsCnt >= HeadCount{0.0}) {
        auto currentLayOffs = HeadCount{0.0};
        for (auto& entry : companyBoard_.roster) {
            if (currentLayOffs >= layOffsCnt) break;
            if (not entry.isOccupied) continue;
            entry.isOccupied = false;
            emptyRosterPool_.add(entry);
            ++currentLayOffs;
            sumWage_ -= entry.wage;
        }
    }

    [[nodiscard]] auto employeeCnt() const -> HeadCount POST(cnt : cnt >= HeadCount{0.0}) {
        ASSERT(companyBoard_.roster.size() >= emptyRosterPool_.size());
        const auto rosterSize{companyBoard_.roster.size() - emptyRosterPool_.size()};
        return HeadCount{static_cast<double>(rosterSize)};
    }

    [[nodiscard]] auto sumWage() const -> Wage POST(wage : wage >= Wage{0.0}) { return sumWage_; }

  private:
    CompanyBoard    companyBoard_;
    EmptyRosterPool emptyRosterPool_;
    Wage            sumWage_{0.0};
};
}  // namespace abm::labor::demander::human_resource

namespace abm::labor::demander {
using HumanResource = human_resource::HumanResource;
}