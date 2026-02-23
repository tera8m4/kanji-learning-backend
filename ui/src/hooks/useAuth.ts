import { useState } from "react";
import { getToken, clearToken } from "../core/auth";
import type { Transport, TelegramAuthPayload } from "../core/transport";

export function useAuth(transport: Transport) {
  const [isAuthenticated, setIsAuthenticated] = useState(() => !!getToken());

  const handleTelegramLogin = async (data: TelegramAuthPayload) => {
    await transport.login(data);
    setIsAuthenticated(true);
  };

  const logout = () => {
    clearToken();
    setIsAuthenticated(false);
  };

  return { isAuthenticated, handleTelegramLogin, logout };
}
