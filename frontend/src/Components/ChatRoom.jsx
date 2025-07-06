import React, { useState, useRef, useEffect } from 'react';
import { useParams, useNavigate, useLocation } from 'react-router-dom';
import { connectWebSocket } from '../../api/chatWebSocket';

const ChatRoom = () => {
  const { RoomId } = useParams();
  const navigate = useNavigate();
  const location = useLocation();
  const username = location.state?.username || sessionStorage.getItem('username') || 'Guest';

  const inputRef = useRef(null);
  const messagesEndRef = useRef(null);
  const wsRef = useRef(null);
  const connectedRef = useRef(false);

  // Load messages from sessionStorage
  const storedMessages = JSON.parse(sessionStorage.getItem('chatMessages')) || [
    { sender: 'system', text: `Welcome to Room ${username}` }
  ];
  const [messages, setMessages] = useState(storedMessages);
  const [input, setInput] = useState('');

  // Save username and roomId in sessionStorage
  useEffect(() => {
    sessionStorage.setItem('username', username);
    sessionStorage.setItem('roomId', RoomId);
  }, [username, RoomId]);

  // Scroll to bottom on new message
  useEffect(() => {
    messagesEndRef.current?.scrollIntoView({ behavior: 'smooth' });
    sessionStorage.setItem('chatMessages', JSON.stringify(messages));
  }, [messages]);

  // Connect WebSocket on mount
  useEffect(() => {
    if (connectedRef.current) return;
    connectedRef.current = true;

    const ws = connectWebSocket(RoomId, username, {
      onMessage: (msg) => {
        if (msg.type === 'info') {
          if (msg.event === 'join') {
            setMessages(prev => [...prev, {
              sender: 'system',
              text: `${msg.user} joined the room.`
            }]);
          } else if (msg.event === 'leave') {
            setMessages(prev => [...prev, {
              sender: 'system',
              text: `${msg.user} left the room.`
            }]);
          } else if (msg.event === 'joined') {
            setMessages(prev => [...prev, {
              sender: 'system',
              text: `You joined Room ${msg.room}`
            }]);
          }
        } else if (msg.type === 'chat') {
          const isSelf = msg.user === username;
          setMessages(prev => [...prev, {
            sender: isSelf ? 'user' : 'other',
            text: msg.message,
            user: msg.user
          }]);
        }
      },
      onClose: () => {
        setMessages(prev => [...prev, {
          sender: 'system',
          text: 'Disconnected from server.'
        }]);
      },
      onError: () => {
        setMessages(prev => [...prev, {
          sender: 'system',
          text: 'Connection error occurred.'
        }]);
      },
      reconnect: true
    });

    wsRef.current = ws;

    // Handle cleanup on tab close/navigation
    const handleUnload = () => {
      wsRef.current?.close();
    };

    window.addEventListener('beforeunload', handleUnload);

    return () => {
      window.removeEventListener('beforeunload', handleUnload);
    };
  }, [RoomId, username]);

  const handleSend = (e) => {
    e.preventDefault();
    if (input.trim() === '') return;

    wsRef.current?.send({
      type: 'chat',
      message: input.trim()
    });

    setInput('');
    if (inputRef.current) inputRef.current.style.height = '48px';
  };

  const handleInputChange = (e) => {
    setInput(e.target.value);
    if (inputRef.current) {
      inputRef.current.style.height = '48px';
      inputRef.current.style.height = Math.min(inputRef.current.scrollHeight, 160) + 'px';
    }
  };

  const handleLeaveRoom = () => {
    wsRef.current?.send({ 
      type: 'leave',
      user: username,
      room: RoomId
     });
    wsRef.current?.close();
    connectedRef.current = false;
    sessionStorage.removeItem('chatMessages');
    sessionStorage.removeItem('username');
    sessionStorage.removeItem('roomId');
    navigate('/');
  };

  return (
    <div className="flex items-center justify-center min-h-screen bg-gradient-to-br from-blue-100 via-purple-100 to-pink-100">
      <div className="bg-white/90 backdrop-blur-md rounded-3xl shadow-2xl w-full max-w-4xl h-screen flex flex-col border border-gray-200">
        {/* Header */}
        <div className="px-8 py-6 border-b border-gray-200 flex items-center justify-between rounded-t-3xl bg-gradient-to-r from-blue-600 via-purple-500 to-pink-500">
          <div>
            <h2 className="text-2xl font-bold text-white drop-shadow">Chat Room</h2>
            <span className="text-sm text-white/80">Room ID: <b>{RoomId}</b></span>
          </div>
          <button
            onClick={handleLeaveRoom}
            className="px-4 py-2 bg-white/20 hover:bg-white/30 text-white text-sm font-semibold rounded-lg shadow transition-all duration-200"
          >
            Leave Room
          </button>
        </div>

        {/* Messages */}
        <div className="flex-1 overflow-y-auto px-8 py-6 space-y-4 bg-white scrollbar-custom">
          {messages.map((msg, idx) => (
            <div
              key={idx}
              className={
                msg.sender === 'system'
                  ? 'flex justify-center'
                  : msg.sender === 'user'
                  ? 'flex justify-end'
                  : 'flex justify-start'
              }
            >
              <div
                className={`px-4 py-2 rounded-lg shadow
                  ${
                    msg.sender === 'user'
                      ? 'bg-blue-500 text-white rounded-br-none'
                      : msg.sender === 'system'
                      ? 'bg-gray-100 text-gray-500 font-semibold text-center'
                      : 'bg-gray-200 text-gray-800 rounded-bl-none'
                  }`}
                style={{
                  maxWidth: '70%',
                  overflowWrap: 'anywhere',
                  wordBreak: 'break-word',
                  whiteSpace: 'pre-wrap',
                  overflowX: 'hidden',
                  ...(msg.sender === 'system' ? { marginLeft: 'auto', marginRight: 'auto' } : {})
                }}
              >
                {msg.sender === 'other' && (
                  <div className="text-xs font-semibold text-gray-600 mb-1">{msg.user}</div>
                )}
                {msg.text}
              </div>
            </div>
          ))}
          <div ref={messagesEndRef} />
        </div>

        {/* Input */}
        <form
          onSubmit={handleSend}
          className="px-8 py-4 border-t border-gray-200 bg-white rounded-b-3xl flex gap-3"
        >
          <textarea
            ref={inputRef}
            rows={2}
            style={{ minHeight: '48px', maxHeight: '160px', resize: 'none' }}
            className="flex-1 px-4 py-2 rounded-lg border border-gray-300 focus:outline-none focus:ring-2 focus:ring-blue-400 text-base overflow-auto scrollbar-custom"
            placeholder="Type your message..."
            value={input}
            onChange={handleInputChange}
          />
          <button
            type="submit"
            className="px-6 py-2 rounded-lg bg-gradient-to-r from-blue-500 to-blue-700 text-white font-bold shadow hover:scale-105 transition-all duration-200"
          >
            Send
          </button>
        </form>
      </div>
    </div>
  );
};

export default ChatRoom;
