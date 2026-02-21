#include "controller.h"
#include "database/database_context.h"
#include "resources.h"
#include "scheduler/wanikani_scheduler.h"
#include "system/platform_info.h"
#include "system/resource.h"
#include "wallpaper/wallpaper_service.h"
#include <crow.h>
#include <crow/middlewares/cors.h>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>
#include <string>

int main()
{
	auto scheduler = std::make_unique<kanji::scheduler::WaniKaniScheduler>();
	kanji::database::DatabaseContext context{kanji::system::PlatformInfo::GetDatabaseLocation()};
	// kanji::wallpaper::WallpaperService wallpaper_service{context.GetReviewStateRepository()};
	kanji::Controller controller{context, std::move(scheduler)};

	crow::App<crow::CORSHandler> app;

	auto& cors = app.get_middleware<crow::CORSHandler>();
	cors.global()
	    .origin("http://localhost:5173")
	    .methods("GET"_method, "POST"_method)
	    .headers("Content-Type");

	std::mutex controller_mutex;

	CROW_ROUTE(app, "/api/kanjis").methods("GET"_method)([&]() {
		std::lock_guard lock(controller_mutex);
		nlohmann::json j = controller.GetReviewKanjis();
		auto res = crow::response(j.dump());
		res.set_header("Content-Type", "application/json");
		return res;
	});

	CROW_ROUTE(app, "/api/answers").methods("POST"_method)([&](const crow::request& req) {
		std::lock_guard lock(controller_mutex);
		auto j = nlohmann::json::parse(req.body);
		controller.SetAnswers(j["answers"]);
		return crow::response(200);
	});

	CROW_ROUTE(app, "/api/learn-more").methods("POST"_method)([&]() {
		std::lock_guard lock(controller_mutex);
		controller.LearnMoreKanjis();
		return crow::response(200);
	});

	CROW_ROUTE(app, "/")([](const crow::request&, crow::response& res) {
		res.set_static_file_info("assets/index.html");
		res.end();
	});

	CROW_ROUTE(app, "/<path>")([](const crow::request&, crow::response& res, const std::string& path) {
		res.set_static_file_info("assets/" + path);
		res.end();
	});

	app.port(8080).run();

	return 0;
}
