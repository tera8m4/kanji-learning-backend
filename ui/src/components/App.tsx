import "./App.css";
import LoadingScreen from "./LoadingScreen";
import CompletionScreen from "./CompletionScreen";
import Header from "./Header";
import FlashCard from "./FlashCard";
import { useKanjiReview } from "../hooks/useKanjiReview";

export default function App() {
  const {
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
    canRollback,
    handleRollback,
  } = useKanjiReview();

  if (isLoading) {
    return <LoadingScreen />;
  }

  if (!currentReview || !currentKanji) {
    return <CompletionScreen kanjis={kanjis} onLearnMore={handleLearnMore} />;
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
      />
    </div>
  );
}
