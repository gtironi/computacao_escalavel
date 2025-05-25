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

# Ensure python3-venv is installed globally
echo "Instalando python3-venv..."
sudo apt-get update
sudo apt-get install -y python3-venv

# Cria o venv se ainda não existir
if [ ! -d ".venv" ]; then
    $PYTHON -m venv .venv
fi

# Ativa o venv
source .venv/bin/activate

# Garante que o pip está disponível dentro do venv
if ! $PYTHON -m pip --version >/dev/null 2>&1; then
    echo "pip não encontrado no venv. Instalando com ensurepip..."
    $PYTHON -m ensurepip --upgrade
fi

# Atualiza pip e instala wheel, se necessário
$PYTHON -m pip install --upgrade pip wheel

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
