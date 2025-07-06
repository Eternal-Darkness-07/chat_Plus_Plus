import { BrowserRouter as Router, Routes, Route } from 'react-router-dom';
import Welcome from './Components/Welcome';
import CreateRoom from './Components/CreateRoom';
import ChatRoom from './Components/ChatRoom';

function App() {
  return (
    <Router>
      <Routes>
        <Route path="/" element={<Welcome />} />
        <Route path="/createroom" element={<CreateRoom />} />
        <Route path="/chatroom/:RoomId" element={<ChatRoom />} />
      </Routes>
    </Router>
  );
}

export default App;