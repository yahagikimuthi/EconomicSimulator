#include <cstdint>
#include <random>

#include "config.hpp"

namespace helper {

class Pcg32 {
  public:
    using result_type = std::uint32_t;

    // デフォルトの乗数・初期増分（PCG のリファレンス実装と同じ値）
    static constexpr std::uint64_t default_multiplier{6364136223846793005ULL};

    // seed と stream(=シーケンス選択)を指定して初期化
    constexpr Pcg32(std::uint64_t seed, std::uint64_t stream = 1) : inc_((stream << 1U) | 1U) {
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
    std::uint64_t state_{};
    std::uint64_t inc_;

    constexpr void step() noexcept { state_ = (state_ * default_multiplier) + inc_; }

    // 32bit 右回転（constexpr 対応）
    static constexpr auto rotr32(std::uint32_t v, std::uint32_t rot) noexcept -> std::uint32_t {
        return (v >> rot) | (v << ((~rot + 1U) & 31U));
    }
};

[[nodiscard]] auto generatePcg32() -> Pcg32 {
    if (not config::setting::useRuntimeRandomSeed) {
        return {config::setting::fixedSeedState, config::setting::fixedSeedInc};
    }

    std::random_device  rd;
    const std::uint64_t stateHigh{rd()};
    const std::uint64_t stateLow{rd()};
    const std::uint64_t state{stateHigh << 32 | stateLow};

    const std::uint64_t incHigh{rd()};
    const std::uint64_t incLow{rd()};
    const std::uint64_t inc{(incHigh << 32 | incLow) | 1ULL};

    return {state, inc};
}
}  // namespace helper