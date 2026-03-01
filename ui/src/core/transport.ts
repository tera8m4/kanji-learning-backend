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

export type ResponseReviews = {
  kanjis: KanjiData[];
  total: number;
};

export type KanjiListEntry = {
  id: number;
  kanji: string;
  meaning: string;
  level: number;
  next_review_date: number;
};

export class Transport {
  private baseUrl = "";
  public onSessionExpired: (() => void) | null = null;

  private getAuthHeaders(): HeadersInit {
    const token = getToken();
    return token ? { Authorization: `Bearer ${token}` } : {};
  }

  private async fetchWithAuth(input: RequestInfo, init?: RequestInit): Promise<Response> {
    const res = await fetch(input, init);
    if (res.status === 401) {
      this.onSessionExpired?.();
    }
    return res;
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
    await this.fetchWithAuth(`${this.baseUrl}/api/answers`, {
      method: "POST",
      headers: { "Content-Type": "application/json", ...this.getAuthHeaders() },
      body: JSON.stringify(req),
    });
  }

  public async getReviews(): Promise<ResponseReviews> {
    const res = await this.fetchWithAuth(`${this.baseUrl}/api/reviews`, {
      headers: this.getAuthHeaders(),
    });
    return res.json();
  }

  public async learnMoreKanjis(): Promise<void> {
    await this.fetchWithAuth(`${this.baseUrl}/api/learn-more`, {
      method: "POST",
      headers: this.getAuthHeaders(),
    });
  }

  public async getKanjiList(): Promise<KanjiListEntry[]> {
    const res = await this.fetchWithAuth(`${this.baseUrl}/api/kanjis`, {
      headers: this.getAuthHeaders(),
    });
    return res.json();
  }
}