import type { RequestAnswers, ResponseKanjis, TelegramAuthPayload } from "./transport";
import { setToken } from "./auth";

// Sample kanji data for testing
const SAMPLE_KANJIS: ResponseKanjis[] = [
  // Batch 1
  [
    {
      id: 1,
      kanji: "黒",
      meaning: "black",
      examples: [
        { word: "黒", reading: "くろ" },
        { word: "黒い", reading: "くろい" },
      ],
    },
    // {
    //   id: 2,
    //   kanji: "白",
    //   meaning: "white",
    //   examples: [
    //     { word: "白", reading: "しろ" },
    //     { word: "白い", reading: "しろい" },
    //   ],
    // },
    // {
    //   id: 3,
    //   kanji: "赤",
    //   meaning: "red",
    //   examples: [
    //     { word: "赤", reading: "あか" },
    //     { word: "赤い", reading: "あかい" },
    //   ],
    // },
  ],
  // Batch 2
  [
    {
      id: 4,
      kanji: "青",
      meaning: "blue",
      examples: [
        { word: "青", reading: "あお" },
        { word: "青い", reading: "あおい" },
      ],
    },
    {
      id: 5,
      kanji: "緑",
      meaning: "green",
      examples: [
        { word: "緑", reading: "みどり" },
      ],
    },
  ],
  // Batch 3
  [
    {
      id: 6,
      kanji: "山",
      meaning: "mountain",
      examples: [
        { word: "山", reading: "やま" },
        { word: "富士山", reading: "ふじさん" },
      ],
    },
    {
      id: 7,
      kanji: "川",
      meaning: "river",
      examples: [
        { word: "川", reading: "かわ" },
      ],
    },
  ],
];

export class MockTransport {
  private currentBatch = 0;
  private learnMoreRequested = false;

  public async login(_data: TelegramAuthPayload): Promise<void> {
    console.log("MockTransport: Login called");
    setToken("mock-token");
  }

  public async sendAnswers(req: RequestAnswers): Promise<void> {
    console.log("MockTransport: Sending answers", req);
    // Simulate network delay
    await new Promise(resolve => setTimeout(resolve, 300));
  }

  public async getKanjis(): Promise<ResponseKanjis> {
    console.log("MockTransport: Getting kanjis, batch", this.currentBatch);
    // Simulate network delay
    await new Promise(resolve => setTimeout(resolve, 500));

    if (this.currentBatch >= SAMPLE_KANJIS.length) {
      // No more kanjis available
      return [];
    }

    const batch = SAMPLE_KANJIS[this.currentBatch];

    // Only advance batch if learnMoreKanjis was called
    // (first call doesn't require learnMoreKanjis)
    if (this.currentBatch > 0 && !this.learnMoreRequested) {
      return [];
    }

    this.learnMoreRequested = false;
    return batch;
  }

  public async learnMoreKanjis(): Promise<void> {
    console.log("MockTransport: Learn more kanjis requested");
    // Simulate network delay
    await new Promise(resolve => setTimeout(resolve, 300));
    this.learnMoreRequested = true;
    this.currentBatch++;
  }
}
