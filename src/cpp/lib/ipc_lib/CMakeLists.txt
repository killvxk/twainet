cmake_minimum_required(VERSION 2.6)
project(ipc_lib)

execute_process(COMMAND protoc --proto_path=${CMAKE_CURRENT_SOURCE_DIR}/../../../messages --cpp_out=${CMAKE_CURRENT_SOURCE_DIR}/../../../messages ${CMAKE_CURRENT_SOURCE_DIR}/../../../messages/ipc.proto
		WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug")
#    set(CMAKE_BUILD_TYPE "Release")
endif(NOT CMAKE_BUILD_TYPE)

set (SRC_${PROJECT_NAME}
    ../../../messages/ipc.pb.cc
    ../../common/tstring.cpp
    ../../utils/utils.cpp
    connector/ipc_connector.cpp
    module/ipc_module.cpp
    module/ipc_object_name.cpp
    thread/ipc_checker_thread.cpp
    thread/ping_thread.cpp
)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}
		    ${CMAKE_CURRENT_SOURCE_DIR}/..
		    ${CMAKE_CURRENT_SOURCE_DIR}/../..
		    ${CMAKE_CURRENT_SOURCE_DIR}/../../external/protobuf-2.5.0/src
		    ${CMAKE_CURRENT_SOURCE_DIR}/../../../messages)

add_library (${PROJECT_NAME} STATIC ${SRC_${PROJECT_NAME}})
set(CMAKE_LIBRARY_OUTPUT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../../../output/${PROJECT_NAME})
set(LIBRARY_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../../../../output/lib/${CMAKE_BUILD_TYPE})