# Chat++ Application

A real-time chat application with a C++ (Drogon) backend and a React (Vite) frontend.

---

## Requirements

### Backend (C++ Drogon)
- C++17 compatible compiler (e.g., g++ 9+ or clang++)
- [CMake](https://cmake.org/) >= 3.14
- [Drogon](https://github.com/drogonframework/drogon) (C++ web framework)
- [jsoncpp](https://github.com/open-source-parsers/jsoncpp) (for JSON parsing)
- (Linux) `build-essential`, `libjsoncpp-dev`, `libssl-dev`, `uuid-dev`, `zlib1g-dev`

### Frontend (React + Vite)
- [Node.js](https://nodejs.org/) >= 18.x
- [npm](https://www.npmjs.com/) >= 9.x

---

## Libraries Used

### Backend
- [Drogon](https://github.com/drogonframework/drogon)
- [jsoncpp](https://github.com/open-source-parsers/jsoncpp)

### Frontend
- [React](https://react.dev/)
- [Vite](https://vitejs.dev/)
- [Tailwind CSS](https://tailwindcss.com/)
- [Axios](https://axios-http.com/)
- [React Router](https://reactrouter.com/)
- [React Icons](https://react-icons.github.io/react-icons/)

---

## Setup Instructions

### 1. Clone the Repository

```sh
git clone <your-repo-url>
cd chat_application
```

---

### 2. Backend Setup (C++ Drogon)

#### **Install Drogon and dependencies**

**On Ubuntu/Debian:**
```sh
sudo apt update
sudo apt install -y build-essential cmake libjsoncpp-dev libssl-dev uuid-dev zlib1g-dev
# Install Drogon (see https://github.com/drogonframework/drogon for latest instructions)
git clone https://github.com/drogonframework/drogon.git
cd drogon
git submodule update --init
mkdir build && cd build
cmake ..
make -j$(nproc)
sudo make install
cd ../..
```

#### **Build the backend**

```sh
cd backend
mkdir -p build
cd build
cmake ..
make
```

#### **Run the backend server**

```sh
./chat_app
```

The backend will start at [http://127.0.0.1:8080](http://127.0.0.1:8080).

---

### 3. Frontend Setup (React + Vite)

```sh
cd frontend
npm install
```

#### **Run the frontend development server**

```sh
npm run dev
```

The frontend will start at [http://localhost:5173](http://localhost:5173) (default Vite port).

---

## Usage

1. **Start the backend server** (see above).
2. **Start the frontend dev server** (see above).
3. Open your browser and go to [http://localhost:5173](http://localhost:5173).
4. Create or join a chat room and start chatting!

---

## Project Structure

```
chat_application/
  backend/
    include/
    src/
    build/
    CMakeLists.txt
  frontend/
    src/
    api/
    public/
    package.json
    vite.config.js
  .gitignore
  README.md
```

---

## Troubleshooting

- Make sure the backend is running on port 8080 before starting the frontend.
- If you change backend ports, update the API URLs in [`frontend/api/roomAPI.js`](frontend/api/roomAPI.js) and [`frontend/api/chatWebSocket.js`](frontend/api/chatWebSocket.js).
- For Drogon installation help, see [Drogon Docs](https://github.com/drogonframework/drogon).

---