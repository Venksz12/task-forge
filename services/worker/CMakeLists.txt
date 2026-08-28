add_executable(taskforge_worker src/main.cpp)
target_include_directories(taskforge_worker PRIVATE include)
target_link_libraries(taskforge_worker PRIVATE taskforge_common)

find_package(Boost 1.82 REQUIRED COMPONENTS system)
find_package(PostgreSQL REQUIRED)
target_link_libraries(taskforge_worker PRIVATE Boost::system PostgreSQL::PostgreSQL)
