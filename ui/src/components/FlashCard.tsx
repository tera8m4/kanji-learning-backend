import { useRef, useEffect } from "react";
import type { KanjiState, ReviewItem } from "./types";

type FlashCardProps = {
  currentReview: ReviewItem;
  currentKanji: KanjiState;
  totalItems: number;
  userInput: string;
  feedback: string | null;
  shake: boolean;
  onInputChange: (value: string) => void;
  onSubmit: (e: React.FormEvent) => void;
};

export default function FlashCard({
  currentReview,
  currentKanji,
  totalItems,
  userInput,
  feedback,
  shake,
  onInputChange,
  onSubmit,
}: FlashCardProps) {
  const inputRef = useRef<HTMLInputElement>(null);

  useEffect(() => {
    if (!feedback && inputRef.current) {
      inputRef.current.focus();
    }
  }, [feedback]);

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
          </div>
        </form>
      </div>

      <div className="seal">練</div>
    </div>
  );
}
