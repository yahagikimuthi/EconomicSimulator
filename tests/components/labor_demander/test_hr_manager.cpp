#include "components/labor_demander/hr_manager.hpp"

#include <algorithm>
#include <vector>

#include "core/base.hpp"
#include "core/values/common.hpp"
#include "core/values/labor.hpp"
#include "doctest.h"
#include "world/message.hpp"

namespace labor::demander {

class [[nodiscard]] HumanResourceManagerTester {
  public:
    HumanResourceManagerTester(HumanResourceManager& manager) : manager_{manager} {}
    auto emptyRosterPool() -> auto& { return manager_.emptyRosterPool_; }
    auto roster() -> auto& { return manager_.companyBoard_.roster; }

  private:
    HumanResourceManager& manager_;
};

TEST_CASE("layOffsのテスト") {  // NOLINT
    using HRManager       = HumanResourceManager;
    using HRManagerTester = HumanResourceManagerTester;
    struct Input {
        world::CompanyBoard                      board;
        std::vector<SafePtr<world::RosterEntry>> emptyRosterPool;
        HeadCount                                layOffsCnt;
    };
    struct Expect {
        std::size_t emptyRosterPoolSize_;
    };

    auto makeHRManager{[](Input& input) -> HRManager {
        HRManager       manager{input.board};
        HRManagerTester tester{manager};
        tester.emptyRosterPool() = input.emptyRosterPool;
        return manager;
    }};

    auto makeDummyBoard{[](world::Workspace& workspace) -> world::CompanyBoard& {
        static world::CompanyBoard board{AgentID{42}};
        auto&                      roster = board.roster;
        roster.clear();
        roster.emplace_back(AgentID{0}, Wage{1.0}, board, workspace);
        roster.emplace_back(AgentID{1}, Wage{2.0}, board, workspace);
        roster.emplace_back(AgentID{2}, Wage{3.0}, board, workspace);
        return board;
    }};

    SUBCASE("解雇人数が0人の場合") {
        world::Workspace workspace;
        Input            input{
                       .board = makeDummyBoard(workspace), .emptyRosterPool = {}, .layOffsCnt = HeadCount{0.0}
        };
        const Expect expect{.emptyRosterPoolSize_ = 0UZ};

        auto manager{makeHRManager(input)};
        manager.layOffs(input.layOffsCnt);

        HRManagerTester tester{manager};
        CHECK(std::ranges::all_of(tester.roster(), &world::RosterEntry::isOccupied));
        CHECK(tester.emptyRosterPool().size() == expect.emptyRosterPoolSize_);
    }

    SUBCASE("解雇指定数<在籍人数（ハッピーパス）") {
        world::Workspace workspace;
        Input            input{
                       .board = makeDummyBoard(workspace), .emptyRosterPool = {}, .layOffsCnt = HeadCount{1.0}
        };
        input.board.roster.at(1).isOccupied = false;
        const Expect expect{.emptyRosterPoolSize_ = 1UZ};

        auto manager{makeHRManager(input)};
        manager.layOffs(input.layOffsCnt);

        HRManagerTester tester{manager};
        CHECK(tester.emptyRosterPool().size() == expect.emptyRosterPoolSize_);
        CHECK(not tester.roster().at(0).isOccupied);
        CHECK(not tester.roster().at(1).isOccupied);
        CHECK(tester.roster().at(2).isOccupied);
    }

    SUBCASE("解雇指定数>=在籍人数") {
        world::Workspace workspace;
        Input            input{
                       .board = makeDummyBoard(workspace), .emptyRosterPool = {}, .layOffsCnt = HeadCount{5.0}
        };
        input.board.roster.at(0).isOccupied = false;
        const Expect expect{.emptyRosterPoolSize_ = 2UZ};

        auto manager{makeHRManager(input)};
        manager.layOffs(input.layOffsCnt);

        HRManagerTester tester{manager};
        CHECK(tester.emptyRosterPool().size() == expect.emptyRosterPoolSize_);
        CHECK(not std::ranges::any_of(tester.roster(), &world::RosterEntry::isOccupied));
    }
}
}  // namespace labor::demander