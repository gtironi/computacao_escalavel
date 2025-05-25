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

# Detect OS (Linux or macOS)
OS=$(uname)

# Instalar dependências Python e venv
echo "Instalando dependências Python..."
if [ "$OS" = "Darwin" ]; then
    brew update
    brew install python3
else
    # Linux: instalar python3-venv
    sudo apt-get update
    sudo apt-get install -y python3-venv
fi

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
$PYTHON -m pip install -r requirements.txt grpcio grpcio-tools

# Install system dependencies
echo "Instalando dependências do sistema..."
if [ "$OS" = "Darwin" ]; then
    # macOS via Homebrew
    brew update
    brew install cmake git pkg-config sqlite3 autoconf libtool grpc protobuf
else
    sudo apt-get update
    sudo apt-get install -y build-essential autoconf libtool pkg-config cmake git libsqlite3-dev libgrpc++-dev libgrpc-dev protobuf-compiler-grpc libprotobuf-dev
fi

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
