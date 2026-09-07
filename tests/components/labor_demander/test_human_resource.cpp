#include "components/labor_demander/human_resource.hpp"

#include "doctest.h"
#include "others/util.hpp"
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

    nothing(addRoster(AgentID{101}, Wage{101}));
    nothing(addRoster(AgentID{202}, Wage{202}));
    nothing(addRoster(AgentID{303}, Wage{303}));

    constexpr auto                  sumWage = 606;
    [[maybe_unused]] constexpr auto avgWage = 202;

    CHECK(hr.employeeCnt().value() == 3);
    CHECK(hr.sumWage().value() == doctest::Approx(sumWage));

    SUBCASE("全員解雇の場合、概算要求はゼロ") {
        const auto reqBudget = hr.planAndRequestBudget(HeadCount{3});
        CHECK(reqBudget.isZero());
    }

    SUBCASE("一部解雇の場合、その分の平均賃金が除かれる") {
        const auto reqBudget = hr.planAndRequestBudget(HeadCount{2});
        CHECK(reqBudget.value() == doctest::Approx(hr.sumWage().value() - (avgWage * 2)));
    }

    SUBCASE("誰も解雇しない場合、総賃金を概算要求") {
        const auto reqBudget = hr.planAndRequestBudget(HeadCount{0});
        CHECK(reqBudget.value() == doctest::Approx(hr.sumWage().value()));
    }
}
}  // namespace
}  // namespace abm::labor::demander