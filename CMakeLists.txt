cmake_minimum_required(VERSION 3.28)
project(proxy LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(Boost REQUIRED)
find_package(asio REQUIRED)

# Исходники
set(SRC_FILES
    main.cpp
)

add_executable(proxy ${SRC_FILES})

# Линковка
target_link_libraries(${PROJECT_NAME} PRIVATE
    boost::boost
    asio::asio
)

