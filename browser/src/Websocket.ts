import { getPixelsAction, getStateAction } from "./Actions";
import { appState } from "./state/appState";

const ip = (window as any).websocketUrl;
const gateway = `ws://${ip}/ws`;

interface Message {
  action: string;
  [key: string]: any;
}

export enum IncommingMessageType {
  MatrixPixelResponse = "matrixPixels",
  MatrixSettingsResponse = "matrixSettings",
  UpdateProgress = "updateProgress",
}

interface MessageListenerMap {
  [key: string]: Function[];
}

class WebsocketConnection {
  private websocket: WebSocket = new WebSocket(gateway);
  private messageListeners: MessageListenerMap = {};

  constructor() {
    this.websocket.onopen = this.onOpen;
    this.websocket.onclose = this.onClose;
    this.websocket.onmessage = this.onMessage;
  }

  private readonly onOpen = (event: Event) => {
    console.log("Connection opened");
    appState.connection.state = this.websocket.readyState;

    // Sync remote state on connection open
    getPixelsAction();
    getStateAction();
  };

  private readonly onClose = (event: Event) => {
    console.log("Connection closed");
    appState.connection.state = this.websocket.readyState;
  };

  private readonly onMessage = (event: MessageEvent) => {
    const data = JSON.parse(event.data);

    console.log("Incomming message", data);

    if (this.messageListeners[data.action]) {
      this.messageListeners[data.action].forEach((listener) => listener(data));
    }
  };

  public subscribe(action: IncommingMessageType, cb: Function): () => void {
    if (!this.messageListeners[action]) {
      this.messageListeners[action] = [];
    }

    this.messageListeners[action].push(cb);

    // Unsubscribe
    return () =>
      (this.messageListeners[action] = this.messageListeners[action].filter(
        (listener) => listener === cb
      ));
  }

  public send(message: Message): void {
    console.log("Send out", message);
    this.websocket.send(JSON.stringify(message));
  }
}

let socket: WebsocketConnection;

export const getSocket = () => {
  if (socket == undefined) {
    socket = new WebsocketConnection();
  }

  return socket;
};
