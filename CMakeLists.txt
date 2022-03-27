cmake_minimum_required(VERSION 3.21)
project(ipk_project_2_2022 C)

set(CMAKE_C_STANDARD 99)

add_executable(ipk_project_2_2022 ipk-simpleftp-server.c ipk-simpleftp-client.c serverConfig.h serverConfig.c clientConfig.h clientConfig.c)
