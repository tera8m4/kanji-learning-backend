#include "app.h"
#include "config.h"
#include <filesystem>
#include <spdlog/spdlog.h>

int main()
{
	const std::filesystem::path config_path{"config.json"};
	if (!std::filesystem::exists(config_path))
	{
		spdlog::warn("No config.json found — notifications disabled");
		return 1;
	}

	const auto config = kanji::config::KanjiAppConfig::LoadFromFile(config_path);
	kanji::KanjiApp{config}.Run();

	return 0;
}
