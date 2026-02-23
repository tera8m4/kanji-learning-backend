#include "telegram_auth.h"
#include "utils/crypto.h"
#include <format>
#include <spdlog/spdlog.h>

namespace kanji::auth
{
	std::string TelegramAuthData::GetDataCheckString() const
	{
		std::string s = std::format("auth_date={}\nfirst_name={}\nid={}", auth_date, first_name, id);
		if (!last_name.empty())
			s += std::format("\nlast_name={}", last_name);
		if (!photo_url.empty())
			s += std::format("\nphoto_url={}", photo_url);
		s += std::format("\nusername={}", username);
		return s;
	}

	bool VerifyTelegramAuth(const TelegramAuthData& data, const std::string& bot_token)
	{
		// Derive secret key: SHA256(bot_token)
		namespace crypto = kanji::utils::crypto;
		auto secret_key = crypto::SHA256(bot_token);

		const auto data_check_string = data.GetDataCheckString();
		const auto computed = crypto::HMAC_SHA256(data_check_string, secret_key).ToLowerCase();

		spdlog::debug("Telegram auth: bot_token length={}, first4={}...", bot_token.size(), bot_token.substr(0, 4));
		spdlog::debug("Telegram auth: secret_key(sha256)={}", secret_key.ToLowerCase());
		spdlog::debug("Telegram auth: id={}, data_check_string={}", data.id, data_check_string);
		spdlog::debug("Telegram auth: computed={}, received={}", computed, data.hash);

		const bool ok = computed == data.hash;
		if (!ok)
		{
			spdlog::warn("Telegram auth verification failed for id={}: HMAC mismatch", data.id);
		}
		return ok;
	}
} // namespace kanji::auth
