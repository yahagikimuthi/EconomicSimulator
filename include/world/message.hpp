#pragma once

#include <tbb/concurrent_vector.h>
#include <atomic>
#include <deque>
#include <optional>
#include <ranges>

#include "config.hpp"
#include "core/forward.hpp"
#include "core/values/common.hpp"
#include "core/values/goods.hpp"
#include "core/values/labor.hpp"

namespace abm {
enum class Market : char { labor, consumerGoods, productionGoods };

class [[nodiscard]] Workspace {
  public:
    Workspace(const double power) : firmProductPower_{power} {}
    ~Workspace() = default;
    Workspace(const Workspace& other);
    auto operator=(const Workspace& other) -> Workspace&;
    Workspace(Workspace&& other) noexcept;
    auto operator=(Workspace&& other) noexcept -> Workspace&;

    void addInput(const double workerProductPower) noexcept {
        const double input{firmProductPower_ * workerProductPower};
        totalInput_.fetch_add(input);
    }
    auto totalInput() const noexcept -> GoodsQuantity { return GoodsQuantity{totalInput_.load()}; }
    void resetInput() noexcept { totalInput_.store(0.0); }

  private:
    double              firmProductPower_;
    std::atomic<double> totalInput_;
};

inline Workspace::Workspace(const Workspace& other)
    : firmProductPower_{other.firmProductPower_}, totalInput_{other.totalInput_.load()} {}
inline auto Workspace::operator=(const Workspace& other) -> Workspace& {
    if (this == &other) return *this;
    firmProductPower_ = other.firmProductPower_;
    const double input{other.totalInput_.load()};
    totalInput_.store(input);
    return *this;
}
inline Workspace::Workspace(Workspace&& other) noexcept
    : firmProductPower_{other.firmProductPower_}, totalInput_{other.totalInput_.load()} {
    other.firmProductPower_ = 0.0;
    other.totalInput_.store(0.0);
}
inline auto Workspace::operator=(Workspace&& other) noexcept -> Workspace& {
    if (this == &other) return *this;
    firmProductPower_ = other.firmProductPower_;
    const double input{other.totalInput_.load()};
    totalInput_.store(input);

    other.firmProductPower_ = 0.0;
    other.totalInput_.store(0.0);
    return *this;
}

struct [[nodiscard]] CompanyBoard {
    const AgentID                                               firmId;
    const Market                                                firmType;
    std::deque<RosterEntry>                                     roster;
    tbb::concurrent_vector<std::reference_wrapper<RosterEntry>> resignationBox;

    CompanyBoard(const AgentID Id, const Market FirmType) : firmId{Id}, firmType{FirmType} {}
    void resign(RosterEntry& resignEntry) { resignationBox.emplace_back(std::ref(resignEntry)); }
    auto addRoster(const AgentID id, const Wage wage, Workspace& workspace) -> RosterEntry&;
};

class [[nodiscard]] RosterEntry {
  public:
    RosterEntry(const AgentID Id, const Wage Wage, CompanyBoard& CompanyBoard, Workspace& Workspace)
        : hholdId{Id}, wage{Wage}, companyBoard{CompanyBoard}, workspace{Workspace} {}
    void addInput(const double productPower) { workspace.addInput(productPower); }
    void resign() { companyBoard.resign(*this); }
    auto firmId() const -> AgentID { return companyBoard.firmId; }
    auto firmType() const -> Market { return companyBoard.firmType; }

    const AgentID hholdId;
    const Wage    wage;
    bool          isOccupied{true};

  private:
    CompanyBoard& companyBoard;
    Workspace&    workspace;
};

inline auto CompanyBoard::addRoster(const AgentID id, const Wage wage, Workspace& workspace)
    -> RosterEntry& {
    return roster.emplace_back(id, wage, *this, workspace);
}

struct LaborEntry {
    const AgentID hholdID;
    const double  productPower;

    bool isOffer{false};
    bool isAccept{false};

    std::optional<RosterEntry&> rosterEntry{std::nullopt};
    const LaborRequest&         request;

    LaborEntry(const AgentID Id, const double ProductPower, const LaborRequest& Request)
        : hholdID{Id}, productPower{ProductPower}, request{Request} {}
};

struct [[nodiscard]] LaborRequest {
    using EntryBoxT = std::ranges::subrange<
        std::ranges::iterator_t<tbb::detail::d1::concurrent_vector<
            abm::LaborEntry,
            tbb::detail::d1::cache_aligned_allocator<abm::LaborEntry>>&>,
        std::ranges::sentinel_t<tbb::detail::d1::concurrent_vector<
            abm::LaborEntry,
            tbb::detail::d1::cache_aligned_allocator<abm::LaborEntry>>&>,
        (std::ranges::sized_range<tbb::detail::d1::concurrent_vector<abm::LaborEntry>&> ||
         std::sized_sentinel_for<
             std::ranges::sentinel_t<tbb::detail::d1::concurrent_vector<
                 abm::LaborEntry,
                 tbb::detail::d1::cache_aligned_allocator<abm::LaborEntry>>&>,
             std::ranges::iterator_t<tbb::detail::d1::concurrent_vector<
                 abm::LaborEntry,
                 tbb::detail::d1::cache_aligned_allocator<abm::LaborEntry>>&>>)
            ? std::ranges::subrange_kind::sized
            : std::ranges::subrange_kind::unsized>;

    LaborRequest(const AgentID Id, const Wage Wage) : firmID{Id}, wage{Wage} {}
    auto entry(const AgentID id, const double productPower) -> LaborEntry& {
        return *entryBox_.emplace_back(id, productPower, *this);
    }
    auto entryBox() -> EntryBoxT { return std::ranges::subrange{entryBox_}; }

    const AgentID firmID;
    const Wage    wage;

  private:
    tbb::concurrent_vector<LaborEntry> entryBox_;
};

class [[nodiscard]] LaborMarket {
  public:
    using RequestBoxT = std::ranges::subrange<
        std::ranges::iterator_t<tbb::detail::d1::concurrent_vector<
            abm::LaborRequest,
            tbb::detail::d1::cache_aligned_allocator<abm::LaborRequest>>&>,
        std::ranges::sentinel_t<tbb::detail::d1::concurrent_vector<
            abm::LaborRequest,
            tbb::detail::d1::cache_aligned_allocator<abm::LaborRequest>>&>,
        (std::ranges::sized_range<tbb::detail::d1::concurrent_vector<abm::LaborRequest>&> ||
         std::sized_sentinel_for<
             std::ranges::sentinel_t<tbb::detail::d1::concurrent_vector<
                 abm::LaborRequest,
                 tbb::detail::d1::cache_aligned_allocator<abm::LaborRequest>>&>,
             std::ranges::iterator_t<tbb::detail::d1::concurrent_vector<
                 abm::LaborRequest,
                 tbb::detail::d1::cache_aligned_allocator<abm::LaborRequest>>&>>)
            ? std::ranges::subrange_kind::sized
            : std::ranges::subrange_kind::unsized>;

    LaborMarket() = default;
    auto requestBox() -> RequestBoxT { return std::ranges::subrange{requestBox_}; }
    auto request(const AgentID id, const Wage wage) -> LaborRequest& {
        return *requestBox_.emplace_back(id, wage);
    }
    void clear() { requestBox_.clear(); }

  private:
    tbb::concurrent_vector<LaborRequest> requestBox_;
};

struct ConsumerGoodsRequest {
    const GoodsQuantity amount;
    GoodsQuantity       tradeAmount{0.0};

    const ConsumerGoodsEntry& entry;
    ConsumerGoodsRequest(const GoodsQuantity Amount, const ConsumerGoodsEntry& Entry)
        : amount{Amount}, entry{Entry} {}
};

struct [[nodiscard]] ConsumerGoodsEntry {
    ConsumerGoodsEntry(const Price Price, const GoodsQuantity Supply)
        : price{Price}, supply{Supply} {}
    auto request(const GoodsQuantity amount) -> ConsumerGoodsRequest& {
        return *requestBox.emplace_back(amount, *this);
    }

    const Price                                  price;
    const GoodsQuantity                          supply;
    tbb::concurrent_vector<ConsumerGoodsRequest> requestBox;
};

struct ProductionGoodsRequest {
    const GoodsQuantity amount;
    GoodsQuantity       tradeAmount{0.0};

    const ProductionGoodsEntry& entry;
    ProductionGoodsRequest(const GoodsQuantity Amount, const ProductionGoodsEntry& Entry)
        : amount{Amount}, entry{Entry} {}
};

struct [[nodiscard]] ProductionGoodsEntry {
    ProductionGoodsEntry(const AgentID Id, const Price Price, const GoodsQuantity Supply)
        : id{Id}, price{Price}, supply{Supply} {}
    auto request(const GoodsQuantity amount) -> ProductionGoodsRequest& {
        return *requestBox.emplace_back(amount, *this);
    }

    const AgentID                                  id;
    const Price                                    price;
    const GoodsQuantity                            supply;
    tbb::concurrent_vector<ProductionGoodsRequest> requestBox;
};

struct CensusDropBox {
    std::vector<double> firmAssets;
    std::vector<double> postedEmployments;
    std::vector<double> employments;
    std::vector<double> prices;
    std::vector<double> supplies;
    std::vector<double> markups;
    std::vector<double> inventories;

    std::vector<double> hholdAssets;
    std::vector<double> wages;

    CensusDropBox() {
        constexpr std::size_t firmCnt{static_cast<std::size_t>(config::agent_count::BtoCFirm)};
        constexpr std::size_t hholdCnt{static_cast<std::size_t>(config::agent_count::hhold)};
        firmAssets.reserve(firmCnt);
        postedEmployments.reserve(firmCnt);
        employments.reserve(firmCnt);
        prices.reserve(firmCnt);
        supplies.reserve(firmCnt);
        markups.reserve(firmCnt);
        inventories.reserve(firmCnt);

        hholdAssets.reserve(hholdCnt);
        wages.reserve(hholdCnt);
    }

    void clear() {
        firmAssets.clear();
        postedEmployments.clear();
        employments.clear();
        prices.clear();
        supplies.clear();
        markups.clear();
        inventories.clear();

        hholdAssets.clear();
        wages.clear();
    }
};
}  // namespace abm