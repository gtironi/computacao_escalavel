#!/bin/bash
# Script para executar o cliente Python e o servidor C++

# Função para exibir menu
show_menu() {
    echo "==== MENU GRPC ====="
    echo "1. Gerar stubs Python e C++"
    echo "2. Compilar servidor C++"
    echo "3. Executar servidor C++"
    echo "4. Executar cliente Python"
    echo "5. Instalar dependências (gRPC, Protobuf)"
    echo "0. Sair"
    echo "===================="
}

# Gerar stubs Python e C++
generate_stubs() {
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
        1) generate_stubs ;;
        2) compile_cpp_server ;;
        3) run_cpp_server ;;
        4) run_python_client ;;
        5) install_dependencies ;;
        0) echo "Saindo..."; exit 0 ;;
        *) echo "Opção inválida! Tente novamente." ;;
    esac
    
    echo
    read -p "Pressione Enter para continuar..."
    clear
done
