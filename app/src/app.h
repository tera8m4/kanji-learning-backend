#pragma once

#include "auth/auth_service.h"
#include "auth/jwt_middleware.h"
#include "config.h"
#include "controller.h"
#include "database/database_context.h"
#include "notification/review_notifier.h"
#include "scheduler/wanikani_scheduler.h"
#include "system/platform_info.h"
#include <crow.h>
#include <crow/middlewares/cors.h>
#include <memory>
#include <mutex>

namespace kanji
{
	class KanjiApp
	{
	public:
		explicit KanjiApp(const config::KanjiAppConfig& in_config);
		void Run();

	private:
		void SetupMiddlewares();
		void RegisterRoutes();

		const config::KanjiAppConfig& config;
		database::DatabaseContext db;
		Controller controller;
		std::unique_ptr<notification::ReviewNotifier> notifier;
		std::shared_ptr<auth::AuthService> auth_service;
		crow::App<crow::CORSHandler, auth::JwtMiddleware> app;
		std::mutex controller_mutex;
	};
} // namespace kanji
