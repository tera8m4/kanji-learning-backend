#pragma once

#include "kanji.h"
#include <vector>

namespace kanji
{
	namespace database
	{
		class DatabaseContext;
	}

	namespace scheduler
	{
		class IScheduler;
	}

	class Controller
	{
	public:
		explicit Controller(database::DatabaseContext& in_db, std::unique_ptr<scheduler::IScheduler> in_scheduler);
		std::vector<KanjiData> GetReviewKanjis();
		void SetAnswers(const std::vector<KanjiAnswer>& in_answers);
		void LearnMoreKanjis();

	private:
		database::DatabaseContext& db;
		std::unique_ptr<scheduler::IScheduler> scheduler;
	};
} // namespace kanji
