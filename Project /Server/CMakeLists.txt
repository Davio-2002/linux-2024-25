cmake_minimum_required(VERSION 3.28)
project(Server)

set(CMAKE_CXX_STANDARD 17)

add_executable(Server main.cpp
        headers/Server.h
        headers/Session.h
        soruces/Server.cpp
        soruces/Session.cpp
)
