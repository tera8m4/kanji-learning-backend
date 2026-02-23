import { getToken, setToken } from "./auth";

export type TelegramAuthPayload = {
  id: string;
  first_name: string;
  last_name?: string;
  username: string;
  photo_url?: string;
  auth_date: number;
  hash: string;
};

export type KanjiAnswer = {
  kanji_id: number,
  incorrect_streak: number,
}

export type RequestAnswers = {
  answers: KanjiAnswer[]
}

export type KanjiWord = {
  word: string,
  reading: string
};

export type KanjiData = {
  id: number,
  kanji: string,
  examples: KanjiWord[],
  meaning: string,
};

export type ResponseKanjis = KanjiData[];

export class Transport {
  private baseUrl = "";

  private getAuthHeaders(): HeadersInit {
    const token = getToken();
    return token ? { Authorization: `Bearer ${token}` } : {};
  }

  public async login(telegramData: TelegramAuthPayload): Promise<void> {
    const res = await fetch(`${this.baseUrl}/api/login`, {
      method: "POST",
      headers: { "Content-Type": "application/json" },
      body: JSON.stringify(telegramData),
    });
    if (!res.ok) throw new Error("Login failed");
    const { token } = await res.json();
    setToken(token);
  }

  public async sendAnswers(req: RequestAnswers): Promise<void> {
    await fetch(`${this.baseUrl}/api/answers`, {
      method: "POST",
      headers: { "Content-Type": "application/json", ...this.getAuthHeaders() },
      body: JSON.stringify(req),
    });
  }

  public async getKanjis(): Promise<ResponseKanjis> {
    const res = await fetch(`${this.baseUrl}/api/kanjis`, {
      headers: this.getAuthHeaders(),
    });
    return res.json();
  }

  public async learnMoreKanjis(): Promise<void> {
    await fetch(`${this.baseUrl}/api/learn-more`, {
      method: "POST",
      headers: this.getAuthHeaders(),
    });
  }
}