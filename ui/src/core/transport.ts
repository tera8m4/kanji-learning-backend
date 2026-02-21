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
  private baseUrl = "http://localhost:8080";

  public async sendAnswers(req: RequestAnswers): Promise<void> {
    await fetch(`${this.baseUrl}/api/answers`, {
      method: "POST",
      headers: { "Content-Type": "application/json" },
      body: JSON.stringify(req),
    });
  }

  public async getKanjis(): Promise<ResponseKanjis> {
    const res = await fetch(`${this.baseUrl}/api/kanjis`);
    return res.json();
  }

  public async learnMoreKanjis(): Promise<void> {
    await fetch(`${this.baseUrl}/api/learn-more`, { method: "POST" });
  }
}