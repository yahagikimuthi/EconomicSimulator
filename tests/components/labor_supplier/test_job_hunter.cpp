#include "components/labor_supplier/job_hunter.hpp"

#include <tbb/concurrent_vector.h>
#include <algorithm>
#include <cstddef>
#include <functional>
#include <ranges>

#include "doctest.h"
#include "helper.hpp"
#include "tests/test_helper.hpp"
#include "world/message.hpp"

template <typename T>
using RefWrapper = std::reference_wrapper<T>;

namespace labor::supplier {
class [[nodiscard]] JobHunterTester {
  public:
    JobHunterTester(JobHunter& jobHunter) : jobHunter_{jobHunter} {}
    auto isPosting() -> bool& { return jobHunter_.isPosting_; }
    auto myEntries() -> std::vector<RefWrapper<world::LaborEntry>>& {
        return jobHunter_.myEntries_;
    }

  private:
    JobHunter& jobHunter_;
};

TEST_CASE("entryのテスト") {  // NOLINT
    const std::uint64_t state{test::makeSeed()};
    const std::uint64_t stream{test::makeSeed()};

    pcg32 masterRng{state, stream};
    INFO("state: " << state);
    INFO("stream: " << stream);

    struct Expect {
        bool        isPosting;
        std::size_t myEntriesSize;
        int         makeEntrySheetCallCnt;
    };

    auto makeJobHunter{[&]() -> JobHunter {
        JobHunter jobHunter{{helper::makeSeed(masterRng), helper::makeSeed(masterRng)}};
        return jobHunter;
    }};

    struct [[nodiscard]] MakeEntrySheet {
        mutable int callCnt{};

        auto operator()(world::LaborRequest& req) const -> world::LaborEntry& {
            ++callCnt;
            return req.entry(AgentID{42}, 1.0);
        }
    };

    SUBCASE("マッチする求人が存在し、上限以下の場合（全件応募）") {
        using world::LaborRequest;
        tbb::concurrent_vector<LaborRequest> requestBox;
        for (const auto i : std::views::iota(0, 6))
            requestBox.emplace_back(AgentID{i}, Wage{i * 100.0});
        struct [[nodiscard]] {
            auto operator()(const world::LaborRequest&) const -> bool { return true; }
        } isAligned;
        MakeEntrySheet makeEntrySheet;

        auto jobHunter{makeJobHunter()};
        jobHunter.entry(isAligned, makeEntrySheet, requestBox, 5, 3);

        const Expect expect{.isPosting = true, .myEntriesSize = 3UZ, .makeEntrySheetCallCnt = 3};

        JobHunterTester tester{jobHunter};
        const auto&     myEntries = tester.myEntries();
        CHECK(tester.isPosting() == expect.isPosting);
        CHECK(myEntries.size() == expect.myEntriesSize);
        CHECK(makeEntrySheet.callCnt == expect.makeEntrySheetCallCnt);
        CHECK(std::ranges::all_of(
            myEntries | std::views::transform([](RefWrapper<world::LaborEntry> e) -> auto {
                const auto& req = e.get().request;
                return &req;
            }),
            [&](const world::LaborRequest* req) -> bool { return req != &requestBox.at(0); }
        ));
    }

    SUBCASE("マッチする求人が上限を超える場合") {
        tbb::concurrent_vector<world::LaborRequest> requestBox;
        for (const auto i : std::views::iota(0, 2))
            requestBox.emplace_back(AgentID{i}, Wage{i * 100.0});
        struct [[nodiscard]] {
            auto operator()(const world::LaborRequest&) const -> bool { return true; }
        } isAligned;
        MakeEntrySheet makeEntrySheet;

        auto jobHunter{makeJobHunter()};
        jobHunter.entry(isAligned, makeEntrySheet, requestBox, 5, 3);

        const Expect expect{.isPosting = true, .myEntriesSize = 2UZ, .makeEntrySheetCallCnt = 2};

        JobHunterTester tester{jobHunter};
        const auto&     myEntries = tester.myEntries();
        CHECK(tester.isPosting() == expect.isPosting);
        CHECK(myEntries.size() == expect.myEntriesSize);
        CHECK(makeEntrySheet.callCnt == expect.makeEntrySheetCallCnt);
        CHECK(&myEntries.at(0).get() != &myEntries.at(1).get());
    }

    SUBCASE("サンプリング後のフィルタリング検証") {
        tbb::concurrent_vector<world::LaborRequest> requestBox;
        for (const auto i : std::views::iota(1, 6))
            requestBox.emplace_back(AgentID{i}, Wage{i * 100.0});
        struct [[nodiscard]] {
            auto operator()(const world::LaborRequest& req) const -> bool {
                if (req.wage >= Wage{450.0}) return true;
                return false;
            }
        } isAligned;
        MakeEntrySheet makeEntrySheet;

        auto jobHunter{makeJobHunter()};
        jobHunter.entry(isAligned, makeEntrySheet, requestBox, 5, 5);

        const Expect    expect{.isPosting = true, .myEntriesSize = 1UZ, .makeEntrySheetCallCnt = 1};
        JobHunterTester tester{jobHunter};

        auto& myEntries = tester.myEntries();
        CHECK(tester.isPosting() == expect.isPosting);
        CHECK(myEntries.size() == expect.myEntriesSize);
        CHECK(makeEntrySheet.callCnt == expect.makeEntrySheetCallCnt);
        auto& myEntry = myEntries.at(0).get();
        CHECK(&myEntry.request == &requestBox.back());
        CHECK(myEntry.request.wage >= Wage{450.0});
    }

    SUBCASE("サンプリングされた求人がすべて条件不適合の場合") {
        tbb::concurrent_vector<world::LaborRequest> requestBox;
        for (const auto i : std::views::iota(1, 6))
            requestBox.emplace_back(AgentID{i}, Wage{i * 100.0});
        struct [[nodiscard]] {
            auto operator()(const world::LaborRequest&) const -> bool { return false; }
        } isAligned;
        MakeEntrySheet makeEntrySheet;

        auto jobHunter{makeJobHunter()};
        jobHunter.entry(isAligned, makeEntrySheet, requestBox, 3, 2);

        const Expect expect{.isPosting = false, .myEntriesSize = 0UZ, .makeEntrySheetCallCnt = 0};
        JobHunterTester tester{jobHunter};
        CHECK(tester.isPosting() == expect.isPosting);
        CHECK(tester.myEntries().size() == expect.myEntriesSize);
        CHECK(makeEntrySheet.callCnt == expect.makeEntrySheetCallCnt);
    }
}
}  // namespace labor::supplier