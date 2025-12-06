#include "controller.h"
#include "database/database_context.h"
#include "scheduler/wanikani_scheduler.h"
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
	kanji::database::DatabaseContext context{"kanji.db"};
	kanji::Controller controller{context, std::move(scheduler)};

	try
	{
		const std::filesystem::path filename{"index.html"};

		auto size = std::filesystem::file_size(filename);
		std::string content(size, '\0');
		std::ifstream in(filename);
		in.read(&content[0], size);

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
		w.set_title("Basic Example");
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
