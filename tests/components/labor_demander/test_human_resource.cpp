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

    nothing(addRoster(AgentID{101}, Wage{101}));
    nothing(addRoster(AgentID{202}, Wage{202}));
    nothing(addRoster(AgentID{303}, Wage{303}));

    constexpr auto                  sumWage = 606;
    [[maybe_unused]] constexpr auto avgWage = 202;

    CHECK(hr.employeeCnt().value() == 3);
    CHECK(hr.sumWage().value() == doctest::Approx(sumWage));
}
}  // namespace
}  // namespace abm::labor::demander