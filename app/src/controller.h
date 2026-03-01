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

	namespace wallpaper
	{
		class WallpaperService;
	}

	class Controller
	{
	public:
		explicit Controller(database::DatabaseContext& in_db, std::unique_ptr<scheduler::IScheduler> in_scheduler);
		ReviewsResponse GetReviews();
		void SetAnswers(const std::vector<KanjiAnswer>& in_answers);
		void LearnMoreKanjis();
		void BatchAddKanjis(const std::vector<KanjiData>& kanjis);
		std::vector<KanjiRecord> GetKanjis();

	private:
		database::DatabaseContext& db;
		std::unique_ptr<scheduler::IScheduler> scheduler;
	};
} // namespace kanji
