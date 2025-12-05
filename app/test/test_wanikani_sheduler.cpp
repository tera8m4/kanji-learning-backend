#include "kanji.h"
#include "sheduler/wanikani_scheduler.h"
#include <catch2/catch_test_macros.hpp>

using namespace kanji;
using namespace kanji::scheduler;

TEST_CASE("WaniKani scheduler follows SRS progression", "[wanikani]")
{
	WaniKaniScheduler scheduler;

	// Start at level 1
	KanjiReviewState state;
	state.kanji_id = 1;
	state.level = 1;
	state.next_review_date = std::chrono::system_clock::now();

	// Correct (+1 stage, so it's now at SRS stage 2)
	state = scheduler.GetNextState(state, 0);
	REQUIRE(state.level == 2);

	// Correct (+1 stage, SRS stage 3)
	state = scheduler.GetNextState(state, 0);
	REQUIRE(state.level == 3);

	// Correct (+1 stage, SRS stage 4)
	state = scheduler.GetNextState(state, 0);
	REQUIRE(state.level == 4);

	// Incorrect once before getting it correct (-1 stage, SRS stage 3)
	state = scheduler.GetNextState(state, 1);
	REQUIRE(state.level == 3);

	// Correct (+1 stage, SRS stage 4)
	state = scheduler.GetNextState(state, 0);
	REQUIRE(state.level == 4);

	// Correct (+1 stage, SRS stage 5 GURU)
	state = scheduler.GetNextState(state, 0);
	REQUIRE(state.level == 5);

	// Correct (+1 stage, SRS stage 6)
	state = scheduler.GetNextState(state, 0);
	REQUIRE(state.level == 6);

	// Incorrect three times before getting it correct
	// 6 - (ceil(3/2) * 2) = 6 - (2 * 2) = 6 - 4 = 2
	state = scheduler.GetNextState(state, 3);
	REQUIRE(state.level == 2);
}

TEST_CASE("WaniKani scheduler penalty calculation", "[wanikani]")
{
	WaniKaniScheduler scheduler;

	SECTION("Penalty before Guru (level < 5)")
	{
		// At level 4, incorrect once: penalty factor = 1
		// 4 - (ceil(1/2) * 1) = 4 - 1 = 3
		KanjiReviewState state{1, 4, std::chrono::system_clock::now()};
		state = scheduler.GetNextState(state, 1);
		REQUIRE(state.level == 3);
	}

	SECTION("Penalty at Guru or higher (level >= 5)")
	{
		// At level 5, incorrect once: penalty factor = 2
		// 5 - (ceil(1/2) * 2) = 5 - 2 = 3
		KanjiReviewState state{1, 5, std::chrono::system_clock::now()};
		state = scheduler.GetNextState(state, 1);
		REQUIRE(state.level == 3);
	}

	SECTION("Multiple incorrect answers at high level")
	{
		// At level 6, incorrect 3 times: penalty factor = 2
		// 6 - (ceil(3/2) * 2) = 6 - 4 = 2
		KanjiReviewState state{1, 6, std::chrono::system_clock::now()};
		state = scheduler.GetNextState(state, 3);
		REQUIRE(state.level == 2);
	}

	SECTION("Level never goes below 1")
	{
		KanjiReviewState state{1, 1, std::chrono::system_clock::now()};
		state = scheduler.GetNextState(state, 10);
		REQUIRE(state.level >= 1);
	}
}

TEST_CASE("WaniKani scheduler review intervals", "[wanikani]")
{
	WaniKaniScheduler scheduler;
	auto now = std::chrono::system_clock::now();

	SECTION("Level 0/1 -> 4 hours")
	{
		KanjiReviewState state{1, 0, now};
		state = scheduler.GetNextState(state, 0);
		auto diff = std::chrono::duration_cast<std::chrono::hours>(state.next_review_date - now);
		REQUIRE(diff.count() >= 3);
		REQUIRE(diff.count() <= 5);
	}

	SECTION("Level 2 -> 8 hours")
	{
		KanjiReviewState state{1, 1, now};
		state = scheduler.GetNextState(state, 0);
		auto diff = std::chrono::duration_cast<std::chrono::hours>(state.next_review_date - now);
		REQUIRE(diff.count() >= 7);
		REQUIRE(diff.count() <= 9);
	}

	SECTION("Level 5 (Guru 1) -> 1 week")
	{
		KanjiReviewState state{1, 4, now};
		state = scheduler.GetNextState(state, 0);
		auto diff = std::chrono::duration_cast<std::chrono::hours>(state.next_review_date - now);
		REQUIRE(diff.count() >= 167);
		REQUIRE(diff.count() <= 169);
	}

	SECTION("Level 8 (Enlightened) -> 4 months")
	{
		KanjiReviewState state{1, 7, now};
		state = scheduler.GetNextState(state, 0);
		auto diff = std::chrono::duration_cast<std::chrono::hours>(state.next_review_date - now);
		REQUIRE(diff.count() >= 24 * 119);
		REQUIRE(diff.count() <= 24 * 121);
	}
}
