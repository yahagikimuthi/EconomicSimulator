#pragma once

#include <cassert>
#include <cmath>
#include <concepts>
#include <cstddef>
#include <vector>

#include "others/setting.hpp"

namespace abm::drop_box {
// 並列化する場合、tbb::concurrent_vectorにしなければならない
// しかし、HighFiveと互換性がないため、連続メモリコンテナで要素の追加を安全に行いたい
// 事前に容量を確保し、スレッドごとにインデックスを割り当てる方式を検討

class Vec final {
  public:
    explicit Vec() noexcept = default;

    void clear() noexcept { vec_.clear(); }

    void add(const double add) noexcept {
        assert(not std::isnan(add));
        vec_.emplace_back(add);
    }

    template <typename T>
        requires requires(T t) {
            { t.value() } -> std::same_as<double>;
        }
    void add(const T add) noexcept {
        assert(not std::isnan(add.value()));
        vec_.emplace_back(add.value());
    }

    void reserve(const std::size_t n) noexcept { vec_.reserve(n); }

    [[nodiscard]] auto get() const noexcept -> const std::vector<double>& { return vec_; }

  private:
    std::vector<double> vec_;
};

struct FinanceDropBox final {
    explicit FinanceDropBox() noexcept {
        namespace cnt = global_setting::agent_count;
        firmAssets.reserve(cnt::capitalFirm + cnt::goodsFirm);
        hholdAssets.reserve(cnt::hhold);
    }

    void clear() noexcept {
        firmAssets.clear();
        hholdAssets.clear();
    }

    Vec firmAssets;
    Vec netIncome;
    Vec hholdAssets;
};

struct LaborDropBox final {
    explicit LaborDropBox() noexcept {
        namespace cnt       = global_setting::agent_count;
        constexpr auto firm = cnt::capitalFirm + cnt::goodsFirm;
        postedEmployments.reserve(firm);
        postedWages.reserve(firm);
        personalCosts.reserve(firm);
        wages.reserve(cnt::hhold);
    }

    void clear() noexcept {
        postedEmployments.clear();
        postedWages.clear();
        employments.clear();
        personalCosts.clear();
        wages.clear();
    }

    Vec postedEmployments;
    Vec postedWages;
    Vec employments;
    Vec personalCosts;
    Vec wages;
};

struct BaseGoodsDropBox {
    explicit BaseGoodsDropBox(const std::size_t firm) noexcept {
        prices.reserve(firm);
        supplies.reserve(firm);
        markups.reserve(firm);
        inventories.reserve(firm);
    }

    void clear() noexcept {
        prices.clear();
        supplies.clear();
        markups.clear();
        inventories.clear();
    }

    Vec prices;
    Vec supplies;
    Vec markups;
    Vec inventories;
};

struct CapitalDropBox final : BaseGoodsDropBox {
    explicit CapitalDropBox() noexcept
        : BaseGoodsDropBox(global_setting::agent_count::capitalFirm) {}
};

struct GoodsDropBox final : BaseGoodsDropBox {
    explicit GoodsDropBox() noexcept : BaseGoodsDropBox(global_setting::agent_count::goodsFirm) {}
};

struct CensusDropBox final {
    explicit CensusDropBox() noexcept = default;

    void clear() noexcept {
        finance.clear();
        labor.clear();
        capital.clear();
        goods.clear();
    }

    FinanceDropBox finance;
    LaborDropBox   labor;
    CapitalDropBox capital;
    GoodsDropBox   goods;
};
}  // namespace abm::drop_box

namespace abm {

using FinanceDropBox = drop_box::FinanceDropBox;
using LaborDropBox   = drop_box::LaborDropBox;
using CapitalDropBox = drop_box::CapitalDropBox;
using GoodsDropBox   = drop_box::GoodsDropBox;
using CensusDropBox  = drop_box::CensusDropBox;

template <typename T>
concept BaseGoodsDropBox = std::derived_from<T, drop_box::BaseGoodsDropBox>;
}  // namespace abm