#include "components/labor_demander/recruiter.hpp"

#include <inplace_vector>

#include "components/labor_demander/common.hpp"
#include "doctest.h"
#include "others/util.hpp"
#include "tests/util.hpp"
#include "values/common.hpp"
#include "values/labor.hpp"
#include "world/labor.hpp"

namespace abm::labor::demander::recruiter {
namespace {
TEST_CASE("Recruiterのテスト") {  // NOLINT
    auto recruiter = Recruiter{};
    auto market    = Market{};
    auto rng       = makeRng();
    auto out       = std::inplace_vector<RefWrap<Request>, 1UZ>{};

    SUBCASE("オファー数が0の場合、Marketにポストしない") {
        const auto plan =
            RecruitPlan{.wage = Wage{10.0}, .employ = HeadCount{0.0}, .offer = HeadCount{0.0}};
        recruiter.post(AgentID{42}, plan, market);

        market.pickRequest(AgentID{101}, out, rng);

        CHECK(out.empty());
    }
}
}  // namespace
}  // namespace abm::labor::demander::recruiter