#include "controller.h"
#include "database/database_context.h"
#include "resources.h"
#include "scheduler/wanikani_scheduler.h"
#include "system/platform_info.h"
#include "system/resource.h"
#include "wallpaper/wallpaper_service.h"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>
#include <string>
#include <webview/webview.h>

#ifdef _WIN32
int WINAPI WinMain(HINSTANCE /*hInst*/, HINSTANCE /*hPrevInst*/,
                   LPSTR /*lpCmdLine*/, int /*nCmdShow*/)
{
#else
int main()
{
#endif
	auto scheduler = std::make_unique<kanji::scheduler::WaniKaniScheduler>();
	kanji::database::DatabaseContext context{kanji::system::PlatformInfo::GetDatabaseLocation()};
	kanji::wallpaper::WallpaperService wallpaper_service{context.GetReviewStateRepository()};
	kanji::Controller controller{context, wallpaper_service, std::move(scheduler)};

	try
	{
		kanji::system::Resource index_html_resource(IDR_INDEX_HTML);
		const std::filesystem::path filename{"index.html"};
		const char* index_html_data = static_cast<const char*>(index_html_resource.GetData());
		std::string content(index_html_data, index_html_resource.GetSize());

		webview::webview w(false, nullptr);
		w.bind("GetKanjis", [&](const std::string&) -> std::string {
			nlohmann::json j = controller.GetReviewKanjis();
			return j.dump();
		});
		w.bind("SendAnswers", [&](const std::string& request) -> std::string {
			spdlog::info("Received asnwers: {0}", request);

			nlohmann::json j = nlohmann::json::parse(request);

			controller.SetAnswers(j[0]["answers"]);
			return {};
		});
		w.bind("LearnMoreKanjis", [&](const std::string&) -> std::string {
			spdlog::info("Learn more kanji request");

			controller.LearnMoreKanjis();
			return {};
		});
		w.set_title("KanjiStudy");
		w.set_size(800, 600, WEBVIEW_HINT_NONE);
		w.set_html(content);
		w.run();
	}
	catch (const webview::exception& e)
	{
		std::cerr << e.what() << '\n';
		return 1;
	}

	return 0;
}
