#pragma once

#include <string>
#include <vector>
#include <chrono>
#include <nlohmann/json.hpp>

namespace kanji {
struct KanjiWord {
  std::string word;
  std::string reading;
};

struct KanjiData {
  std::uint32_t id;
  std::string kanji;
  std::string meaning;
  std::vector<KanjiWord> examples;
};

struct KanjiReviewState {
  std::uint32_t kanji_id;
  int level;
  int incorrect_streak;
  std::chrono::system_clock::time_point next_review_date;
  std::chrono::system_clock::time_point created_at;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(KanjiWord, word, reading)
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(KanjiData, id, kanji, examples, meaning)

};
