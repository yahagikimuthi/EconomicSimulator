#pragma once

#include <cstddef>
#include <vector>

#include "core/base.hpp"
#include "core/values/common.hpp"
#include "core/values/labor.hpp"
#include "world/message.hpp"

namespace labor::demander {
class [[nodiscard]] HumanResourceManager {
  public:
    HumanResourceManager(world::CompanyBoard& companyBoard) : companyBoard_{companyBoard} {}
    auto addRoster(const AgentID id, const Wage wage, world::Workspace& workspace)
        -> world::RosterEntry& PRE(id >= AgentID{0}) PRE(wage > Wage{0.0});
    void acceptResignation();
    void layOffs(const HeadCount layOffsCnt) PRE(layOffsCnt >= HeadCount{0.0});
    auto employeeCnt() const -> HeadCount POST(cnt : cnt >= HeadCount{0.0}) {
        ASSERT(companyBoard_.roster.size() >= emptyRosterPool_.size());
        const std::size_t rosterSize{companyBoard_.roster.size() - emptyRosterPool_.size()};
        return HeadCount{static_cast<double>(rosterSize)};
    }
    auto sumWage() const -> Wage POST(wage : wage >= Wage{0.0});
    void endStep() { companyBoard_.resignationBox.clear(); }

  private:
    template <typename T>
    using refWrapper = std::reference_wrapper<T>;
    world::CompanyBoard&                        companyBoard_;
    std::vector<refWrapper<world::RosterEntry>> emptyRosterPool_;

    friend class HumanResourceManagerTester;
};
}  // namespace labor::demander