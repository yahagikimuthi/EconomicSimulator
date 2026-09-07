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

    [[maybe_unused]] auto& rosterEntry = roster.add(AgentID{42}, Wage{15}, board, space);

    auto       rng         = makeRng();
    auto       finance     = HHoldFinance{AgentID{42}, rng};
    const auto beforeAsset = finance.asset();

    auto req1 = Request{AgentID{42}, Wage{10}};
    auto req2 = Request{AgentID{42}, Wage{15}};
    auto req3 = Request{AgentID{42}, Wage{20}};
    auto req4 = Request{AgentID{1}, Wage{10}};
    auto req5 = Request{AgentID{1}, Wage{15}};
    auto req6 = Request{AgentID{1}, Wage{20}};

    auto employment = Employment{rng};

    SUBCASE("デフォルトの場合の挙動テスト") {
        CHECK(not employment.isEmployed());

        employment.work(finance.makeDepositFn(), Date{9});
        const auto afterAsset = finance.asset();

        CHECK(beforeAsset.value() == doctest::Approx(afterAsset.value()));

        CHECK(employment.wage().isZero());

        auto isAligned = employment.makeIsAlignedRequestFn();

        CHECK(isAligned(req1));
        CHECK(isAligned(req2));
        CHECK(isAligned(req3));
        CHECK(isAligned(req4));
        CHECK(isAligned(req5));
        CHECK(isAligned(req6));
    }

    SUBCASE("雇用されている場合") {
        employment.startWorking(rosterEntry, finance.makeDepositFn());
        rosterEntry.payWage(Money{10});

        employment.work(finance.makeDepositFn(), Date{14});
        const auto afterAsset = finance.asset();

        CHECK(afterAsset.value() == doctest::Approx(10 + beforeAsset.value()));
        CHECK(space.takeout().isPositive());

        employment.work(finance.makeDepositFn(), Date{1});
        const auto finalAsset = finance.asset();

        CHECK(finalAsset.value() == doctest::Approx(afterAsset.value()));
        CHECK(space.takeout().isZero());
    }
}
}  // namespace abm::labor::supplier