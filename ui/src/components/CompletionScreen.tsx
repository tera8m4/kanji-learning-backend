import { useNavigate } from "react-router-dom";
import type { KanjiState } from "./types";

type CompletionScreenProps = {
  kanjis: KanjiState[];
  onLearnMore?: () => void;
};

export default function CompletionScreen({ kanjis, onLearnMore }: CompletionScreenProps) {
  const navigate = useNavigate();
  const correctCount = kanjis.filter(k => k.incorrectStreak === 0).length;
  const totalCount = kanjis.length;

  return (
    <div className="container">
      <header className="header">
        <h1 className="title">完了！</h1>
        <p className="subtitle">Review Complete</p>
      </header>
      <div className="score-container">
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
      {onLearnMore && (
        <button onClick={onLearnMore} className="learn-more-button">
          Learn More Kanjis Today
        </button>
      )}
      <button onClick={() => navigate("/list")} className="learn-more-button" style={{ background: 'transparent', color: '#8b7355', border: '2px solid #d4c4b0', boxShadow: 'none' }}>
        View Kanji
      </button>
    </div>
  );
}
