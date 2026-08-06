#include "components/labor_demander/hr_manager.hpp"

#include <algorithm>
#include <cstddef>
#include <vector>

#include "core/base.hpp"
#include "core/values/common.hpp"
#include "core/values/labor.hpp"
#include "doctest.h"
#include "test_helper.hpp"
#include "world/message.hpp"

using namespace test::helper;

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

    auto makeDummyBoard{[](world::Workspace& workspace) -> world::CompanyBoard {
        world::CompanyBoard board{AgentID{42}};
        auto&               roster = board.roster;
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

TEST_CASE("addRosterのテスト") {  // NOLINT
    using HRManager       = HumanResourceManager;
    using HRManagerTester = HumanResourceManagerTester;
    struct Input {
        world::CompanyBoard                      board;
        std::vector<SafePtr<world::RosterEntry>> emptyRosterPool;
        AgentID                                  id;
        Wage                                     wage;
    };
    struct Expect {
        std::size_t rosterSize;
        AgentID     id;
        Wage        wage;
        std::size_t emptyRosterPoolSize;
    };

    auto makeHRManager{[](Input& input) -> HRManager {
        HRManager       manager{input.board};
        HRManagerTester tester{manager};
        tester.emptyRosterPool() = input.emptyRosterPool;
        return manager;
    }};

    auto makeDummyBoard{[](world::Workspace& workspace) -> world::CompanyBoard {
        world::CompanyBoard board{AgentID{42}};
        auto&               roster = board.roster;
        roster.emplace_back(AgentID{0}, Wage{1.0}, board, workspace);
        roster.emplace_back(AgentID{1}, Wage{2.0}, board, workspace);
        roster.emplace_back(AgentID{2}, Wage{3.0}, board, workspace);
        return board;
    }};

    SUBCASE("空きプールが空の場合") {
        world::Workspace workspace;

        Input input{
            .board           = makeDummyBoard(workspace),
            .emptyRosterPool = {},
            .id              = AgentID{101},
            .wage            = Wage{200.0}
        };
        const Expect expect{
            .rosterSize = 4UZ, .id = input.id, .wage = input.wage, .emptyRosterPoolSize = 0UZ
        };

        auto                        manager{makeHRManager(input)};
        SafePtr<world::RosterEntry> newRoster{manager.addRoster(input.id, input.wage, workspace)};

        HRManagerTester tester{manager};
        CHECK(tester.emptyRosterPool().size() == expect.emptyRosterPoolSize);
        CHECK(tester.roster().size() == expect.rosterSize);
        CHECK(newRoster.get() == &tester.roster().at(3));
        CHECK(newRoster->hholdId == expect.id);
        CHECK(newRoster->wage == expect.wage);
    }

    SUBCASE("空きプールが存在する場合") {
        world::Workspace workspace;

        Input input{
            .board           = makeDummyBoard(workspace),
            .emptyRosterPool = {},
            .id              = AgentID{202},
            .wage            = Wage{300.0}
        };
        input.board.roster.at(1).isOccupied = false;
        input.emptyRosterPool.emplace_back(&input.board.roster.at(1));
        const Expect expect{
            .rosterSize = 3UZ, .id = input.id, .wage = input.wage, .emptyRosterPoolSize = 0UZ
        };

        auto                        manager{makeHRManager(input)};
        SafePtr<world::RosterEntry> newRoster{manager.addRoster(input.id, input.wage, workspace)};

        HRManagerTester tester{manager};
        CHECK(tester.emptyRosterPool().size() == expect.emptyRosterPoolSize);
        CHECK(tester.roster().size() == expect.rosterSize);
        CHECK(newRoster.get() == &tester.roster().at(1));
        CHECK(newRoster->hholdId == expect.id);
        CHECK(newRoster->wage == expect.wage);
    }
}

TEST_CASE("sumWageのテスト") {
    using HRManager = HumanResourceManager;
    auto makeHRManager{[](world::CompanyBoard& board) -> HRManager {
        HRManager manager{board};
        return manager;
    }};

    auto makeDummyBoard{[](world::Workspace& workspace) -> world::CompanyBoard {
        world::CompanyBoard board{AgentID{42}};
        auto&               roster = board.roster;
        roster.emplace_back(AgentID{0}, Wage{101.0}, board, workspace);
        roster.emplace_back(AgentID{1}, Wage{102.0}, board, workspace);
        roster.emplace_back(AgentID{2}, Wage{103.0}, board, workspace);
        return board;
    }};

    SUBCASE("空き名簿が存在しない場合") {
        world::Workspace workspace;

        auto board{makeDummyBoard(workspace)};
        auto manager{makeHRManager(board)};
        Wage sumWage{manager.sumWage()};
        CHECK(equal(sumWage, Wage{306.0}));
    }

    SUBCASE("空き名簿が存在する場合") {
        world::Workspace workspace;

        auto board{makeDummyBoard(workspace)};
        board.roster.at(1).isOccupied = false;

        auto manager{makeHRManager(board)};
        Wage sumWage{manager.sumWage()};
        CHECK(equal(sumWage, Wage{204.0}));
    }
}
}  // namespace labor::demander