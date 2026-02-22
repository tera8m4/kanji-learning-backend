# KanjiReview

A personal kanji flashcard app using the WaniKani Spaced Repetition System (SRS). 

<img width="578" height="473" alt="{B29A0442-A7DC-4E4D-BFCE-77AACE22B7B4}" src="https://github.com/user-attachments/assets/45bc2832-9db7-4825-bdb0-ffd7d758ea51" />

## How it works

The app surfaces kanji cards due for review based on SRS intervals. After each session you mark cards correct or incorrect, and the scheduler adjusts the next review time accordingly. A Telegram bot sends notifications when reviews are ready.

**SRS levels and intervals:**

| Level | Interval |
|-------|----------|
| 1 | 4 hours |
| 2 | 8 hours |
| 3 | 1 day |
| 4 | 2 days |
| 5 (Guru) | 1 week |
| 6 | 2 weeks |
| 7 | 1 month |
| 8 (Enlightened) | 4 months |
| 9 (Burned) | — |

Incorrect answers at Guru or above incur a double penalty (drop 2 levels instead of 1).

## Stack

| Layer | Technology |
|-------|------------|
| Backend | C++23, [Crow](https://github.com/CrowCpp/Crow) HTTP framework |
| Database | SQLite3 |
| Frontend | React 19 + TypeScript |
| Notifications | Telegram Bot API |

## Configuration

Place `config.json` next to the executable:

```json
{
  "notification": {
    "refresh_interval": 30,
    "telegram": {
      "bot_token": "YOUR_BOT_TOKEN",
      "chat_id": 123456789
    }
  }
}
```

The app will not start without this file.

## Deployment

Push to `master` triggers the GitHub Actions workflow which:

1. Builds on Ubuntu 24.04
2. SSHes into the VPS
3. Stops the `kanjireview` systemd service
4. Copies the new binary and assets to `/opt/kanjireview/`
5. Starts the service again

nginx proxies `https://kanji.re-l.tatar` → `http://127.0.0.1:8080`.

**Required GitHub secrets:** `VPS_SSH_KEY`, `VPS_HOST`, `VPS_USER`.
