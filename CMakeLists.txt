cmake_minimum_required(VERSION 3.14)

project(grpc_client_example CXX)

# Padrão do projeto
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

# Inclui o arquivo que define _GRPC_GRPCPP, _PROTOBUF_LIBPROTOBUF, etc.
include(cmake/common.cmake)

# Gera os arquivos .pb.cc e .grpc.pb.cc (ajuste o caminho do .proto conforme necessário)
get_filename_component(proto_file "mock_client/proto/extractor.proto" ABSOLUTE)
get_filename_component(proto_path "${proto_file}" PATH)

set(proto_src "${CMAKE_CURRENT_BINARY_DIR}/extractor.pb.cc")
set(proto_hdr "${CMAKE_CURRENT_BINARY_DIR}/extractor.pb.h")
set(grpc_src "${CMAKE_CURRENT_BINARY_DIR}/extractor.grpc.pb.cc")
set(grpc_hdr "${CMAKE_CURRENT_BINARY_DIR}/extractor.grpc.pb.h")

add_custom_command(
    OUTPUT "${proto_src}" "${proto_hdr}" "${grpc_src}" "${grpc_hdr}"
    COMMAND ${_PROTOBUF_PROTOC}
    ARGS --grpc_out "${CMAKE_CURRENT_BINARY_DIR}"
         --cpp_out "${CMAKE_CURRENT_BINARY_DIR}"
         -I "${proto_path}"
         --plugin=protoc-gen-grpc="${_GRPC_CPP_PLUGIN_EXECUTABLE}"
         "${proto_file}"
    DEPENDS "${proto_file}"
)

# Biblioteca contendo os arquivos gerados
add_library(proto_files
    ${proto_src}
    ${proto_hdr}
    ${grpc_src}
    ${grpc_hdr}
)

target_link_libraries(proto_files
    ${_PROTOBUF_LIBPROTOBUF}
    ${_GRPC_GRPCPP}
)

# Inclui os headers gerados
target_include_directories(proto_files PUBLIC "${CMAKE_CURRENT_BINARY_DIR}")

enable_language(C)
add_library(sqlite3 STATIC deps/sqlite/sqlite3.c)
target_include_directories(sqlite3 PUBLIC deps/sqlite)

# Seu executável
add_executable(grpc_server grpc_server.cpp pipeline.cpp )

# Linka seu executável à biblioteca de proto
target_link_libraries(grpc_server proto_files sqlite3)