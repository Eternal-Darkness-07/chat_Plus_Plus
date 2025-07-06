import React from 'react';
import { useNavigate } from 'react-router-dom';

const Welcome = () => {
  const navigate = useNavigate();

  const handleCreateRoom = () => {
    navigate('/createroom', { state: { mode: 'create' } });
  };

  const handleJoinRoom = () => {
    navigate('/createroom', { state: { mode: 'join' } });
  };

  return (
    <div className="flex items-center justify-center min-h-screen bg-gradient-to-br from-blue-100 via-purple-100 to-pink-100">
      <div className="bg-white/80 backdrop-blur-md px-10 py-12 rounded-3xl shadow-2xl min-w-[340px] flex flex-col items-center justify-center border border-gray-200">
        <h1 className="text-4xl font-extrabold mb-6 text-transparent bg-clip-text bg-gradient-to-r from-blue-600 via-purple-500 to-pink-500 drop-shadow-lg text-center">
          Welcome to Chat++
        </h1>
        <p className="mb-8 text-gray-600 text-sm text-center">Connect, chat, and collaborate in real time.</p>

        <div className="flex gap-6 justify-center w-full mt-4">
          <button
            className="px-7 py-3 rounded-lg bg-gradient-to-r from-blue-500 to-blue-700 text-white font-bold shadow-md hover:scale-105 transition-all duration-200"
            onClick={handleCreateRoom}
          >
            Create Room
          </button>
          <button
            className="px-7 py-3 rounded-lg bg-gradient-to-r from-green-500 to-green-700 text-white font-bold shadow-md hover:scale-105 transition-all duration-200"
            onClick={handleJoinRoom}
          >
            Join Room
          </button>
        </div>
      </div>
    </div>
  );
};

export default Welcome;
