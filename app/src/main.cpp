#include "controller.h"
#include "database/database_context.h"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>
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
	kanji::database::DatabaseContext context{"kanji.db"};
	context.GetReviewStateRepository().InitializeNewReviewStates(20);
	kanji::Controller controller{context};

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
