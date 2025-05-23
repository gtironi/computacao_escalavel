cmake_minimum_required(VERSION 3.14)

project(grpc_client_example CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)

# Tell CMake where to find gRPC and protobuf
# Change this to your actual install path
set(CMAKE_PREFIX_PATH "${CMAKE_CURRENT_SOURCE_DIR}/grpc/install")

# Find gRPC and Protobuf
find_package(Protobuf CONFIG REQUIRED)
find_package(gRPC CONFIG REQUIRED)

# Proto sources
set(PROTO_SRC
  mock_server/proto/extractor.pb.cc
  mock_server/proto/extractor.grpc.pb.cc
)

# Main client sourcesudo
add_executable(grpc_server grpc_server.cpp ${PROTO_SRC})

# Include directories
target_include_directories(grpc_server PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/mock_server/proto
)

# Link gRPC, Protobuf
target_link_libraries(grpc_server
  gRPC::grpc++
  protobuf::libprotobuf
)
