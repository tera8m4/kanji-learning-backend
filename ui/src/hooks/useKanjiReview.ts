import { useState, useEffect } from "react";
import * as wanakana from "wanakana";
import type { KanjiAnswer } from "../core/transport";
import type { KanjiState, ReviewItem } from "../components/types";
import { createTransport } from "../core/transportFactory";

const transport = createTransport();

export function useKanjiReview() {
  const [kanjis, setKanjis] = useState<KanjiState[]>([]);
  const [reviewDeck, setReviewDeck] = useState<ReviewItem[]>([]);
  const [userInput, setUserInput] = useState("");
  const [feedback, setFeedback] = useState<null | string>(null);
  const [shake, setShake] = useState(false);
  const [isLoading, setIsLoading] = useState(true);
  const [canRollback, setCanRollback] = useState(false);

  // Shuffle array in random order using Fisher-Yates algorithm
  const shuffleArray = <T,>(array: T[]): T[] => {
    const shuffled = [...array];
    for (let i = shuffled.length - 1; i > 0; i--) {
      const j = Math.floor(Math.random() * (i + 1));
      [shuffled[i], shuffled[j]] = [shuffled[j], shuffled[i]];
    }
    return shuffled;
  };

  // Wrapper for setUserInput that applies wanakana conversion for word reviews
  const handleUserInput = (value: string) => {
    const currentReview = reviewDeck[0];
    if (currentReview?.type === 'word') {
      // Convert romaji to hiragana for word readings
      setUserInput(wanakana.toHiragana(value, { IMEMode: true }));
    } else {
      setUserInput(value);
    }
  };

  const loadKanjis = (response: any[]) => {
    const loadedKanjis: KanjiState[] = response.map(k => ({
      ...k,
      incorrectStreak: 0,
    }));
    setKanjis(loadedKanjis);

    const deck: ReviewItem[] = [];
    loadedKanjis.forEach((kanji, kanjiIndex) => {
      deck.push({ type: 'meaning', kanjiIndex, question: kanji.kanji, answer: kanji.meaning });
      kanji.examples.forEach(word => {
        deck.push({ type: 'word', kanjiIndex, question: word.word, answer: word.reading });
      });
    });

    // Shuffle the deck for random order
    const shuffledDeck = shuffleArray(deck);
    setReviewDeck(shuffledDeck);
    setIsLoading(false);
  };

  const loadMoreKanjis = () => {
    setIsLoading(true);
    transport.getKanjis().then(response => {
      if (response.length === 0) {
        setIsLoading(false);
        return;
      }
      loadKanjis(response);
    });
  };

  useEffect(() => {
    transport.getKanjis().then(loadKanjis);
  }, []);

  const removeCurrentFromDeck = () => {
    setReviewDeck(prev => {
      const newDeck = prev.slice(1);

      if (newDeck.length === 0) {
        const answers: KanjiAnswer[] = kanjis.map(kanji => ({
          kanji_id: kanji.id,
          incorrect_streak: kanji.incorrectStreak,
        }));
        transport.sendAnswers({ answers }).then(() => {
          loadMoreKanjis();
        });
      }

      return newDeck;
    });

    setFeedback(null);
    setUserInput("");
  };

  const moveToEndAndNext = () => {
    setReviewDeck(prev => {
      const newDeck = [...prev];
      const current = newDeck.shift()!;
      newDeck.push(current);
      return newDeck;
    });
    setFeedback(null);
    setUserInput("");
    setShake(false);
    // Don't clear canRollback here - let it persist to next card
  };

  const handleRollback = () => {
    // Move the last card (previous incorrect answer) back to the front
    setReviewDeck(prev => {
      if (prev.length === 0) return prev;
      const newDeck = [...prev];
      const lastCard = newDeck.pop()!;
      newDeck.unshift(lastCard);
      return newDeck;
    });

    // Revert the incorrect streak increment for the card we're bringing back
    setKanjis(prev => {
      const lastReview = reviewDeck[reviewDeck.length - 1];
      if (!lastReview) return prev;

      const updated = [...prev];
      updated[lastReview.kanjiIndex] = {
        ...updated[lastReview.kanjiIndex],
        incorrectStreak: Math.max(0, updated[lastReview.kanjiIndex].incorrectStreak - 1),
      };
      return updated;
    });

    // Clear feedback and allow retry
    setFeedback(null);
    setUserInput("");
    setShake(false);
    setCanRollback(false);
  };

  const handleSubmit = (e: React.FormEvent) => {
    e.preventDefault();

    const currentReview = reviewDeck[0];
    if (!userInput.trim() || !currentReview || feedback) return;

    const finalInput = currentReview.type === "meaning"
      ? userInput.toLowerCase().trim()
      : wanakana.toHiragana(userInput.toLowerCase().trim());

    const isCorrect = currentReview.answer
      .toLowerCase()
      .trim()
      .split(',')
      .map(x => x.trim())
      .includes(finalInput);

    if (isCorrect) {
      setFeedback("correct");
      setCanRollback(false);
      setTimeout(() => {
        removeCurrentFromDeck();
      }, 500);
    } else {
      setFeedback("incorrect");
      setShake(true);
      setCanRollback(true);

      setKanjis(prev => {
        const updated = [...prev];
        updated[currentReview.kanjiIndex] = {
          ...updated[currentReview.kanjiIndex],
          incorrectStreak: updated[currentReview.kanjiIndex].incorrectStreak + 1,
        };
        return updated;
      });

      setTimeout(() => {
        moveToEndAndNext();
      }, 800);
    }
  };

  const handleLearnMore = async () => {
    setIsLoading(true);
    await transport.learnMoreKanjis();
    const response = await transport.getKanjis();
    loadKanjis(response);
  };

  const currentReview = reviewDeck[0];
  const currentKanji = currentReview ? kanjis[currentReview.kanjiIndex] : null;

  return {
    kanjis,
    reviewDeck,
    currentReview,
    currentKanji,
    userInput,
    setUserInput: handleUserInput,
    feedback,
    shake,
    isLoading,
    handleSubmit,
    handleLearnMore,
    canRollback,
    handleRollback,
  };
}
