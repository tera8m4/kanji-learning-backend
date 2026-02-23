#pragma once

#include "config.h"
#include <string>

namespace kanji::auth
{
	class AuthService
	{
	public:
		explicit AuthService(const kanji::config::AuthSettings& in_auth_settings, int in_allowed_user_id);

		std::string GenerateToken(int telegram_id) const;
		std::string ValidateToken(std::string_view token) const;

	private:
		kanji::config::AuthSettings auth_settings;
		int allowed_user_id;
	};
} // namespace kanji::auth
