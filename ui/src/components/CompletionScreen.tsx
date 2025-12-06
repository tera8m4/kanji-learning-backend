import type { KanjiState } from "./types";

type CompletionScreenProps = {
  kanjis: KanjiState[];
};

export default function CompletionScreen({ kanjis }: CompletionScreenProps) {
  const correctCount = kanjis.filter(k => k.incorrectStreak === 0).length;
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
