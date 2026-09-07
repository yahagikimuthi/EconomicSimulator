#include "components/labor_demander/human_resource.hpp"

#include "components/finance/firm_finance.hpp"
#include "doctest.h"
#include "others/util.hpp"
#include "tests/util.hpp"
#include "world/base_goods.hpp"

namespace abm::labor::demander {
namespace {
TEST_CASE("HumanResourceのテスト") {  // NOLINT
    auto hr        = HumanResource{AgentID{42}, Day{15}};
    auto space     = base_goods::Workspace{};
    auto addRoster = hr.makeAddRosterFn(space);

    CHECK(hr.employeeCnt().isZero());
    CHECK(hr.sumWage().isZero());
    CHECK(hr.planAndRequestBudget(HeadCount{10.0}).isZero());

    hr.layOffs();
    CHECK(hr.employeeCnt().isZero());
    CHECK(hr.sumWage().isZero());

    auto& roster1 = addRoster(AgentID{101}, Wage{101});
    auto& roster2 = addRoster(AgentID{202}, Wage{202});
    auto& roster3 = addRoster(AgentID{303}, Wage{303});

    constexpr auto                  sumWage = 606;
    [[maybe_unused]] constexpr auto avgWage = 202;

    CHECK(hr.employeeCnt().value() == 3);
    CHECK(hr.sumWage().value() == doctest::Approx(sumWage));

    SUBCASE("全員解雇の場合、概算要求はゼロ、解雇の場合、名簿はすべて無効化") {
        const auto reqBudget = hr.planAndRequestBudget(HeadCount{3});
        CHECK(reqBudget.isZero());
        CHECK(reqBudget == hr.requestedBudget());

        hr.layOffs();

        CHECK(not roster1.isOccupied());
        CHECK(not roster2.isOccupied());
        CHECK(not roster3.isOccupied());

        CHECK(hr.sumWage().isZero());
        CHECK(hr.employeeCnt().isZero());
    }

    SUBCASE("一部解雇の場合、総賃金から平均賃金*解雇数を除いた額を概算要求し、名簿が一部無効化") {
        const auto reqBudget = hr.planAndRequestBudget(HeadCount{2});
        CHECK(reqBudget.value() == doctest::Approx(hr.sumWage().value() - (avgWage * 2)));
        CHECK(reqBudget == hr.requestedBudget());

        hr.layOffs();

        CHECK(not roster1.isOccupied());
        CHECK(not roster2.isOccupied());
        CHECK(roster3.isOccupied());

        CHECK(hr.employeeCnt().value() == 1);
        CHECK(hr.sumWage().value() == doctest::Approx(303));
    }

    SUBCASE("誰も解雇しない場合、総賃金を概算要求、名簿はすべて有効") {
        const auto reqBudget = hr.planAndRequestBudget(HeadCount{0});
        CHECK(reqBudget.value() == doctest::Approx(hr.sumWage().value()));
        CHECK(reqBudget == hr.requestedBudget());

        hr.layOffs();

        CHECK(roster1.isOccupied());
        CHECK(roster2.isOccupied());
        CHECK(roster3.isOccupied());

        CHECK(hr.employeeCnt().value() == 3);
        CHECK(hr.sumWage().value() == doctest::Approx(sumWage));
    }

    SUBCASE("予算が満額回答の場合、結果は変わらない") {
        const auto reqBudget = hr.planAndRequestBudget(HeadCount{2});
        CHECK(reqBudget.value() == doctest::Approx(hr.sumWage().value() - (avgWage * 2)));
        CHECK(reqBudget == hr.requestedBudget());

        hr.revisePlan(reqBudget);

        hr.layOffs();

        CHECK(not roster1.isOccupied());
        CHECK(not roster2.isOccupied());
        CHECK(roster3.isOccupied());

        CHECK(hr.employeeCnt().value() == 1);
        CHECK(hr.sumWage().value() == doctest::Approx(303));
    }

    SUBCASE("与えられた予算が0の場合、全員解雇") {
        nothing(hr.planAndRequestBudget(HeadCount{0}));
        hr.revisePlan(Budget{0});

        hr.layOffs();

        CHECK(not roster1.isOccupied());
        CHECK(not roster2.isOccupied());
        CHECK(not roster3.isOccupied());

        CHECK(hr.employeeCnt().isZero());
        CHECK(hr.sumWage().isZero());
    }

    SUBCASE("与えられた予算が平均賃金分 + 微小のとき、1人だけ残す") {
        nothing(hr.planAndRequestBudget(HeadCount{0}));
        hr.revisePlan(Budget{avgWage + 50});

        hr.layOffs();

        CHECK(not roster1.isOccupied());
        CHECK(not roster2.isOccupied());
        CHECK(roster3.isOccupied());

        CHECK(hr.employeeCnt().value() == 1);
        CHECK(hr.sumWage().value() == doctest::Approx(303));
    }

    SUBCASE("payWageはいかなる場合もsumWage分だけ予算を支払う") {
        auto rng        = makeRng();
        auto finance    = FirmFinance{AgentID{42}, rng};
        auto withdrawFn = finance.makeWithdrawFn(FirmFinance::AccountItem::PersonalCost);

        const auto beforeAsset = finance.asset();

        SUBCASE("特に何もしない場合") {
            hr.payWage(withdrawFn);

            const auto afterAsset = finance.asset();
            const auto paid       = beforeAsset - afterAsset;

            CHECK(paid.value() == doctest::Approx(hr.sumWage().value()));

            const auto sum =
                roster1.takeoutPaidWage() + roster2.takeoutPaidWage() + roster3.takeoutPaidWage();
            CHECK(sum.value() == doctest::Approx(paid.value()));
        }

        SUBCASE("全員解雇が発生した場合、支払いはゼロ") {
            nothing(hr.planAndRequestBudget(HeadCount{3}));
            hr.layOffs();

            hr.payWage(withdrawFn);

            const auto afterAsset = finance.asset();
            const auto paid       = beforeAsset - afterAsset;
            CHECK(paid.value() == doctest::Approx(0.0));

            const auto sum =
                roster1.takeoutPaidWage() + roster2.takeoutPaidWage() + roster3.takeoutPaidWage();
            CHECK(sum.value() == paid.value());
        }

        SUBCASE("一部解雇が発生した場合も正しく支払われる") {
            nothing(hr.planAndRequestBudget(HeadCount{2}));
            hr.layOffs();

            hr.payWage(withdrawFn);

            const auto afterAsset = finance.asset();
            const auto paid       = beforeAsset - afterAsset;
            CHECK(paid.value() == doctest::Approx(303));

            const auto sum =
                roster1.takeoutPaidWage() + roster2.takeoutPaidWage() + roster3.takeoutPaidWage();
            CHECK(sum.value() == paid.value());
        }
    }
}
}  // namespace
}  // namespace abm::labor::demander