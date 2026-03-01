import LoadingScreen from "./LoadingScreen";
import CompletionScreen from "./CompletionScreen";
import Header from "./Header";
import FlashCard from "./FlashCard";
import { useKanjiReview } from "../hooks/useKanjiReview";
import type { Transport } from "../core/transport";

interface ReviewScreenProps {
  transport: Transport;
}

export default function ReviewScreen({ transport }: ReviewScreenProps) {
  const {
    kanjis,
    reviewDeck,
    currentReview,
    totalPending,
    currentKanji,
    userInput,
    setUserInput,
    feedback,
    shake,
    isLoading,
    handleSubmit,
    handleLearnMore,
    canRollback,
    handleRollback,
    handleContinue,
  } = useKanjiReview(transport);

  if (isLoading) {
    return <LoadingScreen />;
  }

  if (!currentReview || !currentKanji) {
    return (
      <CompletionScreen
        kanjis={kanjis}
        onLearnMore={handleLearnMore}
      />
    );
  }

  return (
    <div className="container">
      <div className="ink-splatter-1"></div>
      <div className="ink-splatter-2"></div>

      <Header />

      <FlashCard
        currentReview={currentReview}
        currentKanji={currentKanji}
        totalItems={reviewDeck.length}
        userInput={userInput}
        feedback={feedback}
        shake={shake}
        canRollback={canRollback}
        onInputChange={setUserInput}
        onSubmit={handleSubmit}
        onRollback={handleRollback}
        onContinue={handleContinue}
      />

      <div className="score-container">
        <div className="score-box">
          <span className="score-value">{totalPending}</span>
          <span className="score-label">kanji</span>
        </div>
      </div>
    </div>
  );
}
