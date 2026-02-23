#pragma once

#include <cstdint>
#include <string>

namespace kanji::auth
{
	struct TelegramAuthData
	{
		int id;
		std::string first_name;
		std::string last_name;   // optional
		std::string username;
		std::string photo_url;   // optional
		std::int64_t auth_date;
		std::string hash;

		std::string GetDataCheckString() const;
	};

	bool VerifyTelegramAuth(const TelegramAuthData& data, const std::string& bot_token);

} // namespace kanji::auth
