#pragma once

#include "kanji.h"

namespace kanji::scheduler
{
	class IScheduler
	{
	public:
		virtual KanjiReviewState GetNextState(const KanjiReviewState& old_state, const int incorrect_streak) const = 0;
		virtual ~IScheduler() = default;
	};
} // namespace kanji::scheduler
