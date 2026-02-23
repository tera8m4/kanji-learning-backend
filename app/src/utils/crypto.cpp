#include "crypto.h"
#include <format>
#include <openssl/evp.h>
#include <openssl/hmac.h>

namespace kanji::utils::crypto
{

	std::string Hash::ToLowerCase() const
	{
		std::string computed_hash;
		computed_hash.reserve(value.size() * 2);
		for (auto c : value)
		{
			computed_hash += std::format("{:02x}", c);
		}
		return computed_hash;
	}

	Hash SHA256(std::string_view view)
	{
		Hash hash;
		hash.value.resize(EVP_MAX_MD_SIZE);

		unsigned int length = 0;
		EVP_MD_CTX* sha_ctx = EVP_MD_CTX_new();
		EVP_DigestInit_ex(sha_ctx, EVP_sha256(), nullptr);
		EVP_DigestUpdate(sha_ctx, view.data(), view.size());
		EVP_DigestFinal_ex(sha_ctx, hash.value.data(), &length);
		EVP_MD_CTX_free(sha_ctx);

		hash.value.resize(length);

		return hash;
	}

	Hash HMAC_SHA256(std::string_view check_string, const Hash& secret_key)
	{
		Hash hmac;
		hmac.value.resize(EVP_MAX_MD_SIZE);

		unsigned int hmac_len = 0;
		HMAC(EVP_sha256(),
		     secret_key.value.data(), static_cast<int>(secret_key.value.size()),
		     reinterpret_cast<const unsigned char*>(check_string.data()), check_string.size(),
		     hmac.value.data(), &hmac_len);

		hmac.value.resize(hmac_len);

		return hmac;
	}
} // namespace kanji::utils::crypto
