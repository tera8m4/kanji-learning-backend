import { useState, useEffect } from "react";
import { Transport, type KanjiAnswer } from "../core/transport";
import type { KanjiState, ReviewItem } from "../components/types";

const transport = new Transport();

export function useKanjiReview() {
  const [kanjis, setKanjis] = useState<KanjiState[]>([]);
  const [reviewDeck, setReviewDeck] = useState<ReviewItem[]>([]);
  const [userInput, setUserInput] = useState("");
  const [feedback, setFeedback] = useState<null | string>(null);
  const [shake, setShake] = useState(false);
  const [isLoading, setIsLoading] = useState(true);

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

    setReviewDeck(deck);
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
  };

  const handleSubmit = (e: React.FormEvent) => {
    e.preventDefault();

    const currentReview = reviewDeck[0];
    if (!userInput.trim() || !currentReview || feedback) return;

    const isCorrect = currentReview.answer
      .toLowerCase()
      .trim()
      .split(',')
      .map(x => x.trim())
      .includes(userInput.trim().toLowerCase());

    if (isCorrect) {
      setFeedback("correct");
      setTimeout(() => {
        removeCurrentFromDeck();
      }, 500);
    } else {
      setFeedback("incorrect");
      setShake(true);

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
    await transport.learnMoreKanjis();
  };

  const currentReview = reviewDeck[0];
  const currentKanji = currentReview ? kanjis[currentReview.kanjiIndex] : null;

  return {
    kanjis,
    reviewDeck,
    currentReview,
    currentKanji,
    userInput,
    setUserInput,
    feedback,
    shake,
    isLoading,
    handleSubmit,
    handleLearnMore,
  };
}
