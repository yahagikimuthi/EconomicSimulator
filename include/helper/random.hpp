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
        const std::uint64_t old_state{state_};
        step();
        const std::uint32_t xorshifted{
            static_cast<std::uint32_t>(((old_state >> 18U) ^ old_state) >> 27U)
        };
        const std::uint32_t rot{static_cast<std::uint32_t>(old_state >> 59U)};
        return rotr32(xorshifted, rot);
    }

    static constexpr auto min() noexcept -> result_type { return 0U; }
    static constexpr auto max() noexcept -> result_type { return 0xFFFFFFFFU; }

    // 一様な生の32bit乱数を返す（next() と同じ）
    constexpr auto operator()() noexcept -> result_type { return next(); }

  private:
    std::uint64_t state_;
    std::uint64_t inc_;

    constexpr void step() noexcept { state_ = (state_ * default_multiplier) + inc_; }

    // 32bit 右回転（constexpr 対応）
    static constexpr auto rotr32(std::uint32_t v, std::uint32_t rot) noexcept -> std::uint32_t {
        return (v >> rot) | (v << ((~rot + 1U) & 31U));
    }
};
}  // namespace helper