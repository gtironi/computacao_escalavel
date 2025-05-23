#!/bin/bash
# Script para instalação e configuração do gRPC e Protobuf
# Dependências básicas
sudo apt-get update
sudo apt-get install -y build-essential autoconf libtool pkg-config cmake git

# Instalar gRPC e Protobuf através do sistema - método mais simples
sudo apt-get install -y libgrpc++-dev libgrpc-dev protobuf-compiler-grpc libprotobuf-dev

# Gerar stubs de protocolo
# Python stubs
python -m pip install grpcio grpcio-tools protobuf
python -m grpc_tools.protoc -I. --python_out=. --grpc_python_out=. mock_client/proto/extractor.proto

# C++ stubs
protoc -I./mock_client/proto --cpp_out=mock_client/proto --grpc_out=mock_client/proto --plugin=protoc-gen-grpc="$(which grpc_cpp_plugin)" ./mock_client/proto/extractor.proto

# Atualizar CMakeLists.txt para usar o gRPC instalado pelo sistema
cat > CMakeLists.txt << 'EOF'
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
add_executable(grpc_server grpc_server.cpp ${PROTO_SRC})

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
)
EOF

# Configurar e compilar
mkdir -p build
cd build
cmake ..
make -j4

echo "Compilação concluída! Execute o servidor C++ com:"
echo "cd build && ./grpc_server"
