#include "wanikani_scheduler.h"
#include <cmath>

namespace
{
	int CalculateNextLevel(const int level, const int incorrect_count)
	{
		// WaniKani SRS formula:
		// new_srs_stage = current_srs_stage - (incorrect_adjustment_count * srs_penalty_factor)
		// incorrect_adjustment_count = ceil(incorrect_count / 2)
		// srs_penalty_factor = 2 if current_srs_stage >= 5, otherwise 1

		int incorrect_adjustment_count = static_cast<int>(std::ceil(incorrect_count / 2.0));
		int srs_penalty_factor = (level >= 5) ? 2 : 1;
		int new_level = level - incorrect_adjustment_count * srs_penalty_factor;

		// Ensure level doesn't go below 1
		return std::max(1, new_level);
	}

	std::chrono::system_clock::time_point CalculateNextReviewDate(int level)
	{
		using namespace std::chrono;
		auto now = system_clock::now();

		// WaniKani SRS intervals:
		// Level 1 (Apprentice 1) → 4 hours
		// Level 2 (Apprentice 2) → 8 hours
		// Level 3 (Apprentice 3) → 1 day
		// Level 4 (Apprentice 4) → 2 days
		// Level 5 (Guru 1) → 1 week
		// Level 6 (Guru 2) → 2 weeks
		// Level 7 (Master) → 1 month
		// Level 8 (Enlightened) → 4 months
		// Level 9+ (Burned) → no more reviews

		switch (level)
		{
			case 0:
			case 1:
				return now + hours(4);
			case 2:
				return now + hours(8);
			case 3:
				return now + hours(24);
			case 4:
				return now + hours(48);
			case 5:
				return now + hours(24 * 7); // 1 week
			case 6:
				return now + hours(24 * 14); // 2 weeks
			case 7:
				return now + hours(24 * 30); // 1 month (approximation)
			case 8:
				return now + hours(24 * 120); // 4 months (approximation)
			default:
				// Level 9+ is "Burned" - set far future date to indicate no more reviews
				return now + hours(24 * 365 * 10); // 10 years
		}
	}

} // namespace

namespace kanji::scheduler
{
	KanjiReviewState WaniKaniScheduler::GetNextState(const KanjiReviewState& old_state, const int incorrect_streak) const
	{
		KanjiReviewState new_state = old_state;
		if (incorrect_streak <= 0)
		{
			++new_state.level;
		}
		else
		{
			new_state.level = CalculateNextLevel(old_state.level, incorrect_streak);
		}
		new_state.next_review_date = CalculateNextReviewDate(new_state.level);
		return new_state;
	}

} // namespace kanji::scheduler
