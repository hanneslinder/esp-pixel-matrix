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

type ConnectionListener = (isConnected: boolean) => void;

class WebsocketConnection {
  private websocket: WebSocket = new WebSocket(gateway);
  private messageListeners: MessageListenerMap = {};
  private connectionListeners: ConnectionListener[] = [];
  private connected = false;

  constructor() {
    this.websocket.onopen = this.handleOpen;
    this.websocket.onclose = this.handleClose;
    this.websocket.onmessage = this.handleMessage;
  }

  public get readyState(): number {
    return this.websocket.readyState;
  }

  private readonly handleOpen = () => {
    console.log("Connection opened");
    this.connected = true;
    this.connectionListeners.forEach((listener) => listener(true));
  };

  private readonly handleClose = () => {
    console.log("Connection closed");
    this.connected = false;
    this.connectionListeners.forEach((listener) => listener(false));
  };

  private readonly handleMessage = (event: MessageEvent) => {
    const data = JSON.parse(event.data);

    // console.log("Incomming message", data);

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
        (listener) => listener === cb,
      ));
  }

  public subscribeToConnection(cb: ConnectionListener): () => void {
    this.connectionListeners.push(cb);
    cb(this.connected);

    return () => {
      this.connectionListeners = this.connectionListeners.filter(
        (listener) => listener !== cb,
      );
    };
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
