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

	int KanjiRepository::GetPendingReviewCount() const
	{
		const char* sql =
		    "SELECT COUNT(*) "
		    "FROM kanji_review_state "
		    "WHERE next_review_date < ?;";
		sqlite3_stmt* stmt;

		int rc = sqlite3_prepare_v2(connection, sql, -1, &stmt, nullptr);
		if (rc != SQLITE_OK)
		{
			spdlog::error("Failed to prepare statement: {0}", sqlite3_errmsg(connection));
			return 0;
		}

		std::int64_t now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
		sqlite3_bind_int64(stmt, 1, now);

		int count = 0;
		if (sqlite3_step(stmt) == SQLITE_ROW)
		{
			count = sqlite3_column_int(stmt, 0);
		}

		sqlite3_finalize(stmt);
		return count;
	}

	void KanjiRepository::BatchInsertKanjis(const std::vector<KanjiData>& kanjis)
	{
		if (kanjis.empty())
		{
			return;
		}

		char* err_msg = nullptr;
		sqlite3_exec(connection, "BEGIN TRANSACTION;", nullptr, nullptr, &err_msg);
		if (err_msg)
		{
			spdlog::error("Failed to begin transaction: {0}", err_msg);
			sqlite3_free(err_msg);
			return;
		}

		const char* kanji_sql = "INSERT INTO kanjis (kanji, meaning) VALUES (?, ?);";
		const char* word_sql = "INSERT INTO kanji_words (kanji_id, word, reading) VALUES (?, ?, ?);";
		const char* review_sql =
		    "INSERT OR IGNORE INTO kanji_review_state (kanji_id, level, next_review_date, created_at) "
		    "VALUES (?, 0, ?, ?);";

		sqlite3_stmt* kanji_stmt;
		sqlite3_stmt* word_stmt;
		sqlite3_stmt* review_stmt;

		if (sqlite3_prepare_v2(connection, kanji_sql, -1, &kanji_stmt, nullptr) != SQLITE_OK ||
		    sqlite3_prepare_v2(connection, word_sql, -1, &word_stmt, nullptr) != SQLITE_OK ||
		    sqlite3_prepare_v2(connection, review_sql, -1, &review_stmt, nullptr) != SQLITE_OK)
		{
			spdlog::error("Failed to prepare batch insert statements: {0}", sqlite3_errmsg(connection));
			sqlite3_exec(connection, "ROLLBACK;", nullptr, nullptr, nullptr);
			return;
		}

		std::int64_t now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());

		for (const auto& kanji : kanjis)
		{
			sqlite3_bind_text(kanji_stmt, 1, kanji.kanji.c_str(), -1, SQLITE_TRANSIENT);
			sqlite3_bind_text(kanji_stmt, 2, kanji.meaning.c_str(), -1, SQLITE_TRANSIENT);

			if (sqlite3_step(kanji_stmt) != SQLITE_DONE)
			{
				spdlog::error("Failed to insert kanji '{0}': {1}", kanji.kanji, sqlite3_errmsg(connection));
				sqlite3_reset(kanji_stmt);
				continue;
			}

			std::int64_t kanji_id = sqlite3_last_insert_rowid(connection);
			sqlite3_reset(kanji_stmt);

			for (const auto& word : kanji.examples)
			{
				sqlite3_bind_int64(word_stmt, 1, kanji_id);
				sqlite3_bind_text(word_stmt, 2, word.word.c_str(), -1, SQLITE_TRANSIENT);
				sqlite3_bind_text(word_stmt, 3, word.reading.c_str(), -1, SQLITE_TRANSIENT);

				if (sqlite3_step(word_stmt) != SQLITE_DONE)
				{
					spdlog::error("Failed to insert word '{0}': {1}", word.word, sqlite3_errmsg(connection));
				}
				sqlite3_reset(word_stmt);
			}

			sqlite3_bind_int64(review_stmt, 1, kanji_id);
			sqlite3_bind_int64(review_stmt, 2, now);
			sqlite3_bind_int64(review_stmt, 3, now);

			if (sqlite3_step(review_stmt) != SQLITE_DONE)
			{
				spdlog::error("Failed to insert review state for kanji '{0}': {1}", kanji.kanji,
				              sqlite3_errmsg(connection));
			}
			sqlite3_reset(review_stmt);
		}

		sqlite3_finalize(kanji_stmt);
		sqlite3_finalize(word_stmt);
		sqlite3_finalize(review_stmt);

		sqlite3_exec(connection, "COMMIT;", nullptr, nullptr, &err_msg);
		if (err_msg)
		{
			spdlog::error("Failed to commit transaction: {0}", err_msg);
			sqlite3_free(err_msg);
		}
	}

	std::vector<KanjiRecord> KanjiRepository::GetKanjis() const
	{
		std::vector<KanjiRecord> result;
		const char* sql =
		    "SELECT k.id, k.kanji, k.meaning, rs.level, rs.next_review_date "
		    "FROM kanjis k "
		    "INNER JOIN kanji_review_state rs ON k.id = rs.kanji_id "
		    "ORDER BY rs.next_review_date;";
		sqlite3_stmt* stmt;

		int rc = sqlite3_prepare_v2(connection, sql, -1, &stmt, nullptr);
		if (rc != SQLITE_OK)
		{
			spdlog::error("Failed to prepare statement: {0}", sqlite3_errmsg(connection));
			return result;
		}

		while ((rc = sqlite3_step(stmt)) == SQLITE_ROW)
		{
			KanjiRecord entry;
			entry.id = sqlite3_column_int(stmt, 0);
			entry.kanji = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
			entry.meaning = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
			entry.level = sqlite3_column_int(stmt, 3);
			entry.next_review_date = sqlite3_column_int64(stmt, 4);
			result.push_back(entry);
		}

		sqlite3_finalize(stmt);
		return result;
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
