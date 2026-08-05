#include "components/labor_demander/recruiter.hpp"

#include <tbb/concurrent_vector.h>
#include <cstddef>

#include "core/base.hpp"
#include "core/values/common.hpp"
#include "core/values/labor.hpp"
#include "test_helper.hpp"
#include "world/message.hpp"

using namespace test::helper;

namespace labor::demander {
namespace {
class [[nodiscard]] PlannerStub {
  public:
    void judgePlan(const HeadCount) {}
    auto wagePlan() const -> Wage { return wagePlan_; }
    auto offerPlan() const -> HeadCount { return offerPlan_; }
    void endStep(world::CensusDropBox&, HeadCount, HeadCount) {}

    Wage      wagePlan_;
    HeadCount offerPlan_;
};
}  // namespace

class [[nodiscard]] RecruiterTester {
  public:
    RecruiterTester(const Recruiter<PlannerStub>& recruiter) : recruiter_{recruiter} {}
    auto isRecruiting() const -> bool { return recruiter_.isRecruiting_; }
    auto isPosting() const -> bool { return recruiter_.isPosting_; }
    auto myRequest() const -> SafePtr<world::LaborRequest> { return recruiter_.myRequest_; }
    auto remainOfferNum() const -> HeadCount { return recruiter_.ledger_.remainOfferNum; }

  private:
    const Recruiter<PlannerStub>& recruiter_;
};

TEST_CASE("postのテスト") {
    struct Input {
        HeadCount                                   offerPlan;
        Wage                                        wagePlan;
        AgentID                                     id;
        HeadCount                                   desiredEmploy;
        tbb::concurrent_vector<world::LaborRequest> requestBox;
    };

    struct Expect {
        bool                         isRecruiting;
        bool                         isPosting;
        std::size_t                  requestBoxSize;
        SafePtr<world::LaborRequest> requestPtr;
        HeadCount                    remainOfferNum;
    };

    auto makeRecruiter{[](const Input& input) -> Recruiter<PlannerStub> {
        PlannerStub planner{.wagePlan_ = input.wagePlan, .offerPlan_ = input.offerPlan};
        return {planner};
    }};

    SUBCASE("オファー予定数が0の場合") {
        Input input{
            .offerPlan     = HeadCount{0.0},
            .wagePlan      = Wage{1.0},
            .id            = AgentID{1},
            .desiredEmploy = HeadCount{0.0},
            .requestBox    = {}
        };
        const Expect expect{
            .isRecruiting   = true,
            .isPosting      = false,
            .requestBoxSize = 0UZ,
            .requestPtr     = nullptr,
            .remainOfferNum = HeadCount{0}
        };

        Recruiter<PlannerStub> recruiter{makeRecruiter(input)};
        recruiter.post(input.id, input.desiredEmploy, input.requestBox);
        RecruiterTester tester{recruiter};

        CHECK(tester.isRecruiting() == expect.isRecruiting);
        CHECK(tester.isPosting() == expect.isPosting);
        CHECK(input.requestBox.size() == expect.requestBoxSize);
        CHECK(tester.myRequest().get() == expect.requestPtr.get());
        CHECK(equal(tester.remainOfferNum(), expect.remainOfferNum));
    }

    SUBCASE("オファー数が1以上のとき") {
        Input input{
            .offerPlan     = HeadCount{5.0},
            .wagePlan      = Wage{150.0},
            .id            = AgentID{42},
            .desiredEmploy = HeadCount{10.0},
            .requestBox    = {}
        };

        Recruiter<PlannerStub> recruiter{makeRecruiter(input)};
        recruiter.post(input.id, input.desiredEmploy, input.requestBox);
        RecruiterTester tester{recruiter};

        const Expect expect{
            .isRecruiting   = true,
            .isPosting      = true,
            .requestBoxSize = 1UZ,
            .requestPtr     = &input.requestBox.back(),
            .remainOfferNum = HeadCount{5}
        };

        CHECK(tester.isRecruiting() == expect.isRecruiting);
        CHECK(tester.isPosting() == expect.isPosting);
        CHECK(input.requestBox.size() == expect.requestBoxSize);
        CHECK(tester.myRequest().get() == expect.requestPtr.get());
        CHECK(equal(tester.remainOfferNum(), expect.remainOfferNum));
        CHECK(input.requestBox.back().firmID == input.id);
        CHECK(equal(input.requestBox.back().wage, input.wagePlan));
    }
}
}  // namespace labor::demander