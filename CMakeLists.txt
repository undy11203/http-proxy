cmake_minimum_required(VERSION 3.28)
project(proxy LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
set(THREADS_PREFER_PTHREAD_FLAG TRUE)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Werror -g")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR})

find_package(Threads REQUIRED)

set(SRC_FILES
    main.cpp
    networks/socket.cpp
    networks/socketaddress.cpp
    http-parse/httpparse.cpp
    thread_pool/threadpool.cpp
    storage/storage.cpp
    storage/subscribemanager.cpp
)

add_executable(proxy ${SRC_FILES})

target_link_libraries(proxy PRIVATE Threads::Threads)