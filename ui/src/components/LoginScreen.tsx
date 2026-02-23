import { useEffect, useRef } from "react";
import type { TelegramAuthPayload } from "../core/transport";
import "./LoginScreen.css";

interface LoginScreenProps {
  onLogin: (data: TelegramAuthPayload) => void;
}

export default function LoginScreen({ onLogin }: LoginScreenProps) {
  const widgetRef = useRef<HTMLDivElement>(null);

  useEffect(() => {
    // eslint-disable-next-line @typescript-eslint/no-explicit-any
    (window as any).onTelegramAuth = (data: TelegramAuthPayload) => onLogin(data);

    const script = document.createElement("script");
    script.src = "https://telegram.org/js/telegram-widget.js?22";
    script.setAttribute(
      "data-telegram-login",
      import.meta.env.VITE_TELEGRAM_BOT_USERNAME
    );
    script.setAttribute("data-size", "large");
    script.setAttribute("data-onauth", "onTelegramAuth(user)");
    script.setAttribute("data-request-access", "write");
    script.async = true;
    widgetRef.current?.appendChild(script);

    return () => {
      // eslint-disable-next-line @typescript-eslint/no-explicit-any
      delete (window as any).onTelegramAuth;
    };
  }, [onLogin]);

  return (
    <div className="login-container">
      <div className="ink-splatter-1"></div>
      <div className="ink-splatter-2"></div>

      <div className="login-card">
        <div className="login-header">
          <h1 className="login-title">漢字</h1>
          <p className="login-subtitle">Review</p>
        </div>

        <div className="login-divider"></div>

        <p className="login-prompt">Sign in with Telegram to continue</p>

        <div ref={widgetRef} className="telegram-widget-container"></div>

        <div className="login-seal">印</div>
      </div>
    </div>
  );
}
