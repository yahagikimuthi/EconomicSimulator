#include "components/labor_demander/recruiter.hpp"

#include <tbb/concurrent_vector.h>
#include <cstddef>

#include "core/base.hpp"
#include "core/values/common.hpp"
#include "core/values/labor.hpp"
#include "tests/components/labor_demander/test_helper.hpp"
#include "tests/test_helper.hpp"
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

TEST_CASE("postのテスト") {  // NOLINT
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

        Recruiter recruiter{makeRecruiter(input)};
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

        Recruiter recruiter{makeRecruiter(input)};
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

TEST_CASE("offerのテスト") {  // NOLINT
    struct Input {
        bool                         isPosting;
        world::LaborRequest          request;
        SafePtr<world::LaborRequest> myRequest;
        HeadCount                    remainOfferNum;
        HeadCount                    applicantNum;
    };
    struct Expect {
        HeadCount   remainOfferNum;
        HeadCount   applicantNum;
        std::size_t offerApplicantsCnt;
    };
    auto makeRecruiter{[](const Input& input) -> Recruiter<PlannerStub> {
        PlannerStub     planner{.wagePlan_ = Wage{0.0}, .offerPlan_ = HeadCount{0.0}};
        Recruiter       recruiter{planner};
        RecruiterTester tester{recruiter};
        tester.isPosting()      = input.isPosting;
        tester.myRequest()      = input.myRequest;
        tester.remainOfferNum() = input.remainOfferNum;
        tester.applicantNum()   = input.applicantNum;
        return recruiter;
    }};

    SUBCASE("isPosting=falseの場合") {
        Input input{
            .isPosting      = false,
            .request        = {AgentID{42}, Wage{0.0}},
            .myRequest      = nullptr,
            .remainOfferNum = HeadCount{5.0},
            .applicantNum   = HeadCount{0.0}
        };
        const Expect expect{
            .remainOfferNum     = HeadCount{5.0},
            .applicantNum       = HeadCount{0.0},
            .offerApplicantsCnt = 0UZ
        };
        Recruiter recruiter{makeRecruiter(input)};

        recruiter.offer();
        RecruiterTester tester{recruiter};

        CHECK(tester.remainOfferNum() == expect.remainOfferNum);
        CHECK(tester.applicantNum() == expect.applicantNum);
        CHECK(tester.offerApplicants().size() == expect.offerApplicantsCnt);
    }

    SUBCASE("応募者リストが空の場合") {
        Input input{
            .isPosting      = true,
            .request        = {AgentID{42}, Wage{1.0}},
            .myRequest      = nullptr,
            .remainOfferNum = HeadCount{5.0},
            .applicantNum   = HeadCount{0.0}
        };
        input.myRequest        = &input.request;
        input.request.entryBox = {};
        const Expect expect{
            .remainOfferNum     = HeadCount{5.0},
            .applicantNum       = HeadCount{0.0},
            .offerApplicantsCnt = 0UZ
        };
        Recruiter recruiter{makeRecruiter(input)};
        recruiter.offer();
        RecruiterTester tester{recruiter};

        CHECK(tester.remainOfferNum() == expect.remainOfferNum);
        CHECK(tester.applicantNum() == expect.applicantNum);
        CHECK(tester.offerApplicants().size() == expect.offerApplicantsCnt);
    }

    SUBCASE("応募者数<募集枠数") {
        Input input{
            .isPosting      = true,
            .request        = {AgentID{42}, Wage{1.0}},
            .myRequest      = nullptr,
            .remainOfferNum = HeadCount{5.0},
            .applicantNum   = HeadCount{0.0}
        };
        input.myRequest = &input.request;
        input.request.entryBox.emplace_back(AgentID{0}, 1.0, input.request);
        input.request.entryBox.emplace_back(AgentID{1}, 2.0, input.request);

        const Expect expect{
            .remainOfferNum     = HeadCount{3.0},
            .applicantNum       = HeadCount{2.0},
            .offerApplicantsCnt = 2UZ
        };

        Recruiter recruiter{makeRecruiter(input)};
        recruiter.offer();
        RecruiterTester tester{recruiter};

        CHECK(tester.remainOfferNum() == expect.remainOfferNum);
        CHECK(tester.applicantNum() == expect.applicantNum);
        CHECK(tester.offerApplicants().size() == expect.offerApplicantsCnt);
        auto& entryBox = input.request.entryBox;
        CHECK(entryBox.at(0).isOffer);
        CHECK(entryBox.at(1).isOffer);
    }

    SUBCASE("応募者数>募集枠数") {
        Input input{
            .isPosting      = true,
            .request        = {AgentID{42}, Wage{1.0}},
            .myRequest      = nullptr,
            .remainOfferNum = HeadCount{2.0},
            .applicantNum   = HeadCount{0.0}
        };
        input.myRequest = &input.request;
        input.request.entryBox.emplace_back(AgentID{0}, 1.0, input.request);
        input.request.entryBox.emplace_back(AgentID{1}, 2.0, input.request);
        input.request.entryBox.emplace_back(AgentID{2}, 3.0, input.request);
        input.request.entryBox.emplace_back(AgentID{3}, 4.0, input.request);
        input.request.entryBox.emplace_back(AgentID{4}, 5.0, input.request);

        const Expect expect{
            .remainOfferNum     = HeadCount{0.0},
            .applicantNum       = HeadCount{5.0},
            .offerApplicantsCnt = 2UZ
        };

        Recruiter recruiter{makeRecruiter(input)};
        recruiter.offer();
        RecruiterTester tester{recruiter};

        CHECK(tester.remainOfferNum() == expect.remainOfferNum);
        CHECK(tester.applicantNum() == expect.applicantNum);
        CHECK(tester.offerApplicants().size() == expect.offerApplicantsCnt);
        auto& entryBox = input.request.entryBox;
        CHECK(not entryBox.at(0).isOffer);
        CHECK(not entryBox.at(1).isOffer);
        CHECK(not entryBox.at(2).isOffer);
        CHECK(entryBox.at(3).isOffer);
        CHECK(entryBox.at(4).isOffer);
    }
}

TEST_CASE("registerMemberのテスト") {  // NOLINT
    using LEntry = world::LaborEntry;
    struct Input {
        bool                         isPosting;
        HeadCount                    employing;
        std::vector<SafePtr<LEntry>> offerApplicants;
        SafePtr<world::LaborRequest> myRequest;
    };
    struct Expect {
        int       addRosterCallCnt;
        HeadCount employing;
    };
    auto makeRecruiter{[](const Input& input) -> Recruiter<PlannerStub> {
        PlannerStub     planner{.wagePlan_ = Wage{-1.0}, .offerPlan_ = HeadCount{-1.0}};
        Recruiter       recruiter{planner};
        RecruiterTester tester{recruiter};
        tester.isPosting()       = input.isPosting;
        tester.offerApplicants() = input.offerApplicants;
        tester.employing()       = input.employing;
        tester.myRequest()       = input.myRequest;
        return recruiter;
    }};

    struct AddRoster {
        int                            callCnt{};
        std::deque<world::RosterEntry> roster;
        world::CompanyBoard            board{AgentID{0}};
        world::Workspace               workspace{};
        auto operator()(AgentID id, Wage wage) -> SafePtr<world::RosterEntry> {
            ++callCnt;
            return &roster.emplace_back(id, wage, board, workspace);
        }
    };

    SUBCASE("isPosting=falseの場合") {
        AddRoster addRoster;

        world::LaborRequest request{AgentID{42}, Wage{1.0}};
        Input               input{
                          .isPosting       = false,
                          .employing       = HeadCount{10.0},
                          .offerApplicants = {},
                          .myRequest       = &request
        };
        request.entryBox.emplace_back(AgentID{42}, 1.0, request);
        request.entryBox.back().isOffer = true;
        input.offerApplicants.emplace_back(&request.entryBox.at(0));

        const Expect expect{.addRosterCallCnt = 0, .employing = HeadCount{10.0}};
        Recruiter    recruiter{makeRecruiter(input)};
        recruiter.registerMember(addRoster);

        RecruiterTester tester{recruiter};
        CHECK(addRoster.callCnt == expect.addRosterCallCnt);
        CHECK(tester.employing() == expect.employing);
        CHECK(input.offerApplicants.front()->rosterEntry.get() == nullptr);
    }

    SUBCASE("オファーを出したがisAcceptが0のとき") {
        AddRoster addRoster;

        world::LaborRequest request{AgentID{42}, Wage{150.0}};
        Input               input{
                          .isPosting       = false,
                          .employing       = HeadCount{10.0},
                          .offerApplicants = {},
                          .myRequest       = &request
        };
        auto& entryBox = request.entryBox;
        entryBox       = {{AgentID{1}, 0.1, request}, {AgentID{2}, 0.2, request}};
        for (auto& e : entryBox) e.isOffer = true;
        input.offerApplicants = {&entryBox.at(0), &entryBox.at(1)};

        const Expect expect{.addRosterCallCnt = 0, .employing = HeadCount{10.0}};
        Recruiter    recruiter{makeRecruiter(input)};
        recruiter.registerMember(addRoster);

        RecruiterTester tester{recruiter};
        CHECK(addRoster.callCnt == expect.addRosterCallCnt);
        CHECK(tester.employing() == expect.employing);
        CHECK(not input.offerApplicants.at(0)->rosterEntry);
        CHECK(not input.offerApplicants.at(1)->rosterEntry);
    }

    SUBCASE("一部の応募者が受諾した場合") {
        using namespace world;
        AddRoster addRoster;

        LaborRequest request{AgentID{42}, Wage{150.0}};
        Input        input{
                   .isPosting       = true,
                   .employing       = HeadCount{10.0},
                   .offerApplicants = {},
                   .myRequest       = &request
        };
        auto& entryBox = request.entryBox;
        entryBox       = {
            {AgentID{101}, 0.0, request}, {AgentID{101}, 0.0, request}, {AgentID{103}, 0.0, request}
        };
        for (auto& e : request.entryBox) e.isOffer = true;
        entryBox.at(0).isAccept = true;
        entryBox.at(2).isAccept = true;
        input.offerApplicants   = {&entryBox.at(0), &entryBox.at(1), &entryBox.at(2)};

        const Expect expect{.addRosterCallCnt = 2, .employing = HeadCount{12.0}};

        Recruiter recruiter{makeRecruiter(input)};
        recruiter.registerMember(addRoster);

        RecruiterTester tester{recruiter};
        CHECK(addRoster.callCnt == expect.addRosterCallCnt);
        CHECK(tester.employing() == expect.employing);
        CHECK(input.offerApplicants.at(0)->rosterEntry->hholdId == AgentID{101});
        CHECK(input.offerApplicants.at(0)->rosterEntry->wage == Wage{150.0});
        CHECK(input.offerApplicants.at(0)->rosterEntry.get() == &addRoster.roster.at(0));
        CHECK(input.offerApplicants.at(1)->rosterEntry.get() == nullptr);
        CHECK(input.offerApplicants.at(2)->rosterEntry->hholdId == AgentID{103});
        CHECK(input.offerApplicants.at(2)->rosterEntry->wage == Wage{150.0});
        CHECK(input.offerApplicants.at(2)->rosterEntry.get() == &addRoster.roster.at(1));
    }
}
}  // namespace labor::demander