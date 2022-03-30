cmake_minimum_required(VERSION 3.21)
project(ipk_project_2_2022 C)

set(CMAKE_C_STANDARD 99)

add_executable(ipk_project_2_2022 server/ipk-simpleftp-server.c client/ipk-simpleftp-client.c server/serverConfig.h server/serverConfig.c client/clientConfig.h client/clientConfig.c server/auth.h server/auth.c)
