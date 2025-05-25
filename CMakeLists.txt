cmake_minimum_required(VERSION 3.10)
project(grpc_client_example CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)

# Find gRPC and Protobuf
if(APPLE)
  message(STATUS "macOS detectado, usando find_package para Protobuf e gRPC")
  find_package(Protobuf REQUIRED)
  find_package(gRPC CONFIG REQUIRED)
  set(GRPC_LIBS gRPC::grpc++)
  set(PROTOBUF_LIBS ${Protobuf_LIBRARIES})
  set(PROTO_INCLUDE_DIRS ${Protobuf_INCLUDE_DIRS})
else()
  find_package(PkgConfig REQUIRED)
  pkg_check_modules(GRPC REQUIRED grpc++)
  pkg_check_modules(PROTOBUF REQUIRED protobuf)
  set(GRPC_LIBS ${GRPC_LIBRARIES})
  set(PROTOBUF_LIBS ${PROTOBUF_LIBRARIES})
  set(PROTO_INCLUDE_DIRS ${PROTOBUF_INCLUDE_DIRS})
endif()

# Proto sources
set(PROTO_SRC
  mock_client/proto/extractor.pb.cc
  mock_client/proto/extractor.grpc.pb.cc
)

# Main client source
add_executable(grpc_server
  grpc_server.cpp
  pipeline.cpp    # implementa pipeline()
  ${PROTO_SRC}
)

# Include directories
target_include_directories(grpc_server PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/mock_client/proto
  ${PROTO_INCLUDE_DIRS}
)

# Link gRPC, Protobuf
target_link_libraries(grpc_server
  ${GRPC_LIBS}
  ${PROTOBUF_LIBS}
  sqlite3
  pthread
)
