const WEBSOCKET_URL = 'ws://localhost:8080/ws/chat';

export const connectWebSocket = (roomId, username, {
    onMessage,
    onOpen,
    onClose,
    onError,
    reconnect = false,
    reconnectDelay = 3000
} = {}) => {
    let ws;
    let shouldReconnect = reconnect;

    const url = `${WEBSOCKET_URL}?roomId=${encodeURIComponent(roomId)}&username=${encodeURIComponent(username)}`;

    const createWebSocket = () => {
        ws = new WebSocket(url);

        ws.onopen = () => {
            console.log("[WebSocket] Connected");
            if (onOpen) onOpen();
        };

        ws.onmessage = (event) => {
            try {
                const data = JSON.parse(event.data);
                if (onMessage) onMessage(data);
            } catch (err) {
                console.error("[WebSocket] Invalid JSON:", event.data);
            }
        };

        ws.onclose = (event) => {
            console.warn(`[WebSocket] Disconnected: ${event.code} (${event.reason})`);
            if (onClose) onClose(event);
            if (shouldReconnect) {
                console.log(`[WebSocket] Reconnecting in ${reconnectDelay}ms...`);
                setTimeout(createWebSocket, reconnectDelay);
            }
        };

        ws.onerror = (err) => {
            console.error("[WebSocket] Error:", err);
            if (onError) onError(err);
        };
    };

    createWebSocket();

    return {
        send: (msg) => {
            if (ws.readyState === WebSocket.OPEN) {
                ws.send(JSON.stringify(msg));
            } else {
                console.warn("[WebSocket] Message not sent. Ready state:", ws.readyState);
            }
        },
        close: () => {
            shouldReconnect = false;
            if (ws) ws.close();
        },
        raw: () => ws
    };
};
