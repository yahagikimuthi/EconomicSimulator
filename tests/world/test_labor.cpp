#include "world/labor.hpp"

#include <utility>

#include "doctest.h"
#include "others/util.hpp"
#include "values/common.hpp"
#include "values/labor.hpp"
#include "world/base_goods.hpp"

namespace abm::labor {
TEST_CASE("Rosterのテスト") {  // NOLINT
    auto roster = Roster{};
    auto board  = CompanyBoard{AgentID{42}, Day{1}};
    auto space  = base_goods::Workspace{};

    SUBCASE("addのみの場合、sumWageが機能することのテスト") {
        nothing(roster.add(AgentID{101}, Wage{101}, board, space));
        nothing(roster.add(AgentID{202}, Wage{202}, board, space));
        nothing(roster.add(AgentID{303}, Wage{303}, board, space));

        CHECK(roster.sumWage().value() == doctest::Approx(606));
    }

    SUBCASE("名簿を削除する場合もsumWageが機能することのテスト") {
        nothing(roster.add(AgentID{101}, Wage{101}, board, space));
        auto& disableEntry = roster.add(AgentID{202}, Wage{202}, board, space);
        nothing(roster.add(AgentID{303}, Wage{303}, board, space));

        disableEntry.resign();

        CHECK(roster.sumWage().value() == doctest::Approx(404));
    }

    SUBCASE("名簿が再利用されていることのテスト") {
        nothing(roster.add(AgentID{101}, Wage{101}, board, space));
        auto* const beforeEntry = &roster.add(AgentID{202}, Wage{202}, board, space);
        nothing(roster.add(AgentID{303}, Wage{303}, board, space));

        beforeEntry->resign();

        auto* const afterEntry = &roster.add(AgentID{404}, Wage{404}, board, space);

        CHECK(beforeEntry == afterEntry);
    }

    SUBCASE("validEntriesがすべて有効であることのテスト") {
        nothing(roster.add(AgentID{101}, Wage{101}, board, space));
        auto& disableEntry = roster.add(AgentID{202}, Wage{202}, board, space);
        nothing(roster.add(AgentID{303}, Wage{303}, board, space));

        disableEntry.resign();

        for (const auto& entry : std::as_const(roster).validEntries()) CHECK(entry.isOccupied());
    }

    SUBCASE("employeeCntのテスト") {
        nothing(roster.add(AgentID{101}, Wage{101}, board, space));
        auto& disableEntry = roster.add(AgentID{202}, Wage{202}, board, space);
        nothing(roster.add(AgentID{303}, Wage{303}, board, space));

        CHECK(roster.employeeCnt().value() == 3);

        disableEntry.resign();

        CHECK(roster.employeeCnt().value() == 2);
    }
}

TEST_CASE("RosterEntryのテスト") {  // NOLINT
    constexpr auto workDay = Day{15};

    auto  roster = Roster{};
    auto  board  = CompanyBoard{AgentID{42}, workDay};
    auto  space  = base_goods::Workspace{};
    auto& entry  = roster.add(AgentID{101}, Wage{101}, board, space);

    SUBCASE("労働日の場合、Workspaceに労働貢献を実際に行うこと") {
        entry.addInput(10.0, Date{workDay.value() - 1});

        const auto input = space.takeOut();
        CHECK(input.value() == 10.0);
    }

    SUBCASE("労働日でない場合、労働貢献は事実上行わないこと") {
        entry.addInput(10.0, Date{2});

        const auto input = space.takeOut();
        CHECK(input.isZero());
    }

    SUBCASE("resignが呼ばれた場合、isOccupiedがfalseとなること") {
        entry.resign();

        CHECK(not entry.isOccupied());
    }
}
}  // namespace abm::labor