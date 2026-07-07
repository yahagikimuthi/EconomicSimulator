#include <entt/entt.hpp>
#include <iostream>

// コンポーネントの定義
struct Position {
    double x;
    double y;
};

struct Velocity {
    double dx;
    double dy;
};

auto main() -> int {
    entt::registry registry;

    // エンティティの作成とコンポーネントの付与
    auto entity = registry.create();
    registry.emplace<Position>(entity, 0.0, 0.0);
    registry.emplace<Velocity>(entity, 1.0, 2.0);

    // システム（処理）の実行
    auto view = registry.view<Position, const Velocity>();

    for (auto [ent, pos, vel] : view.each()) {
        pos.x += vel.dx;
        pos.y += vel.dy;

        std::cout << "Entity moved to: (" << pos.x << ", " << pos.y << ")\n";
    }
}