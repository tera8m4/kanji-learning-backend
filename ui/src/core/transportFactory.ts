import { Transport } from "./transport";
import { MockTransport } from "./mockTransport";

export function createTransport() {
  if (import.meta.env.VITE_USE_MOCK) {
    console.log("Using MockTransport");
    return new MockTransport();
  }

  console.log("Using real Transport");
  return new Transport();
}
