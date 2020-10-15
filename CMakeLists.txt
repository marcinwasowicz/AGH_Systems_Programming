cmake_minimum_required(VERSION 3.17)
project(System_Programming C)

set(CMAKE_C_STANDARD 99)

add_executable(Classes1_task2 Classes1/Task2.c)
target_link_libraries(Classes1_task2 pthread)

add_executable(Classes1_task1 Classes1/Task1.c)
target_link_libraries(Classes1_task1 pthread)

add_executable(Classes1_task3 Classes1/Task3.c)
target_link_libraries(Classes1_task3 pthread)