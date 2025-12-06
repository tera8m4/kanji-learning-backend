import type { KanjiWord } from "../core/transport";

export type KanjiState = {
  kanji: string;
  examples: KanjiWord[];
  meaning: string;
  incorrectStreak: number;
  id: number;
};

export type ReviewItem = {
  type: 'word' | 'meaning';
  kanjiIndex: number;
  question: string;
  answer: string;
};
