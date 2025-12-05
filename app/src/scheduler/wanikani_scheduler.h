#pragma once

#include "scheduler.h"
#include <chrono>

namespace kanji::scheduler
{
	class WaniKaniScheduler : public IScheduler
	{
	public:
		virtual KanjiReviewState GetNextState(const KanjiReviewState& old_state, int incorrect_streak) const override;
	};
} // namespace kanji::scheduler
