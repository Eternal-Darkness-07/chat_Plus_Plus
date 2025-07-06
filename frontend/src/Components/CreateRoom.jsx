import React, { useState, useEffect } from 'react';
import { createRoom, ActiveRoom } from '../../api/roomAPI';
import { useNavigate, useLocation } from 'react-router-dom';
import { FaRegCopy } from 'react-icons/fa';

const CreateRoom = () => {
  const navigate = useNavigate();
  const location = useLocation();
  const mode = location.state?.mode || 'create';

  const [roomId, setRoomId] = useState('');
  const [username, setUsername] = useState('');
  const [loading, setLoading] = useState(false);
  const [warning, setWarning] = useState('');
  const [copyMsg, setCopyMsg] = useState('');

  const showWarning = (msg) => {
    setWarning(msg);
    setTimeout(() => setWarning(''), 4000);
  };

  const handleGenerateRoomId = async () => {
    setLoading(true);
    try {
      const response = await createRoom();
      setRoomId(response.roomId);
    } catch (err) {
      console.error(err);
      showWarning('❌ Failed to generate room ID.');
    }
    setLoading(false);
  };

  const handleCopyRoomId = () => {
    if (roomId) {
      navigator.clipboard.writeText(roomId);
      setCopyMsg('✅ Copied!');
      setTimeout(() => setCopyMsg(''), 3000);
    }
  };

  const handleSubmit = async () => {
    if (!username.trim()) return showWarning('⚠️ Username is required');
    if (!roomId) return showWarning('⚠️ Room ID is required');

    setLoading(true);
    try {
      await ActiveRoom(roomId);
      navigate(`/chatroom/${roomId}`, { state: { username } });
    } catch (error) {
      showWarning('❌ Failed to join/activate room.');
    }
    setLoading(false);
  };

  useEffect(() => {
    if (mode === 'create') handleGenerateRoomId();
  }, [mode]);

  return (
    <div className="flex items-center justify-center min-h-screen bg-gradient-to-br from-blue-100 via-purple-100 to-pink-100">
      <div className="bg-white/80 backdrop-blur-md px-8 py-10 rounded-3xl shadow-2xl min-w-[340px] w-full max-w-md flex flex-col items-center border border-gray-200 space-y-6">
        <div className="w-full text-center">
          <h2 className="text-3xl font-extrabold mb-2 text-transparent bg-clip-text bg-gradient-to-r from-blue-600 via-purple-500 to-pink-500 drop-shadow-lg">
            {mode === 'create' ? 'Create a Room' : 'Join a Room'}
          </h2>
          <p className="text-gray-600 text-sm">
            {mode === 'create'
              ? 'Your unique room ID is generated.'
              : 'Enter Room ID and your name to join.'}
          </p>
        </div>

        {/* Warning */}
        {warning && (
          <div className="w-full px-4 py-3 rounded-md bg-yellow-100 text-yellow-800 font-semibold border border-yellow-300 text-sm shadow text-center">
            {warning}
          </div>
        )}

        {/* Username */}
        <div className="w-full">
          <label className="text-sm text-gray-700 font-semibold block mb-1">Your Name</label>
          <input
            type="text"
            value={username}
            onChange={(e) => setUsername(e.target.value)}
            placeholder="Enter your name"
            className="px-4 py-2 w-full border border-gray-300 rounded-lg focus:outline-none focus:ring-2 focus:ring-blue-400 shadow-sm"
          />
        </div>

        {/* Room ID */}
        {mode === 'create' ? (
          <>

            <div className="w-full">
              <label className="text-sm text-gray-700 font-semibold block mb-1">Room ID</label>
              <div className="flex gap-2">
                {/* Input: 80% width */}
                <input
                  type="text"
                  value={roomId}
                  readOnly
                  className="flex-[4] px-4 py-2 border border-gray-300 rounded-lg bg-gray-100 cursor-not-allowed focus:outline-none shadow-sm"
                />

                {/* Button: 20% width */}
                <button
                  onClick={handleCopyRoomId}
                  type="button"
                  className="flex-[1] text-sm px-2 py-2 rounded-lg bg-gray-200 hover:bg-gray-300 text-gray-700 font-semibold shadow"
                >
                  <FaRegCopy className="mx-auto" />
                </button>
              </div>

              {copyMsg && (
                <p className="text-green-600 text-xs font-medium mt-2">{copyMsg}</p>
              )}
            </div>
            <button
              onClick={handleGenerateRoomId}
              className="w-full px-6 py-3 rounded-lg bg-gradient-to-r from-blue-500 to-blue-700 text-white font-bold shadow-md hover:scale-105 transition-all duration-200"
              disabled={loading}
            >
              {loading ? 'Generating...' : 'Generate New Room ID'}
            </button>
          </>
        ) : (
          <div className="w-full">
            <label className="text-sm text-gray-700 font-semibold block mb-1">Room ID</label>
            <input
              type="text"
              value={roomId}
              onChange={(e) => setRoomId(e.target.value)}
              placeholder="Enter Room ID"
              className="px-4 py-2 w-full border border-gray-300 rounded-lg focus:outline-none focus:ring-2 focus:ring-blue-400 shadow-sm"
            />
          </div>
        )}

        {/* Submit Button */}
        <button
          onClick={handleSubmit}
          className="w-full px-6 py-3 rounded-lg bg-gradient-to-r from-green-500 to-green-700 text-white font-bold shadow-md hover:scale-105 transition-all duration-200"
          disabled={loading}
        >
          {loading ? 'Processing...' : mode === 'create' ? 'Activate Room' : 'Join Room'}
        </button>
      </div>
    </div>

  );
};

export default CreateRoom;
