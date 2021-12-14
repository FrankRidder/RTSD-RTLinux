cmake_minimum_required(VERSION 3.16)
project(RTSD_RTLinux C)

set(CMAKE_C_STANDARD 99)
set(THREADS_PREFER_PTHREAD_FLAG ON)

find_package(Threads REQUIRED)

add_executable(RTSD_RTLinux main.c)
target_link_libraries(RTSD_RTLinux PRIVATE Threads::Threads  m)
