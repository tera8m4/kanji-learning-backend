#include "kanji_repository.h"
#include "sqlite_connection.h"
#include <spdlog/spdlog.h>
#include <sqlite3.h>

namespace kanji::database
{
	KanjiRepository::KanjiRepository(const SQLiteConnection& in_connection)
	    : connection{in_connection}
	{
	}

	std::vector<KanjiData> KanjiRepository::GetKanjiForReview() const
	{
		std::vector<KanjiData> kanjis;
		const char* sql =
		    "SELECT k.id, k.kanji, k.meaning "
		    "FROM kanjis k "
		    "INNER JOIN kanji_review_state rs ON k.id = rs.kanji_id "
		    "WHERE rs.next_review_date < ? "
		    "ORDER BY rs.next_review_date "
		    "LIMIT 5;";
		sqlite3_stmt* stmt;

		int rc = sqlite3_prepare_v2(connection, sql, -1, &stmt, nullptr);
		if (rc != SQLITE_OK)
		{
			spdlog::error("Failed to prepare statement: {0}", sqlite3_errmsg(connection));
			return kanjis;
		}

		std::int64_t now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
		sqlite3_bind_int64(stmt, 1, now);

		while ((rc = sqlite3_step(stmt)) == SQLITE_ROW)
		{
			KanjiData kanji;
			kanji.id = sqlite3_column_int(stmt, 0);
			kanji.kanji = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
			kanji.meaning = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
			kanji.examples = GetKanjiWords(kanji.id);
			kanjis.push_back(kanji);
		}

		sqlite3_finalize(stmt);
		return kanjis;
	}

	std::vector<KanjiWord> KanjiRepository::GetKanjiWords(const std::uint32_t kanji_id) const
	{
		std::vector<KanjiWord> words;
		const char* sql = "SELECT word, reading FROM kanji_words WHERE kanji_id = ?;";
		sqlite3_stmt* stmt;

		int rc = sqlite3_prepare_v2(connection, sql, -1, &stmt, nullptr);
		if (rc != SQLITE_OK)
		{
			spdlog::error("Failed to prepare statement: {0}", sqlite3_errmsg(connection));
			return words;
		}

		sqlite3_bind_int(stmt, 1, kanji_id);

		while ((rc = sqlite3_step(stmt)) == SQLITE_ROW)
		{
			KanjiWord word;
			word.word = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
			word.reading = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
			words.push_back(word);
		}

		sqlite3_finalize(stmt);
		return words;
	}

} // namespace kanji::database
