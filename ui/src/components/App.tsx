import { useState, useEffect, useRef } from "react";
import { Transport, type KanjiAnswer, type KanjiWord } from "../core/transport";
import "./App.css";

type KanjiState = {
  kanji: string,
  examples: KanjiWord[],
  meaning: string,
  incorrectStreak: number,
  id: number,
}

const transport = new Transport();

type ReviewItem = {
  type: 'word' | 'meaning';
  kanjiIndex: number;
  question: string,
  answer: string,
};

export default function App() {
  const [kanjis, setKanjis] = useState<KanjiState[]>([]);
  const [reviewDeck, setReviewDeck] = useState<ReviewItem[]>([]);
  const [userInput, setUserInput] = useState("");
  const [feedback, setFeedback] = useState<null | string>(null);
  const [shake, setShake] = useState(false);
  const [isLoading, setIsLoading] = useState(true);
  const inputRef = useRef<HTMLInputElement>(null);

  // Load kanjis from transport on mount
  useEffect(() => {
    transport.getKanjis().then(response => {
      const loadedKanjis: KanjiState[] = response.map(k => ({
        ...k,
        incorrectStreak: 0,
      }));
      setKanjis(loadedKanjis);

      // Initialize review deck
      const deck: ReviewItem[] = [];
      loadedKanjis.forEach((kanji, kanjiIndex) => {
        // Add meaning review
        deck.push({ type: 'meaning', kanjiIndex, question: kanji.kanji, answer: kanji.meaning });
        // Add word reviews
        kanji.examples.forEach(word => {
          deck.push({ type: 'word', kanjiIndex, question: word.word, answer: word.reading });
        });
      });

      setReviewDeck(deck);
      setIsLoading(false);
    });
  }, []);

  const currentReview = reviewDeck[0];
  const currentKanji = currentReview ? kanjis[currentReview.kanjiIndex] : null;

  const handleSubmit = (e: React.FormEvent) => {
    e.preventDefault();
    if (!userInput.trim() || !currentReview || !currentKanji || feedback) return;

    let isCorrect = currentReview.answer.toLowerCase().trim().split(',').map(x => x.trim()).includes(userInput.trim().toLowerCase());

    if (isCorrect) {
      setFeedback("correct");
      // Remove from deck
      setTimeout(() => {
        removeCurrentFromDeck();
      }, 500);
    } else {
      setFeedback("incorrect");
      setShake(true);

      // Mark kanji as incorrect
      setKanjis(prev => {
        const updated = [...prev];
        updated[currentReview.kanjiIndex] = {
          ...updated[currentReview.kanjiIndex],
          incorrectStreak: updated[currentReview.kanjiIndex].incorrectStreak + 1,
        };
        return updated;
      });

      // Move to end and show next card
      setTimeout(() => {
        moveToEndAndNext();
      }, 800);
    }
  };

  const removeCurrentFromDeck = () => {
    setReviewDeck(prev => {
      const newDeck = prev.slice(1);

      // Check if deck is now empty
      if (newDeck.length === 0) {
        // Send answers first
        const answers: KanjiAnswer[] = kanjis.map(kanji => ({
          kanji_id: kanji.id,
          incorrect_streak: kanji.incorrectStreak,
        }));
        transport.sendAnswers({ answers }).then(() => {
          // Try to get more kanjis
          loadMoreKanjis();
        });
      }

      return newDeck;
    });

    setFeedback(null);
    setUserInput("");
  };

  const loadMoreKanjis = () => {
    setIsLoading(true);
    transport.getKanjis().then(response => {
      if (response.length === 0) {
        // No more kanjis - stay in completion state
        setIsLoading(false);
        return;
      }

      // Load new batch of kanjis
      const loadedKanjis: KanjiState[] = response.map(k => ({
        ...k,
        incorrectStreak: 0,
      }));
      setKanjis(loadedKanjis);

      // Initialize review deck
      const deck: ReviewItem[] = [];
      loadedKanjis.forEach((kanji, kanjiIndex) => {
        deck.push({ type: 'meaning', kanjiIndex, question: kanji.kanji, answer: kanji.meaning });
        kanji.examples.forEach(word => {
          deck.push({ type: 'word', kanjiIndex, question: word.word, answer: word.reading });
        });
      });
      setReviewDeck(deck);
      setIsLoading(false);
    });
  };

  const moveToEndAndNext = () => {
    setReviewDeck(prev => {
      const newDeck = [...prev];
      const current = newDeck.shift()!;
      // Mark as not first try correct and move to end
      newDeck.push(current);
      return newDeck;
    });
    setFeedback(null);
    setUserInput("");
    setShake(false);
  };

  // Refocus input when feedback clears (after moving to next card)
  useEffect(() => {
    if (!feedback && inputRef.current) {
      inputRef.current.focus();
    }
  }, [feedback]);

  // Loading state
  if (isLoading) {
    return (
      <div className="container">
        <header className="header">
          <h1 className="title">読み込み中...</h1>
          <p className="subtitle">Loading</p>
        </header>
      </div>
    );
  }

  // Completion state - only show if we have kanjis but no more review items
  if (!currentReview || !currentKanji) {
    const correctCount = kanjis.filter(k => k.incorrectStreak == 0).length;
    const totalCount = kanjis.length;

    return (
      <div className="container">
        <header className="header">
          <h1 className="title">完了！</h1>
          <p className="subtitle">Review Complete</p>
        </header>
        <div className="score-container" style={{ position: 'static', marginTop: '20px' }}>
          <div className="score-box">
            <span className="score-label">正解</span>
            <span className="score-value">{correctCount}</span>
          </div>
          <div className="score-divider">/</div>
          <div className="score-box">
            <span className="score-label">合計</span>
            <span className="score-value">{totalCount}</span>
          </div>
        </div>
      </div>
    );
  }

  const totalItems = reviewDeck.length;
  const getPlaceholder = () => {
    if (currentReview.type === 'meaning') {
      return "Enter the meaning...";
    } else {
      return "Enter the reading...";
    }
  };

  return (
    <div className="container">
      {/* Decorative ink splatter background elements */}
      <div className="ink-splatter-1"></div>
      <div className="ink-splatter-2"></div>

      {/* Header */}
      <header className="header">
        <h1 className="title">漢字練習</h1>
        <p className="subtitle">Kanji Practice</p>
      </header>

      {/* Flashcard */}
      <div
        className={`card ${feedback === "correct" ? "card-correct" : ""} ${feedback === "incorrect" ? "card-incorrect" : ""} ${shake ? "card-shake" : ""}`}
      >
        <div className="card-inner">
          {/* Card number indicator */}
          <div className="card-number">
            Remaining: {totalItems}
          </div>

          {/* Small kanji in top right */}
          <div className="card-kanji-small">{currentKanji.kanji}</div>

          {/* Question display */}
          <div className="kanji-container">
            <span className="kanji">{currentReview.question}</span>
          </div>

          {/* Input form */}
          <form onSubmit={handleSubmit} className="form">
            <div className="input-wrapper">
              <input
                ref={inputRef}
                type="text"
                value={userInput}
                onChange={(e) => setUserInput(e.target.value)}
                placeholder={getPlaceholder()}
                className={`input ${feedback === "correct" ? "input-correct" : ""} ${feedback === "incorrect" ? "input-incorrect" : ""}`}
                disabled={feedback !== null}
                autoFocus
              />
              <div className="input-underline"></div>
            </div>

            {/* Feedback message */}
            <div className="feedback-container">
              {feedback && (
                <div className={`feedback ${feedback === "correct" ? "feedback-correct" : "feedback-incorrect"}`}>
                  {feedback === "correct" ? "正解！ Correct!" : "もう一度 Try again"}
                </div>
              )}
            </div>
          </form>
        </div>

        {/* Decorative seal */}
        <div className="seal">練</div>
      </div>

    </div>
  );
}
