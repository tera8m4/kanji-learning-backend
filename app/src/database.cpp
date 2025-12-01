#include "database.h"
#include <iostream>

namespace kanji
{
	Database::Database(const std::string& db_path)
	    : db_(nullptr), db_path_(db_path)
	{
	}

	Database::~Database()
	{
		if (db_)
		{
			sqlite3_close(db_);
		}
	}

	bool Database::Initialize()
	{
		int rc = sqlite3_open(db_path_.c_str(), &db_);
		if (rc != SQLITE_OK)
		{
			std::cerr << "Cannot open database: " << sqlite3_errmsg(db_) << std::endl;
			return false;
		}

		return CreateTables();
	}

	bool Database::CreateTables()
	{
		const char* kanji_table_sql =
		    "CREATE TABLE IF NOT EXISTS kanjis ("
		    "id INTEGER PRIMARY KEY,"
		    "kanji TEXT NOT NULL,"
		    "meaning TEXT NOT NULL"
		    ");";

		const char* words_table_sql =
		    "CREATE TABLE IF NOT EXISTS kanji_words ("
		    "id INTEGER PRIMARY KEY AUTOINCREMENT,"
		    "kanji_id INTEGER NOT NULL,"
		    "word TEXT NOT NULL,"
		    "reading TEXT NOT NULL,"
		    "FOREIGN KEY(kanji_id) REFERENCES kanjis(id) ON DELETE CASCADE"
		    ");";

		const char* review_state_table_sql =
		    "CREATE TABLE IF NOT EXISTS kanji_review_state ("
		    "kanji_id INTEGER PRIMARY KEY,"
		    "level INTEGER NOT NULL DEFAULT 0,"
		    "incorrect_streak INTEGER NOT NULL DEFAULT 0,"
		    "next_review_date INTEGER NOT NULL DEFAULT NOW(),"
				"created_at INTEGER NOT NULL DEFAULT NOW()"
		    "FOREIGN KEY(kanji_id) REFERENCES kanjis(id) ON DELETE CASCADE"
		    ");";

		char* err_msg = nullptr;

		int rc = sqlite3_exec(db_, kanji_table_sql, nullptr, nullptr, &err_msg);
		if (rc != SQLITE_OK)
		{
			std::cerr << "SQL error: " << err_msg << std::endl;
			sqlite3_free(err_msg);
			return false;
		}

		rc = sqlite3_exec(db_, words_table_sql, nullptr, nullptr, &err_msg);
		if (rc != SQLITE_OK)
		{
			std::cerr << "SQL error: " << err_msg << std::endl;
			sqlite3_free(err_msg);
			return false;
		}

		rc = sqlite3_exec(db_, review_state_table_sql, nullptr, nullptr, &err_msg);
		if (rc != SQLITE_OK)
		{
			std::cerr << "SQL error: " << err_msg << std::endl;
			sqlite3_free(err_msg);
			return false;
		}

		return true;
	}

	std::optional<KanjiData> Database::GetKanjiById(std::uint32_t id)
	{
		const char* sql = "SELECT id, kanji, meaning FROM kanjis WHERE id = ?;";
		sqlite3_stmt* stmt;

		int rc = sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr);
		if (rc != SQLITE_OK)
		{
			std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db_) << std::endl;
			return std::nullopt;
		}

		sqlite3_bind_int(stmt, 1, id);

		rc = sqlite3_step(stmt);
		if (rc == SQLITE_ROW)
		{
			KanjiData kanji;
			kanji.id = sqlite3_column_int(stmt, 0);
			kanji.kanji = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
			kanji.meaning = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
			kanji.examples = GetKanjiWords(kanji.id);

			sqlite3_finalize(stmt);
			return kanji;
		}

		sqlite3_finalize(stmt);
		return std::nullopt;
	}

	std::vector<KanjiWord> Database::GetKanjiWords(std::uint32_t kanji_id)
	{
		std::vector<KanjiWord> words;
		const char* sql = "SELECT word, reading FROM kanji_words WHERE kanji_id = ?;";
		sqlite3_stmt* stmt;

		int rc = sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr);
		if (rc != SQLITE_OK)
		{
			std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db_) << std::endl;
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

	std::optional<KanjiReviewState> Database::GetReviewState(std::uint32_t kanji_id)
	{
		const char* sql = "SELECT kanji_id, level, incorrect_streak, next_review_date, created_at FROM kanji_review_state WHERE kanji_id = ?;";
		sqlite3_stmt* stmt;

		int rc = sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr);
		if (rc != SQLITE_OK)
		{
			std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db_) << std::endl;
			return std::nullopt;
		}

		sqlite3_bind_int(stmt, 1, kanji_id);

		rc = sqlite3_step(stmt);
		if (rc == SQLITE_ROW)
		{
			KanjiReviewState state;
			state.kanji_id = sqlite3_column_int(stmt, 0);
			state.level = sqlite3_column_int(stmt, 1);
			state.incorrect_streak = sqlite3_column_int(stmt, 2);

			std::int64_t next_review_timestamp = sqlite3_column_int64(stmt, 3);
			state.next_review_date = std::chrono::system_clock::from_time_t(next_review_timestamp);

			std::int64_t created_timestamp = sqlite3_column_int64(stmt, 4);
			state.created_at = std::chrono::system_clock::from_time_t(created_timestamp);

			sqlite3_finalize(stmt);
			return state;
		}

		sqlite3_finalize(stmt);
		return std::nullopt;
	}

	bool Database::CreateOrUpdateReviewState(const KanjiReviewState& state)
	{
		const char* sql =
		    "INSERT INTO kanji_review_state (kanji_id, level, incorrect_streak, next_review_date, created_at) "
		    "VALUES (?, ?, ?, ?, unixepoch()) "
		    "ON CONFLICT(kanji_id) DO UPDATE SET "
		    "level = excluded.level, "
		    "incorrect_streak = excluded.incorrect_streak, "
		    "next_review_date = excluded.next_review_date;";
		sqlite3_stmt* stmt;

		int rc = sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr);
		if (rc != SQLITE_OK)
		{
			std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db_) << std::endl;
			return false;
		}

		std::int64_t timestamp = std::chrono::system_clock::to_time_t(state.next_review_date);

		sqlite3_bind_int(stmt, 1, state.kanji_id);
		sqlite3_bind_int(stmt, 2, state.level);
		sqlite3_bind_int(stmt, 3, state.incorrect_streak);
		sqlite3_bind_int64(stmt, 4, timestamp);

		rc = sqlite3_step(stmt);
		sqlite3_finalize(stmt);

		if (rc != SQLITE_DONE)
		{
			std::cerr << "Failed to insert/update review state: " << sqlite3_errmsg(db_) << std::endl;
			return false;
		}

		return true;
	}

	std::vector<KanjiData> Database::GetKanjisForReview()
	{
		std::vector<KanjiData> kanjis;
		const char* sql =
		    "SELECT k.id, k.kanji, k.meaning "
		    "FROM kanjis k "
		    "INNER JOIN kanji_review_state rs ON k.id = rs.kanji_id "
		    "WHERE rs.next_review_date < ?;";
		sqlite3_stmt* stmt;

		int rc = sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr);
		if (rc != SQLITE_OK)
		{
			std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db_) << std::endl;
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

	bool Database::InitializeNewReviewStates(int count)
	{
		// Check if there are any unreviewed states (created_at == next_review_date)
		const char* check_sql = "SELECT COUNT(*) FROM kanji_review_state WHERE created_at = next_review_date;";
		sqlite3_stmt* check_stmt;

		int rc = sqlite3_prepare_v2(db_, check_sql, -1, &check_stmt, nullptr);
		if (rc != SQLITE_OK)
		{
			std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db_) << std::endl;
			return false;
		}

		rc = sqlite3_step(check_stmt);
		int unreviewed_count = 0;
		if (rc == SQLITE_ROW)
		{
			unreviewed_count = sqlite3_column_int(check_stmt, 0);
		}
		sqlite3_finalize(check_stmt);

		// If there are unreviewed states, don't add new ones
		if (unreviewed_count > 0)
		{
			return true;
		}

		// Get next K kanjis that don't have review states
		const char* select_sql =
		    "SELECT id FROM kanjis "
		    "WHERE id NOT IN (SELECT kanji_id FROM kanji_review_state) "
		    "ORDER BY id LIMIT ?;";
		sqlite3_stmt* select_stmt;

		rc = sqlite3_prepare_v2(db_, select_sql, -1, &select_stmt, nullptr);
		if (rc != SQLITE_OK)
		{
			std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db_) << std::endl;
			return false;
		}

		sqlite3_bind_int(select_stmt, 1, count);

		std::vector<std::uint32_t> kanji_ids;
		while ((rc = sqlite3_step(select_stmt)) == SQLITE_ROW)
		{
			kanji_ids.push_back(sqlite3_column_int(select_stmt, 0));
		}
		sqlite3_finalize(select_stmt);

		// Insert new review states with next_review_date = created_at = now()
		std::int64_t now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
		const char* insert_sql =
		    "INSERT INTO kanji_review_state (kanji_id, level, incorrect_streak, next_review_date, created_at) "
		    "VALUES (?, 0, 0, ?, ?);";

		for (std::uint32_t kanji_id : kanji_ids)
		{
			sqlite3_stmt* insert_stmt;
			rc = sqlite3_prepare_v2(db_, insert_sql, -1, &insert_stmt, nullptr);
			if (rc != SQLITE_OK)
			{
				std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db_) << std::endl;
				return false;
			}

			sqlite3_bind_int(insert_stmt, 1, kanji_id);
			sqlite3_bind_int64(insert_stmt, 2, now);
			sqlite3_bind_int64(insert_stmt, 3, now);

			rc = sqlite3_step(insert_stmt);
			sqlite3_finalize(insert_stmt);

			if (rc != SQLITE_DONE)
			{
				std::cerr << "Failed to insert review state: " << sqlite3_errmsg(db_) << std::endl;
				return false;
			}
		}

		return true;
	}

} // namespace kanji
