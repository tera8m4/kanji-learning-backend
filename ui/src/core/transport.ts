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
  public async sendAnswers(req: RequestAnswers): Promise<void> {
    await (window as any).SendAnswers(req);
  }

  public async getKanjis(): Promise<ResponseKanjis> {
    const result = await (window as any).GetKanjis();
    return result;
  }
}