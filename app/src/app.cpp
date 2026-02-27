#include "app.h"
#include "auth/telegram_auth.h"
#include "notification/telegram_notification_service.h"
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>

namespace kanji
{

	KanjiApp::KanjiApp(const config::KanjiAppConfig& in_config)
	    : config{in_config}
	    , db{system::PlatformInfo::GetDatabaseLocation()}
	    , controller{db, std::make_unique<scheduler::WaniKaniScheduler>()}
	    , auth_service{std::make_shared<auth::AuthService>(config.auth, config.notification.telegram.chat_id)}
	{
		using TelegramService = notification::TelegramNotificationService;
		auto telegram_service = std::make_unique<TelegramService>(config.notification.telegram);
		const auto interval = std::chrono::minutes{config.notification.refresh_interval};
		notifier = std::make_unique<notification::ReviewNotifier>(db, std::move(telegram_service), interval);
		notifier->Start();

		SetupMiddlewares();
		RegisterRoutes();
	}

	void KanjiApp::Run()
	{
		app.bindaddr("127.0.0.1").port(8080).run();
	}

	void KanjiApp::SetupMiddlewares()
	{
		auto& cors = app.get_middleware<crow::CORSHandler>();
		cors.global()
		    .origin("http://localhost:5173")
		    .methods("GET"_method, "POST"_method)
		    .headers("Content-Type", "Authorization");

		app.get_middleware<auth::JwtMiddleware>().auth_service = auth_service;
	}

	void KanjiApp::RegisterRoutes()
	{
		CROW_ROUTE(app, "/api/login").methods("POST"_method)([&](const crow::request& req) {
			auto j = nlohmann::json::parse(req.body);
			auth::TelegramAuthData data{
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

			if (!auth::VerifyTelegramAuth(data, config.notification.telegram.bot_token))
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
	}

} // namespace kanji
