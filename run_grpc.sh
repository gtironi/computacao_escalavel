#!/bin/bash
# Script para executar o cliente Python e o servidor C++

# Função para exibir menu
show_menu() {
    echo "==== MENU GRPC ====="
    echo "1. Instalar dependências (gRPC, Protobuf)"
    echo "2. Gerar stubs Python e C++"
    echo "3. Compilar servidor C++"
    echo "4. Executar servidor C++"
    echo "5. Executar cliente Python"
    echo "0. Sair"
    echo "===================="
}

# Gerar stubs Python e C++
generate_stubs() {
    # Verifica versão do protoc
    PROTOC_VERSION=$(protoc --version | awk '{print $2}')
    echo "Versão do protoc: $PROTOC_VERSION"
    # Requer protoc >= 3
    MAJOR=${PROTOC_VERSION%%.*}
    if [[ "$MAJOR" -lt 3 ]]; then
        echo "Erro: protoc >= 3 é necessário."
        exit 1
    fi
    echo "Gerando stubs Python..."
    python -m grpc_tools.protoc -I. --python_out=. --grpc_python_out=. mock_client/proto/extractor.proto
    
    echo "Gerando stubs C++..."
    protoc -I./mock_client/proto --cpp_out=mock_client/proto --grpc_out=mock_client/proto --plugin=protoc-gen-grpc="$(which grpc_cpp_plugin)" ./mock_client/proto/extractor.proto
    
    echo "Stubs gerados com sucesso!"
}

# Compilar servidor C++
compile_cpp_server() {
    echo "Compilando servidor C++..."
    mkdir -p build
    cd build
    cmake ..
    make -j4
    cd ..
    echo "Servidor C++ compilado com sucesso!"
}

# Executar servidor C++
run_cpp_server() {
    echo "Executando servidor C++..."
    cd build
    ./grpc_server
    cd ..
}

# Executar cliente Python
run_python_client() {
    echo "Executando cliente Python..."
    PYTHONPATH=. python mock_client/client/grpc_client.py
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
