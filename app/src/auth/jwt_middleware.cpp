#include "jwt_middleware.h"
#include "auth_service.h"
#include <spdlog/spdlog.h>

namespace kanji::auth
{
	void JwtMiddleware::before_handle(crow::request& req, crow::response& res, context&)
	{
		static constexpr std::string_view bearer_prefix = "Bearer ";

		if (req.url == "/" || req.url == "/api/login")
		{
			return;
		}

		const auto auth_header = req.get_header_value("Authorization");
		if (auth_header.empty() || auth_header.substr(0, bearer_prefix.size()) != bearer_prefix)
		{
			spdlog::warn("JWT middleware: missing/invalid Authorization header for {}", req.url);
			res.code = 401;
			res.end();
			return;
		}

		try
		{
			auth_service->ValidateToken(auth_header.substr(bearer_prefix.size()));
		}
		catch (const std::exception& e)
		{
			spdlog::warn("JWT middleware: token validation failed for {}: {}", req.url, e.what());
			res.code = 401;
			res.end();
		}
	}

	void JwtMiddleware::after_handle(crow::request& req, crow::response& res, context&)
	{
	}

} // namespace kanji::auth
