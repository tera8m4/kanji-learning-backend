#pragma once

#include "kanji.h"
#include <sqlite3.h>
#include <string>
#include <vector>
#include <optional>

namespace kanji
{
	class Database
	{
	public:
		Database(const std::string& db_path);
		~Database();

		Database(const Database&) = delete;
		Database& operator=(const Database&) = delete;

		bool Initialize();

		std::optional<KanjiData> GetKanjiById(std::uint32_t id);
		std::optional<KanjiReviewState> GetReviewState(std::uint32_t kanji_id);
		bool CreateOrUpdateReviewState(const KanjiReviewState& state);
		std::vector<KanjiData> GetKanjisForReview();
		bool InitializeNewReviewStates(int count);

	private:
		sqlite3* db_;
		std::string db_path_;

		bool CreateTables();
		std::vector<KanjiWord> GetKanjiWords(std::uint32_t kanji_id);
	};
} // namespace kanji
