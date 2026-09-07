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

            CHECK(hr.employeeCnt().isZero());
            CHECK(hr.sumWage().isZero());
        }
    }

    SUBCASE("オファー数が0でない場合、サンプリングが可能でID、賃金が等しい") {
        constexpr auto plan =
            RecruitPlan{.wage = Wage{10.0}, .employ = HeadCount{1.0}, .offer = HeadCount{3.0}};
        recruiter.post(AgentID{42}, plan, market);
        market.pickRequest(AgentID{101}, out, rng);
        CHECK(out.size() == 1UZ);
        auto& request = out[0].get();
        CHECK(request.firmID == AgentID{42});
        CHECK(request.wage == Wage{10.0});

        SUBCASE("誰もエントリーしない場合、結果は空") {
            recruiter.offer();
            const auto result = recruiter.endRecruiting(hr.makeAddRosterFn(space));

            CHECK(result.applicants.isZero());
            CHECK(result.employ.isZero());
            CHECK(hr.employeeCnt().isZero());
            CHECK(hr.sumWage().isZero());
        }

        SUBCASE("エントリー数 < オファー数の場合、全員オファーされる") {
            auto& entry1 = request.entry(AgentID{101}, 101);
            auto& entry2 = request.entry(AgentID{202}, 202);

            recruiter.offer();

            CHECK(entry1.isOffer());
            CHECK(entry2.isOffer());

            SUBCASE("全員オファーを受諾しなかった場合、結果は空") {
                const auto result = recruiter.endRecruiting(hr.makeAddRosterFn(space));

                CHECK(result.applicants.value() == 2);
                CHECK(result.employ.isZero());
                CHECK(hr.employeeCnt().isZero());
                CHECK(hr.sumWage().isZero());
            }

            SUBCASE("一部がオファーを受諾した場合、正しく採用が行われる") {
                entry1.accept();

                const auto result = recruiter.endRecruiting(hr.makeAddRosterFn(space));

                const auto& rosterEntry1 = entry1.takeoutRosterEntry();
                CHECK(rosterEntry1.firmId().value() == 42);
                CHECK(rosterEntry1.employeeId.value() == 101);
                CHECK(rosterEntry1.wage.value() == 10);

                CHECK(result.applicants.value() == 2);
                CHECK(result.employ.value() == 1);
                CHECK(hr.employeeCnt().value() == 1);
                CHECK(hr.sumWage().value() == 10.0);
            }

            SUBCASE("全員が受諾した場合も正しく採用される") {
                entry1.accept();
                entry2.accept();

                const auto result = recruiter.endRecruiting(hr.makeAddRosterFn(space));

                const auto& rosterEntry1 = entry1.takeoutRosterEntry();
                const auto& rosterEntry2 = entry2.takeoutRosterEntry();

                CHECK(rosterEntry1.firmId().value() == 42);
                CHECK(rosterEntry1.employeeId.value() == 101);
                CHECK(rosterEntry1.wage.value() == 10);
                CHECK(rosterEntry2.firmId().value() == 42);
                CHECK(rosterEntry2.employeeId.value() == 202);
                CHECK(rosterEntry2.wage.value() == 10);

                CHECK(result.applicants.value() == 2);
                CHECK(result.employ.value() == 2);
                CHECK(hr.employeeCnt().value() == 2);
                CHECK(hr.sumWage().value() == 20.0);
            }
        }

        SUBCASE("エントリー数=オファー数の場合、全員オファーされる") {
            auto& entry1 = request.entry(AgentID{101}, 101);
            auto& entry2 = request.entry(AgentID{202}, 202);
            auto& entry3 = request.entry(AgentID{303}, 303);

            recruiter.offer();

            CHECK(entry1.isOffer());
            CHECK(entry2.isOffer());
            CHECK(entry3.isOffer());

            SUBCASE("誰もオファーを受諾しなかった場合、結果は空") {
                const auto result = recruiter.endRecruiting(hr.makeAddRosterFn(space));

                CHECK(result.applicants.value() == 3);
                CHECK(result.employ.isZero());
                CHECK(hr.employeeCnt().isZero());
                CHECK(hr.sumWage().isZero());
            }

            SUBCASE("一部が受諾した場合、正しく雇用される") {
                entry1.accept();
                entry3.accept();

                const auto result = recruiter.endRecruiting(hr.makeAddRosterFn(space));

                const auto& rosterEntry1 = entry1.takeoutRosterEntry();
                const auto& rosterEntry3 = entry3.takeoutRosterEntry();
                CHECK(rosterEntry1.firmId().value() == 42);
                CHECK(rosterEntry1.employeeId.value() == 101);
                CHECK(rosterEntry1.wage.value() == 10);
                CHECK(rosterEntry3.firmId().value() == 42);
                CHECK(rosterEntry3.employeeId.value() == 303);
                CHECK(rosterEntry3.wage.value() == 10);

                CHECK(result.applicants.value() == 3);
                CHECK(result.employ.value() == 2);
                CHECK(hr.employeeCnt().value() == 2);
                CHECK(hr.sumWage().value() == 20);
            }

            SUBCASE("全員が受諾した場合も全員雇用される") {
                entry1.accept();
                entry2.accept();
                entry3.accept();

                const auto result = recruiter.endRecruiting(hr.makeAddRosterFn(space));

                const auto& rosterEntry1 = entry1.takeoutRosterEntry();
                const auto& rosterEntry2 = entry2.takeoutRosterEntry();
                const auto& rosterEntry3 = entry3.takeoutRosterEntry();

                CHECK(rosterEntry1.firmId().value() == 42);
                CHECK(rosterEntry1.employeeId.value() == 101);
                CHECK(rosterEntry1.wage.value() == 10);
                CHECK(rosterEntry2.firmId().value() == 42);
                CHECK(rosterEntry2.employeeId.value() == 202);
                CHECK(rosterEntry2.wage.value() == 10);
                CHECK(rosterEntry3.firmId().value() == 42);
                CHECK(rosterEntry3.employeeId.value() == 303);
                CHECK(rosterEntry3.wage.value() == 10);

                CHECK(result.applicants.value() == 3);
                CHECK(result.employ.value() == 3);
                CHECK(hr.employeeCnt().value() == 3);
                CHECK(hr.sumWage().value() == 30);
            }
        }

        SUBCASE("応募者数>オファー数の場合、労働生産性が高い人がオファーされる") {
            auto& entry1 = request.entry(AgentID{101}, 101);
            auto& entry2 = request.entry(AgentID{202}, 202);
            auto& entry3 = request.entry(AgentID{303}, 303);
            auto& entry4 = request.entry(AgentID{404}, 404);

            recruiter.offer();
            CHECK(not entry1.isOffer());
            CHECK(entry2.isOffer());
            CHECK(entry3.isOffer());
            CHECK(entry4.isOffer());

            SUBCASE("誰も受諾しない場合、結果は空") {
                const auto result = recruiter.endRecruiting(hr.makeAddRosterFn(space));

                CHECK(result.applicants.value() == 4);
                CHECK(result.employ.value() == 0);
                CHECK(hr.employeeCnt().isZero());
                CHECK(hr.sumWage().isZero());
            }

            SUBCASE("一部が受諾した場合、正しく雇用される") {
                entry3.accept();
                entry4.accept();

                const auto result = recruiter.endRecruiting(hr.makeAddRosterFn(space));

                const auto& rosterEntry3 = entry3.takeoutRosterEntry();
                const auto& rosterEntry4 = entry4.takeoutRosterEntry();

                CHECK(rosterEntry3.firmId().value() == 42);
                CHECK(rosterEntry3.employeeId.value() == 303);
                CHECK(rosterEntry3.wage.value() == 10);
                CHECK(rosterEntry4.firmId().value() == 42);
                CHECK(rosterEntry4.employeeId.value() == 404);
                CHECK(rosterEntry4.wage.value() == 10);

                CHECK(result.applicants.value() == 4);
                CHECK(result.employ.value() == 2);
                CHECK(hr.employeeCnt().value() == 2);
                CHECK(hr.sumWage().value() == 20);
            }
        }
    }
}
}  // namespace
}  // namespace abm::labor::demander::recruiter