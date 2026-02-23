import "./App.css";
import LoadingScreen from "./LoadingScreen";
import CompletionScreen from "./CompletionScreen";
import Header from "./Header";
import FlashCard from "./FlashCard";
import LoginScreen from "./LoginScreen";
import { useKanjiReview } from "../hooks/useKanjiReview";
import { useAuth } from "../hooks/useAuth";
import type { Transport } from "../core/transport";

interface AppProps {
  transport: Transport;
}

export default function App({ transport }: AppProps) {
  const { isAuthenticated, handleTelegramLogin } = useAuth(transport);

  if (!isAuthenticated) {
    return <LoginScreen onLogin={handleTelegramLogin} />;
  }

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
  } = useKanjiReview(transport);

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
