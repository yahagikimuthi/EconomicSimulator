// pcg32_constexpr.cpp
// コンパイル時（constexpr）に計算可能な PCG32 乱数生成器
// C++17 以降でビルド可能 (C++14 でも rotr を再帰にすれば動作します)
//
// 参考: PCG (Permuted Congruential Generator) by M.E. O'Neill
//       https://www.pcg-random.org/

#include <array>
#include <cstdint>
#include <ranges>

namespace helper {

class Pcg32 {
  public:
    using result_type = std::uint32_t;

    // デフォルトの乗数・初期増分（PCG のリファレンス実装と同じ値）
    static constexpr std::uint64_t default_multiplier{6364136223846793005ULL};

    // state: 内部状態 (64bit)
    // inc  : 増分（ストリーム選択用の奇数値）
    constexpr Pcg32() : state_{0x853c49e6748fea9bULL}, inc_{0xda3e39cb94b95bdbULL} {}

    // seed と stream(=シーケンス選択)を指定して初期化
    constexpr Pcg32(std::uint64_t seed, std::uint64_t stream = 1)
        : state_{}, inc_((stream << 1U) | 1U) {
        step();
        state_ += seed;
        step();
    }

    // 32bit 乱数を1つ生成（呼び出すたびに内部状態が進む）
    constexpr auto next() noexcept -> result_type {
        const std::uint64_t old_state = state_;
        step();
        const std::uint32_t xorshifted{
            static_cast<std::uint32_t>(((old_state >> 18U) ^ old_state) >> 27U)
        };
        const std::uint32_t rot{static_cast<std::uint32_t>(old_state >> 59U)};
        return rotr32(xorshifted, rot);
    }

    // [0, bound) の範囲の乱数（バイアス除去つき、Lemire 法）
    constexpr auto next_bounded(result_type bound) -> result_type {
        // bound が 0 の場合は全範囲を返す
        if (bound == 0) return next();
        std::uint32_t threshold = (~bound + 1U) % bound;  // = (2^32 - bound) % bound
        for (;;) {
            std::uint32_t r = next();
            if (r >= threshold) return r % bound;
        }
    }

    // [0.0, 1.0) の範囲の浮動小数点数
    constexpr auto next_double() noexcept -> double {
        // 上位32bitを使い 2^32 で割る
        return static_cast<double>(next()) / 4294967296.0;  // 2^32
    }

    constexpr auto state() const noexcept -> std::uint64_t { return state_; }
    constexpr auto inc() const noexcept -> std::uint64_t { return inc_; }

  private:
    std::uint64_t state_;
    std::uint64_t inc_;

    constexpr void step() noexcept { state_ = (state_ * default_multiplier) + inc_; }

    // 32bit 右回転（constexpr 対応）
    static constexpr std::uint32_t rotr32(std::uint32_t v, std::uint32_t rot) noexcept {
        return (v >> rot) | (v << ((~rot + 1U) & 31U));
    }
};

// ---- コンパイル時に N 個の乱数列を生成するヘルパ ----
template <std::size_t N>
struct RandomArray {
    std::array<std::uint32_t, N> data{};

    constexpr RandomArray(std::uint64_t seed, std::uint64_t stream = 1) {
        Pcg32 rng(seed, stream);
        for (const auto i : std::views::iota(0UZ, N)) {
            data[i] = rng.next();
        }
    }
};

// =====================================================
// 使用例
// =====================================================

// (1) コンパイル時に1つの乱数を計算し、static_assert で検証できる
constexpr std::uint32_t compile_time_value = [] {
    Pcg32 rng(42);      // seed = 42
    rng.next();         // 1つ捨てる（例）
    return rng.next();  // これが結果
}();

// (2) コンパイル時に配列全体を生成（例: シェーダ用ノイズテーブルなど）
constexpr RandomArray<8> table(1234);

// (3) [0, 100) の範囲の乱数をコンパイル時に生成
constexpr std::uint32_t compile_time_bounded = [] {
    Pcg32 rng(2024);
    return rng.next_bounded(100);
}();
}  // namespace helper