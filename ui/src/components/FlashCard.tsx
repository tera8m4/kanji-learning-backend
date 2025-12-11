import { useRef, useEffect } from "react";
import type { KanjiState, ReviewItem } from "./types";

type FlashCardProps = {
  currentReview: ReviewItem;
  currentKanji: KanjiState;
  totalItems: number;
  userInput: string;
  feedback: string | null;
  shake: boolean;
  canRollback: boolean;
  onInputChange: (value: string) => void;
  onSubmit: (e: React.FormEvent) => void;
  onRollback: () => void;
};

export default function FlashCard({
  currentReview,
  currentKanji,
  totalItems,
  userInput,
  feedback,
  shake,
  canRollback,
  onInputChange,
  onSubmit,
  onRollback,
}: FlashCardProps) {
  const inputRef = useRef<HTMLInputElement>(null);

  // Restore focus when feedback clears (after moving to next card)
  useEffect(() => {
    if (!feedback && inputRef.current) {
      inputRef.current.focus();
    }
  }, [feedback, currentReview]);

  // Bind Ctrl+Z / Cmd+Z to rollback
  useEffect(() => {
    const handleKeyDown = (e: KeyboardEvent) => {
      if ((e.ctrlKey || e.metaKey) && e.key === 'z' && canRollback) {
        e.preventDefault();
        onRollback();
      }
    };

    window.addEventListener('keydown', handleKeyDown);
    return () => window.removeEventListener('keydown', handleKeyDown);
  }, [canRollback, onRollback]);

  const getPlaceholder = () => {
    if (currentReview.type === 'meaning') {
      return "Enter the meaning...";
    } else {
      return "Enter the reading...";
    }
  };

  return (
    <div
      className={`card ${feedback === "correct" ? "card-correct" : ""} ${feedback === "incorrect" ? "card-incorrect" : ""} ${shake ? "card-shake" : ""}`}
    >
      <div className="card-inner">
        <div className="card-number">
          Remaining: {totalItems}
        </div>

        <div className="card-kanji-small">{currentKanji.kanji}</div>

        <div className="kanji-container">
          <span className="kanji">{currentReview.question}</span>
        </div>

        <form onSubmit={onSubmit} className="form">
          <div className="input-wrapper">
            <input
              ref={inputRef}
              type="text"
              value={userInput}
              onChange={(e) => onInputChange(e.target.value)}
              placeholder={getPlaceholder()}
              className={`input ${feedback === "correct" ? "input-correct" : ""} ${feedback === "incorrect" ? "input-incorrect" : ""}`}
              disabled={feedback !== null}
              autoFocus
            />
            <div className="input-underline"></div>
          </div>

          <div className="feedback-container">
            {feedback && (
              <div className={`feedback ${feedback === "correct" ? "feedback-correct" : "feedback-incorrect"}`}>
                {feedback === "correct" ? "正解！ Correct!" : "もう一度 Try again"}
              </div>
            )}
            {canRollback && (
              <button
                type="button"
                onClick={onRollback}
                className="rollback-button"
              >
                ↶ Undo Previous Answer
              </button>
            )}
          </div>
        </form>
      </div>

      <div className="seal">練</div>
    </div>
  );
}
