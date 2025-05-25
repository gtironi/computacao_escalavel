#!/bin/bash
# Script para instalação e configuração do gRPC e Protobuf

# Detect Python interpreter
if command -v python3 >/dev/null 2>&1; then
    PYTHON=python3
elif command -v python >/dev/null 2>&1; then
    PYTHON=python
else
    echo "Erro: Python não encontrado." >&2
    exit 1
fi

# Ensure pip is available
if ! command -v pip >/dev/null 2>&1; then
    echo "pip não encontrado. Instalando python3-pip..."
    sudo apt-get update && sudo apt-get install -y python3-pip
fi

# Ensure venv module is available
if ! command -v $PYTHON -m venv --help >/dev/null 2>&1; then
    echo "Módulo venv não encontrado. Instalando python3-venv..."
    sudo apt-get update && sudo apt-get install -y python3-venv
fi

# Create venv if not exists
if [ ! -d "venv" ]; then
    $PYTHON -m venv .venv
fi

# Activate venv
source .venv/bin/activate

# Upgrade pip and install Python dependencies
$PYTHON -m pip install --upgrade pip
$PYTHON -m pip install -r requirements.txt grpcio grpcio-tools

# Install system dependencies
sudo apt-get update
sudo apt-get install -y build-essential autoconf libtool pkg-config cmake git libsqlite3-dev libgrpc++-dev libgrpc-dev protobuf-compiler-grpc libprotobuf-dev

# Gerar stubs de protocolo
# Python stubs
$PYTHON -m grpc_tools.protoc -I. --python_out=. --grpc_python_out=. mock_client/proto/extractor.proto

# C++ stubs
protoc -I./mock_client/proto --cpp_out=mock_client/proto --grpc_out=mock_client/proto --plugin=protoc-gen-grpc="$(which grpc_cpp_plugin)" ./mock_client/proto/extractor.proto

# Configurar e compilar
mkdir -p build
cd build
cmake ..
make -j$(nproc)
cd ..

echo "Compilação concluída! Para rodar, use run_grpc.sh e selecione as opções desejadas."
