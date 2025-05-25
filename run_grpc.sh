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

# Ensure venv module is available
if ! $PYTHON -c "import venv" &>/dev/null; then
    echo "Módulo venv não encontrado. Instalando python3-venv..."
    sudo apt-get update && sudo apt-get install -y python3-venv
fi

# Create or activate virtualenv
if [ ! -d "venv" ]; then
    $PYTHON -m venv venv
fi
source venv/bin/activate

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
