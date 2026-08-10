#include "components/labor_demander/labor_demander.hpp"

#include <cstddef>
#include <functional>
#include <optional>

#include "components/labor_demander/recruiter.hpp"
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
        std::optional<world::LaborRequest&>        myRequest;
        HeadCount                                  employing;
        std::vector<RefWrapper<world::LaborEntry>> offerApplicants;
    };
    struct Expect {
        HeadCount   employing;
        std::size_t rosterSize;
        std::size_t emptyRosterPoolSize;
    };
    auto makeDummyBoard{[](world::Workspace& workspace) -> world::CompanyBoard {
        world::CompanyBoard board{AgentID{42}};
        auto&               roster = board.roster;
        roster.emplace_back(AgentID{0}, Wage{1.0}, board, workspace);
        roster.emplace_back(AgentID{1}, Wage{2.0}, board, workspace);
        roster.emplace_back(AgentID{2}, Wage{3.0}, board, workspace);
        return board;
    }};
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
    auto makeHrManager{[](Input& input) -> HRManager { return HRManager{std::move(input.board)}; }};
    auto makeLaborRequest{[]() -> world::LaborRequest {
        world::LaborRequest request{AgentID{42}, Wage{250.0}};
        auto&               entryBox = request.entryBox;
        entryBox                     = {{AgentID{101}, 0.1, request}, {AgentID{102}, 0.2, request}};
        return request;
    }};

    SUBCASE("正常系") {
        world::Workspace    workspace;
        world::LaborRequest request{AgentID{42}, Wage{250.0}};
        request.entry(AgentID{101}, 1.0);
        request.entry(AgentID{102}, 2.0);

        Input input{
            .board           = makeDummyBoard(workspace),
            .isPosting       = true,
            .myRequest       = request,
            .employing       = HeadCount{10.0},
            .offerApplicants = {}
        };
        auto& entryBox        = request.entryBox;
        input.offerApplicants = {std::ref(entryBox.at(0)), std::ref(entryBox.at(1))};
    }
}
}  // namespace labor::demander