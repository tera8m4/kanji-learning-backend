#pragma once

#include <string>
#include <string_view>
#include <vector>

namespace kanji::utils::crypto
{
	struct Hash
	{
		std::vector<unsigned char> value;

		std::string ToLowerCase() const;
	};

	Hash SHA256(std::string_view view);
	Hash HMAC_SHA256(std::string_view data_check_string, const Hash& secret_key);
} // namespace kanji::utils::crypto
