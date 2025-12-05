#include "controller.h"
#include "database/database_context.h"
#include "kanji.h"

namespace kanji
{
	Controller::Controller(database::DatabaseContext& in_db)
	    : db{in_db}
	{
	}

	std::vector<KanjiData> Controller::GetReviewKanjis()
	{
		return db
		    .GetKanjiRepository()
		    .GetKanjiForReview();
	}

	void Controller::SetAnswers(const std::vector<std::pair<int, bool>>& in_answers)
	{
	}
} // namespace kanji
