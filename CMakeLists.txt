cmake_minimum_required(VERSION 3.18)
project(gpu_work_queue_lab LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(scheduler src/scheduler.cpp)
target_include_directories(scheduler PUBLIC include)
target_compile_options(scheduler PRIVATE -Wall -Wextra -Wpedantic)

add_executable(work_queue_demo src/main.cpp)
target_link_libraries(work_queue_demo PRIVATE scheduler)
target_compile_options(work_queue_demo PRIVATE -Wall -Wextra -Wpedantic)

include(CTest)
if(BUILD_TESTING)
  add_executable(scheduler_tests tests/scheduler_tests.cpp)
  target_link_libraries(scheduler_tests PRIVATE scheduler)
  target_compile_options(scheduler_tests PRIVATE -Wall -Wextra -Wpedantic)
  add_test(NAME scheduler_tests COMMAND scheduler_tests)
endif()
