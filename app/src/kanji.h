#pragma once

#include <chrono>
#include <nlohmann/json.hpp>
#include <string>
#include <vector>

namespace kanji
{
	struct KanjiWord
	{
		std::string word;
		std::string reading;
	};

	struct KanjiData
	{
		std::uint32_t id;
		std::string kanji;
		std::string meaning;
		std::vector<KanjiWord> examples;
	};

	struct KanjiReviewState
	{
		std::uint32_t kanji_id;
		int level;
		std::chrono::system_clock::time_point next_review_date;
		std::chrono::system_clock::time_point created_at;
	};

	struct KanjiAnswer
	{
		std::uint32_t kanji_id;
		int incorrect_streak;
	};

	struct KanjiRecord
	{
		std::uint32_t id;
		std::string kanji;
		std::string meaning;
		int level;
		std::int64_t next_review_date;
	};

	struct ReviewsResponse
	{
		std::vector<KanjiData> kanjis;
		int total;
	};

	NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(KanjiWord, word, reading)
	NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(KanjiData, id, kanji, examples, meaning)
	NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(KanjiAnswer, kanji_id, incorrect_streak)
	NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(KanjiRecord, id, kanji, meaning, level, next_review_date)
	NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(ReviewsResponse, kanjis, total)

}; // namespace kanji
