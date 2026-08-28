add_executable(taskforge_scheduler src/main.cpp)
target_include_directories(taskforge_scheduler PRIVATE include)
target_link_libraries(taskforge_scheduler PRIVATE taskforge_common)

find_package(Boost 1.82 REQUIRED COMPONENTS system)
find_package(PostgreSQL REQUIRED)
target_link_libraries(taskforge_scheduler PRIVATE Boost::system PostgreSQL::PostgreSQL)
