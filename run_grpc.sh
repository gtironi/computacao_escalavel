#!/bin/bash
# Script para executar o cliente Python e o servidor C++

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

# Função para exibir menu
show_menu() {
    echo "==== MENU GRPC ====="
    echo "1. Instalar dependências (gRPC, Protobuf, C++, Python)"
    echo "2. Gerar stubs Python e C++"
    echo "3. Compilar servidor C++"
    echo "4. Executar servidor C++"
    echo "5. Executar cliente Python"
    echo "0. Sair"
    echo "===================="
}

# Gerar stubs Python e C++
generate_stubs() {
    echo "Gerando stubs Python e C++..."
    # Python stubs
    $PYTHON -m grpc_tools.protoc -I. --python_out=mock_client/proto --grpc_python_out=mock_client/proto extractor.proto
    # C++ stubs
    protoc -I./mock_client/proto --cpp_out=mock_client/proto --grpc_out=mock_client/proto --plugin=protoc-gen-grpc="$(which grpc_cpp_plugin)" mock_client/proto/extractor.proto
    echo "Stubs gerados com sucesso!"
}

# Compilar servidor C++
compile_cpp_server() {
    echo "Compilando servidor C++..."
    mkdir -p build
    cd build
    cmake ..
    make -j$(nproc)
    cd ..
    echo "Servidor C++ compilado com sucesso!"
}

# Executar servidor C++
run_cpp_server() {
    echo "Executando servidor C++..."
    ./build/grpc_server
}

# Executar cliente Python
run_python_client() {
    echo "Executando cliente Python..."
    export PYTHONPATH="$(pwd)"
    $PYTHON -m mock_client.client.grpc_client
}

# Instalar dependências
install_dependencies() {
    echo "Instalando dependências..."
    bash setup_grpc.sh
    echo "Dependências instaladas com sucesso!"
}

# Loop principal
while true; do
    show_menu
    read -p "Escolha uma opção: " choice
    case $choice in
        1) install_dependencies ;;
        2) generate_stubs ;;
        3) compile_cpp_server ;;
        4) run_cpp_server ;;
        5) run_python_client ;;
        0) echo "Saindo..."; exit 0 ;;
        *) echo "Opção inválida! Tente novamente." ;;
    esac
    echo
    read -p "Pressione Enter para continuar..."
    clear
done
