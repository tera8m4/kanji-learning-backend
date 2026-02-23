#include "auth_service.h"
#include <jwt-cpp/traits/nlohmann-json/defaults.h>

namespace kanji::auth
{

	AuthService::AuthService(const kanji::config::AuthSettings& in_auth_settings, int in_allowed_user_id)
	    : auth_settings{in_auth_settings}, allowed_user_id{in_allowed_user_id}
	{
	}

	std::string AuthService::GenerateToken(int telegram_id) const
	{
		using namespace std::chrono;
		const auto now = system_clock::now();
		const auto exp = now + hours{auth_settings.token_expiry_hours};

		return jwt::create()
		    .set_subject(std::to_string(telegram_id))
		    .set_issued_at(now)
		    .set_expires_at(exp)
		    .sign(jwt::algorithm::hs256{auth_settings.jwt_secret});
	}

	std::string AuthService::ValidateToken(std::string_view token) const
	{
		const auto verifier = jwt::verify()
		                          .allow_algorithm(jwt::algorithm::hs256{auth_settings.jwt_secret})
		                          .with_claim("sub", jwt::claim(std::to_string(allowed_user_id)));

		auto decoded = jwt::decode(std::string{token});
		verifier.verify(decoded);
		return decoded.get_subject();
	}

} // namespace kanji::auth
