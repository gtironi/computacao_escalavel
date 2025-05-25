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

# Configurar e compilar
mkdir -p build
cd build
cmake ..
make -j4

echo "Compilação concluída! Execute o servidor C++ com:"
echo "cd build && ./grpc_server"
