#include "review_notifier.h"
#include "database/database_context.h"
#include <spdlog/spdlog.h>

namespace kanji::notification
{
	ReviewNotifier::ReviewNotifier(database::DatabaseContext& in_db,
	                               std::unique_ptr<INotificationService> in_notification_service,
	                               std::chrono::minutes in_check_interval)
	    : db{in_db}
	    , notification_service{std::move(in_notification_service)}
	    , check_interval{in_check_interval}
	{
	}

	void ReviewNotifier::Start()
	{
		worker = std::jthread([this](std::stop_token token) { Run(token); });
	}

	void ReviewNotifier::Run(std::stop_token stop_token)
	{
		while (!stop_token.stop_requested())
		{
			const int count = db.GetKanjiRepository().GetPendingReviewCount();

			if (count > 0)
			{
				spdlog::info("ReviewNotifier: {} reviews pending, sending reminder", count);
				notification_service->SendReviewReminder(count);
			}

			std::condition_variable_any cv;
			std::mutex mutex;
			std::unique_lock lock(mutex);
			cv.wait_for(lock, stop_token, check_interval, [] { return false; });
		}

		spdlog::info("ReviewNotifier: shutdown");
	}
} // namespace kanji::notification
