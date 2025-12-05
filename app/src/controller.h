#pragma once

#include "kanji.h"
#include <vector>

namespace kanji
{
	namespace database
	{
		class DatabaseContext;
	}

	class Controller
	{
	public:
		explicit Controller(database::DatabaseContext& in_db);
		std::vector<KanjiData> GetReviewKanjis();
		void SetAnswers(const std::vector<std::pair<int, bool>>& in_answers);

	private:
		database::DatabaseContext& db;
	};
} // namespace kanji
