#include "sqlite_connection.h"
#include <iostream>
#include <spdlog/spdlog.h>
#include <sqlite3.h>

namespace kanji::database
{
	SQLiteConnection::SQLiteConnection(std::string in_db_path)
	    : db_path{std::move(in_db_path)}
	{
		int rc = sqlite3_open(db_path.c_str(), &db);
		if (rc != SQLITE_OK)
		{
			spdlog::error("Cannot open database: {0}", sqlite3_errmsg(db));
		}
	}

	SQLiteConnection::~SQLiteConnection()
	{
		if (db)
		{
			sqlite3_close(db);
		}
	}

	bool SQLiteConnection::Initialize()
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
		    "next_review_date INTEGER NOT NULL DEFAULT (unixepoch()),"
		    "created_at INTEGER NOT NULL DEFAULT (unixepoch()),"
		    "FOREIGN KEY(kanji_id) REFERENCES kanjis(id) ON DELETE CASCADE"
		    ");";

		char* err_msg = nullptr;

		int rc = sqlite3_exec(db, kanji_table_sql, nullptr, nullptr, &err_msg);
		if (rc != SQLITE_OK)
		{
			spdlog::error("SQL error: {0}", err_msg);
			sqlite3_free(err_msg);
			return false;
		}

		rc = sqlite3_exec(db, words_table_sql, nullptr, nullptr, &err_msg);
		if (rc != SQLITE_OK)
		{
			spdlog::error("SQL error: {0}", err_msg);
			sqlite3_free(err_msg);
			return false;
		}

		rc = sqlite3_exec(db, review_state_table_sql, nullptr, nullptr, &err_msg);
		if (rc != SQLITE_OK)
		{
			spdlog::error("SQL error: {0}", err_msg);
			sqlite3_free(err_msg);
			return false;
		}

		return true;
	}

	sqlite3* SQLiteConnection::GetDB() const
	{
		return db;
	}

	SQLiteConnection::operator sqlite3*() const
	{
		return GetDB();
	}
} // namespace kanji::database
