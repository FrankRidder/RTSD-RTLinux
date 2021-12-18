cmake_minimum_required(VERSION 3.6)
project(RTSD_RTLinux C)


set(CMAKE_C_STANDARD 99)
set(THREADS_PREFER_PTHREAD_FLAG ON)

find_library(LIBRT rt)
find_package(Threads REQUIRED)

add_executable(RTSD_RTLinux main.c)
target_link_libraries(RTSD_RTLinux PRIVATE Threads::Threads ${LIBRT} m)

target_compile_options(RTSD_RTLinux PRIVATE -I/usr/xenomai/include/cobalt -I/usr/xenomai/include -D_GNU_SOURCE -D_REENTRANT -fasynchronous-unwind-tables -DCOBALT -DCOBALT_WRAP -Wl,--no-as-needed -Wl,@/usr/xenomai/lib/cobalt.wrappers -Wl,@/usr/xenomai/lib/modechk.wrappers  /usr/xenomai/lib/xenomai/bootstrap.o -Wl,--wrap=main -Wl,--dynamic-list=/usr/xenomai/lib/dynlist.ld -L/usr/xenomai/lib -lcobalt -lmodechk -lpthread -lrt)

