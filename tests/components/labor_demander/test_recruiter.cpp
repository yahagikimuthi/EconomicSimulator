#include "components/labor_demander/recruiter.hpp"

#include <inplace_vector>

#include "components/labor_demander/common.hpp"
#include "components/labor_demander/human_resource.hpp"
#include "doctest.h"
#include "others/util.hpp"
#include "tests/util.hpp"
#include "values/common.hpp"
#include "values/labor.hpp"
#include "world/base_goods.hpp"
#include "world/labor.hpp"

namespace abm::labor::demander::recruiter {
namespace {
TEST_CASE("Recruiterのテスト") {  // NOLINT
    auto recruiter = Recruiter{};
    auto market    = Market{};
    auto rng       = makeRng();
    auto out       = std::inplace_vector<RefWrap<Request>, 1UZ>{};
    auto hr        = HumanResource{AgentID{42}, Day{1}};
    auto space     = base_goods::Workspace{};

    SUBCASE("オファー数が0の場合") {
        constexpr auto plan =
            RecruitPlan{.wage = Wage{10.0}, .employ = HeadCount{0.0}, .offer = HeadCount{0.0}};
        recruiter.post(AgentID{42}, plan, market);

        SUBCASE("Marketにポストしない") {
            market.pickRequest(AgentID{101}, out, rng);

            CHECK(out.empty());
        }

        SUBCASE("リクルートの結果は空") {
            recruiter.offer();
            const auto result = recruiter.endRecruiting(hr.makeAddRosterFn(space));

            CHECK(result.applicants.isZero());
            CHECK(result.employ.isZero());
        }
    }

    SUBCASE("オファー数が0でない場合") {
        constexpr auto plan =
            RecruitPlan{.wage = Wage{10.0}, .employ = HeadCount{3.0}, .offer = HeadCount{5.0}};
        recruiter.post(AgentID{42}, plan, market);

        SUBCASE("Marketからサンプリングすることが可能で、IDと賃金が等しい") {
            market.pickRequest(AgentID{101}, out, rng);

            CHECK(out.size() == 1UZ);
            const auto& sample = out[0].get();
            CHECK(sample.firmID == AgentID{42});
            CHECK(sample.wage == Wage{10.0});
        }
    }
}
}  // namespace
}  // namespace abm::labor::demander::recruiter