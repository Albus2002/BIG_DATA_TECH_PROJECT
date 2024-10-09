cmake_minimum_required(VERSION 2.8)

project(miniTAP)

# Prefer pthreads
find_package(Threads REQUIRED)

set(THREADS_PREFER_PTHREAD_FLAG ON)
set(THREADS_PREFER_PTHREAD_FLAG ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")

set(CMAKE_CXX_STANDARD 20)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src/include)

aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/src/. DIR_SRC)

# add_subdirectory(contribs/clickhouse-cpp)

add_executable(${PROJECT_NAME} ${DIR_SRC})

# Find and link threads
find_package(Threads REQUIRED)
target_link_libraries(${PROJECT_NAME} Threads::Threads)

# target_include_directories(${PROJECT_NAME} PRIVATE contribs/clickhouse-cpp/ contribs/clickhouse-cpp/contrib/absl)
# target_link_libraries(${PROJECT_NAME} PRIVATE clickhouse-cpp-lib)