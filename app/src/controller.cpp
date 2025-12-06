#include "controller.h"
#include "database/database_context.h"
#include "kanji.h"
#include "scheduler/scheduler.h"

namespace kanji
{
	Controller::Controller(database::DatabaseContext& in_db, std::unique_ptr<scheduler::IScheduler> in_scheduler)
	    : db{in_db}
	    , scheduler{std::move(in_scheduler)}
	{
	}

	std::vector<KanjiData> Controller::GetReviewKanjis()
	{
		return db
		    .GetKanjiRepository()
		    .GetKanjiForReview();
	}

	void Controller::SetAnswers(const std::vector<KanjiAnswer>& in_answers)
	{
		auto& review_repo = db.GetReviewStateRepository();
		std::vector<std::uint32_t> ids;
		ids.reserve(in_answers.size());

		std::transform(in_answers.begin(), in_answers.end(), std::back_inserter(ids), [](const KanjiAnswer& answer) {
			return answer.kanji_id;
		});

		const std::vector<KanjiReviewState>& old_review_states = review_repo.GetReviewStates(ids);
		for (int i = 0; i < in_answers.size(); ++i)
		{
			const auto& old_state = old_review_states[i];
			const auto& new_state = scheduler->GetNextState(old_state, in_answers[i].incorrect_streak);
			review_repo.CreateOrUpdateReviewState(new_state);
		}
	}

	void Controller::LearnMoreKanjis()
	{
		constexpr int MAX_REVIEWS_REQUEST = 10;

		auto& review_repo = db.GetReviewStateRepository();
		review_repo.InitializeNewReviewStates(MAX_REVIEWS_REQUEST);
	}
} // namespace kanji
