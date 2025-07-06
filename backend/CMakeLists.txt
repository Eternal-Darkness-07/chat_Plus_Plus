cmake_minimum_required(VERSION 3.14)
project(chat_app)

set(CMAKE_CXX_STANDARD 17)

# Include your own header files (if any)
include_directories(${PROJECT_SOURCE_DIR}/include)

# Find and link Drogon
find_package(Drogon CONFIG REQUIRED)

# Add source files
add_executable(chat_app
    src/main.cpp
    src/Chat.cc
    src/Chat_websocket.cc
    src/CorsFilter.cc
)

# Link Drogon
target_link_libraries(chat_app PRIVATE Drogon::Drogon)

# Add Drogon include directories explicitly (sometimes needed)
target_include_directories(chat_app PRIVATE ${Drogon_INCLUDE_DIRS})
