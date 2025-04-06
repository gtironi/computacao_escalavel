# Sistema de Reserva de Viagens

## Visão Geral
Este projeto modela um sistema de reserva de viagens, permitindo que os usuários reservem voos, hotéis ou ambos simultaneamente. O desenvolvimento está sendo realizado como parte da disciplina de Computação Escalável.

## Modelagem do Sistema
O sistema foi projetado para processar grandes volumes de dados de maneira eficiente. Ele utiliza um fluxo ETL otimizado para garantir escalabilidade e balanceamento de carga.

### Estrutura do Sistema
O projeto é composto por:
- `main.cpp`: Arquivo principal que inicia a execução do sistema.
- `framework/`: Diretório contendo todas as classes e implementações necessárias para o funcionamento do sistema.

### Fluxo ETL e Balanceamento de Carga
O sistema utiliza um modelo baseado em filas e threads para maximizar a eficiência:
- Existe **uma fila global de tarefas**, gerenciada automaticamente dentro do próprio framework.
- Todas as classes de **extratores, tratadores e loaders** enviam suas tarefas para essa fila.
- As tarefas são executadas paralelamente por threads disponíveis no hardware.
- Sempre que uma thread fica livre, ela busca o próximo item da fila para execução.

### Gerenciamento de Buffer e Controle de Execução
- A comunicação entre **extratores, tratadores e loaders** é feita por meio de buffers.
- Sempre que uma nova tarefa é criada, o sistema **calcula previamente o espaço que ocupará no buffer**.
- O sistema impede a execução de uma tarefa caso o buffer esteja cheio, garantindo estabilidade e evitando desperdício de recursos.

## Como Executar
1. Clone este repositório:
   ```sh
   git clone https://github.com/gtironi/computacao_escalavel.git
   cd computacao_escalavel
   ```
2. Instale as dependencias:
   ```sh
   ./requirements.sh
   ```
3. Compile os arquivos do framework e a `main.cpp`:
   ```sh
   g++ -o programa main.cpp framework/*.h -std=c++20 -lsqlite3 -pthread
   ```
4. Execute o programa:
   ```sh
   ./programa
   ```

## Autores
Projeto desenvolvido por [Gustavo Tironi](https://github.com/gtironi), [Kauan](), [Matheus](), [Pedro](), [Sillas]()
