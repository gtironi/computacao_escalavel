cmake_minimum_required(VERSION 3.10)
project(grpc_client_example CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)

# Find gRPC and Protobuf
find_package(PkgConfig REQUIRED)
pkg_check_modules(GRPC REQUIRED grpc++)
pkg_check_modules(PROTOBUF REQUIRED protobuf)

# Proto sources
set(PROTO_SRC
  mock_client/proto/extractor.pb.cc
  mock_client/proto/extractor.grpc.pb.cc
)

# Main client source
add_executable(grpc_server grpc_server.cpp pipeline.cpp ${PROTO_SRC})

# Include directories
target_include_directories(grpc_server PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/mock_client/proto
  ${GRPC_INCLUDE_DIRS}
  ${PROTOBUF_INCLUDE_DIRS}
)

# Link gRPC, Protobuf
target_link_libraries(grpc_server
  ${GRPC_LIBRARIES}
  ${PROTOBUF_LIBRARIES}
  sqlite3
)
