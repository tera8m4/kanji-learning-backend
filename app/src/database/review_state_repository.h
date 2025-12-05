#pragma once

namespace kanji
{
	struct KanjiReviewState;
}

namespace kanji::database
{
	class SQLiteConnection;

	class ReviewStateRepository
	{
	public:
		explicit ReviewStateRepository(const SQLiteConnection& in_connection)
		    : connection{in_connection}
		{}
		void InitializeNewReviewStates(int count);
		void CreateOrUpdateReviewState(const KanjiReviewState& state);

	private:
		const SQLiteConnection& connection;
	};
} // namespace kanji::database
