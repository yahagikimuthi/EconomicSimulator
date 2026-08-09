#include "components/labor_demander/labor_demander.hpp"

#include <cstddef>
#include <functional>
#include <optional>

#include "components/labor_demander/recruiter.hpp"
#include "core/base.hpp"
#include "core/values/common.hpp"
#include "core/values/labor.hpp"
#include "doctest.h"
#include "tests/components/labor_demander/test_helper.hpp"
#include "world/message.hpp"

namespace labor::demander {
using HRManager       = HumanResourceManager;
using HRManagerTester = HumanResourceManagerTester;
namespace {
template <typename T>
using RefWrapper = std::reference_wrapper<T>;

class DummyPlanner {
  public:
    void judgePlan(HeadCount) {}
    auto wagePlan() const -> Wage { return Wage{1.0}; }
    auto offerPlan() const -> HeadCount { return HeadCount{1.0}; }
    void endStep(world::CensusDropBox&, HeadCount, HeadCount) {}
};
}  // namespace

TEST_CASE("registerMemberのテスト") {
    struct Input {
        world::CompanyBoard                        board;
        bool                                       isPosting;
        std::optional<world::LaborRequest>         myRequest;
        HeadCount                                  employing;
        std::vector<RefWrapper<world::LaborEntry>> offerApplicants;
    };
    struct Expect {
        HeadCount   employing;
        std::size_t rosterSize;
        std::size_t emptyRosterPoolSize;
    };
    auto makeRecruiter{[](Input& input) -> Recruiter<DummyPlanner> {
        DummyPlanner    planner;
        Recruiter       recruiter{planner};
        RecruiterTester tester{recruiter};
        tester.isPosting()       = input.isPosting;
        tester.myRequest()       = input.myRequest;
        tester.employing()       = input.employing;
        tester.offerApplicants() = input.offerApplicants;
        return recruiter;
    }};
    auto makeHrManager{[](Input& input) -> HRManager { return HRManager{input.board}; }};
    auto makeLaborRequest{[]() -> world::LaborRequest {
        world::LaborRequest request{AgentID{42}, Wage{250.0}};
        auto&               entryBox = request.entryBox;
        entryBox                     = {{AgentID{101}, 0.1, request}, {AgentID{102}, 0.2, request}};
        return request;
    }};
    (void)makeRecruiter;
    (void)makeHrManager;
    (void)makeLaborRequest;
}
}  // namespace labor::demander