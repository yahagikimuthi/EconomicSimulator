#include "world/labor.hpp"

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
        nothing(roster.add(AgentID{102}, Wage{102}, board, space));
        nothing(roster.add(AgentID{103}, Wage{103}, board, space));

        CHECK(roster.sumWage().value() == doctest::Approx(306));
    }

    SUBCASE("名簿を削除する場合もsumWageが機能することのテスト") {
        nothing(roster.add(AgentID{101}, Wage{101}, board, space));
        auto& disableEntry = roster.add(AgentID{202}, Wage{202}, board, space);
        nothing(roster.add(AgentID{303}, Wage{303}, board, space));

        disableEntry.resign();

        CHECK(roster.sumWage().value() == doctest::Approx(404));
    }
}
}  // namespace abm::labor