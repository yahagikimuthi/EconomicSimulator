#include "components/labor_supplier/employment.hpp"

#include "components/finance/others_finance.hpp"
#include "doctest.h"
#include "tests/util.hpp"
#include "world/base_goods.hpp"
#include "world/labor.hpp"

namespace abm::labor::supplier {
TEST_CASE("Employmentのテスト") {  // NOLINT
    auto roster = Roster{};
    auto board  = CompanyBoard{AgentID{101}, Day{15}};
    auto space  = base_goods::Workspace{};

    [[maybe_unused]] auto& rosterEntry = roster.add(AgentID{42}, Wage{10}, board, space);

    auto       rng         = makeRng();
    auto       finance     = HHoldFinance{AgentID{42}, rng};
    const auto beforeAsset = finance.asset();

    auto employment = Employment{rng};

    SUBCASE("デフォルトの場合の挙動テスト") {
        CHECK(not employment.isEmployed());

        employment.work(finance.makeDepositFn(), Date{9});
        const auto afterAsset = finance.asset();

        CHECK(beforeAsset.value() == doctest::Approx(afterAsset.value()));

        CHECK(employment.wage().isZero());
    }
}
}  // namespace abm::labor::supplier