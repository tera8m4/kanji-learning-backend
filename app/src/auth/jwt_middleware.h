#pragma once

#include <crow.h>
#include <memory>

namespace kanji::auth
{
	class AuthService;

	struct JwtMiddleware
	{
		struct context
		{
		};

		void before_handle(crow::request& req, crow::response& res, context&);
		void after_handle(crow::request& req, crow::response& res, context&);

		std::shared_ptr<AuthService> auth_service;
	};
} // namespace kanji::auth
