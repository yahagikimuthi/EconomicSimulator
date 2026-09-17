#include <crow.h>

auto main() -> int {
    crow::SimpleApp app;

    CROW_ROUTE(app, "api/simulate")
        .methods(crow::HTTPMethod::POST)([](const crow::request& req) noexcept -> auto {
            auto body = crow::json::load(req.body);
            if (!body) return crow::response(400, "Invalid Json");

            crow::json::wvalue res;
            res["status"] = "success";
            res["gdp"]    = 1000.0;

            crow::response response(res);
            response.add_header("Access-Control-Allow-Origin", "*");
            return response;
        });

    app.port(18080).multithreaded().run();
}