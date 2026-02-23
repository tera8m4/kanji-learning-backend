#include "auth/auth_service.h"
#include "auth/jwt_middleware.h"
#include "auth/telegram_auth.h"
#include "config.h"
#include "controller.h"
#include "database/database_context.h"
#include "notification/review_notifier.h"
#include "notification/telegram_notification_service.h"
#include "scheduler/wanikani_scheduler.h"
#include "system/platform_info.h"
#include "system/resource.h"
#include <crow.h>
#include <crow/middlewares/cors.h>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>
#include <string>

int main()
{
	auto scheduler = std::make_unique<kanji::scheduler::WaniKaniScheduler>();
	kanji::database::DatabaseContext context{kanji::system::PlatformInfo::GetDatabaseLocation()};
	kanji::Controller controller{context, std::move(scheduler)};
	std::unique_ptr<kanji::notification::ReviewNotifier> notifier;
	kanji::config::KanjiAppConfig config;

	const std::filesystem::path config_path{"config.json"};
	if (std::filesystem::exists(config_path))
	{
		using TelgramService = kanji::notification::TelegramNotificationService;
		config = kanji::config::KanjiAppConfig::LoadFromFile(config_path);
		auto telegram_service = std::make_unique<TelgramService>(config.notification.telegram);
		const auto interval = std::chrono::minutes{config.notification.refresh_interval};
		notifier = std::make_unique<kanji::notification::ReviewNotifier>(context, std::move(telegram_service), interval);
		notifier->Start();
	}
	else
	{
		spdlog::warn("No config.json found — notifications disabled");
		return 1;
	}

	auto auth_service = std::make_shared<kanji::auth::AuthService>(config.auth, config.notification.telegram.chat_id);
	crow::App<crow::CORSHandler, kanji::auth::JwtMiddleware> app;

	auto& cors = app.get_middleware<crow::CORSHandler>();
	cors.global()
	    .origin("http://localhost:5173")
	    .methods("GET"_method, "POST"_method)
	    .headers("Content-Type", "Authorization");

	app.get_middleware<kanji::auth::JwtMiddleware>().auth_service = auth_service;

	std::mutex controller_mutex;

	CROW_ROUTE(app, "/api/login").methods("POST"_method)([&](const crow::request& req) {
		auto j = nlohmann::json::parse(req.body);
		kanji::auth::TelegramAuthData data{
		    .id = j["id"].get<int>(),
		    .first_name = j["first_name"],
		    .last_name = j.value("last_name", ""),
		    .username = j["username"],
		    .photo_url = j.value("photo_url", ""),
		    .auth_date = j["auth_date"].get<int64_t>(),
		    .hash = j["hash"],
		};

		spdlog::debug("Login request body: {}", req.body);
		spdlog::info("Login attempt: id={}, username={}", data.id, data.username);

		if (!kanji::auth::VerifyTelegramAuth(data, config.notification.telegram.bot_token))
		{
			spdlog::warn("Login rejected (401) for id={}", data.id);
			return crow::response(401);
		}

		auto token = auth_service->GenerateToken(data.id);
		nlohmann::json resp = {{"token", token}};
		auto res = crow::response(resp.dump());
		res.set_header("Content-Type", "application/json");
		return res;
	});

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

	app.bindaddr("127.0.0.1").port(8080).run();

	return 0;
}
