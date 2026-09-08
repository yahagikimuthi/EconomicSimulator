#include "components/labor_supplier/employment.hpp"

#include "components/finance/others_finance.hpp"
#include "doctest.h"
#include "tests/util.hpp"
#include "values/common.hpp"
#include "values/date.hpp"
#include "values/labor.hpp"
#include "world/base_goods.hpp"
#include "world/labor.hpp"

namespace abm::labor::supplier {
namespace {
TEST_CASE("Employmentのテスト") {  // NOLINT
    auto       roster      = Roster{};
    auto       board       = CompanyBoard{AgentID{101}, Day{15}};
    auto       space       = base_goods::Workspace{};
    auto&      rosterEntry = roster.add(AgentID{42}, Wage{15}, board, space);
    auto       rng         = makeRng();
    auto       finance     = HHoldFinance{AgentID{42}, rng};
    const auto beforeAsset = finance.asset();

    auto req1 = Request{AgentID{101}, Wage{10}};
    auto req2 = Request{AgentID{101}, Wage{15}};
    auto req3 = Request{AgentID{101}, Wage{20}};
    auto req4 = Request{AgentID{202}, Wage{10}};
    auto req5 = Request{AgentID{202}, Wage{15}};
    auto req6 = Request{AgentID{202}, Wage{20}};

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
        CHECK(employment.isEmployed());
        CHECK(employment.wage().value() == doctest::Approx(15));

        SUBCASE("賃金を支払わない場合、資産の増減はなし") {
            employment.work(finance.makeDepositFn(), Date{1});
            const auto afterAsset = finance.asset();
            CHECK(afterAsset.value() == doctest::Approx(beforeAsset.value()));
        }

        SUBCASE("賃金が支払われた場合、資産が増加する、その後支払わなかった場合は増加しない") {
            rosterEntry.payWage(Money{10});

            employment.work(finance.makeDepositFn(), Date{1});
            const auto afterAsset = finance.asset();
            CHECK(afterAsset.value() == doctest::Approx(beforeAsset.value() + 10));

            employment.work(finance.makeDepositFn(), Date{1});
            const auto finalAsset = finance.asset();
            CHECK(finalAsset.value() == doctest::Approx(afterAsset.value()));
        }

        SUBCASE("労働日の場合、労働貢献が行われる") {
            CHECK(space.takeout().isZero());
            employment.work(finance.makeDepositFn(), Date{15});
            CHECK(space.takeout().isPositive());
        }

        SUBCASE("isAlignedはIDが同じ場合または賃金が現状以下の場合false、それ以外trueを返す") {
            auto isAligned = employment.makeIsAlignedRequestFn();

            CHECK(not isAligned(req1));
            CHECK(not isAligned(req2));
            CHECK(not isAligned(req3));
            CHECK(not isAligned(req4));
            CHECK(not isAligned(req5));
            CHECK(isAligned(req6));
        }

        SUBCASE("エントリーで書かれた労働生産性と同じ分だけ労働貢献をすること") {
            const auto entry = employment.makeEntrySheetFn(AgentID{42})(req6);

            employment.work(finance.makeDepositFn(), Date{15});

            CHECK(space.takeout().value() == doctest::Approx(entry.productPower));
        }

        SUBCASE("転職の場合") {
            auto newRoster      = Roster{};
            auto newBoard       = CompanyBoard{AgentID{202}, Day{15}};
            auto newRosterEntry = newRoster.add(AgentID{42}, Wage{18}, newBoard, space);

            SUBCASE("事前に賃金が支払われていても回収する") {
                rosterEntry.payWage(Money{10});

                employment.startWorking(newRosterEntry, finance.makeDepositFn());

                CHECK(employment.isEmployed());
                CHECK(employment.wage().value() == 18);
                CHECK(finance.asset().value() == doctest::Approx(beforeAsset.value() + 10));
            }

            SUBCASE("isAlignedは適正に動く") {
                employment.startWorking(newRosterEntry, finance.makeDepositFn());
                auto isAligned = employment.makeIsAlignedRequestFn();

                CHECK(not isAligned(req1));
                CHECK(not isAligned(req2));
                CHECK(isAligned(req3));
                CHECK(not isAligned(req4));
                CHECK(not isAligned(req5));
                CHECK(not isAligned(req6));
            }
        }
    }
}
}  // namespace
}  // namespace abm::labor::supplier