cmake_minimum_required(VERSION 3.17)
project(comp30023_2021_project_2_master C)

set(CMAKE_C_STANDARD 11)

include_directories(.)

add_executable(comp30023_2021_project_2_master
        helper1.c
        helper1.h
        main.c
        phase1.c test_sender.c network_handler.c network_handler.h utils.c utils.h message_handler.c message_handler.h)
