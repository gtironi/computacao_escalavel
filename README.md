# Sistema de Reserva de Viagens

## Visão Geral
Este projeto apresenta um **framework modular e extensível** para construção de pipelines de dados com alto grau de paralelismo. O objetivo é abstrair a complexidade do controle de concorrência, permitindo que o usuário foque na lógica de transformação dos dados. O desenvolvimento está sendo realizado como parte da disciplina de Computação Escalável, com foco em desempenho, paralelismo e escalabilidade.

Como exemplo de uso, implementamos um **sistema de reservas de viagens**, simulando um fluxo realista de análise de dados em larga escala. O pipeline recebe dados simulados da empresa, processa essas informações em múltiplas etapas paralelas e gera análises relevantes para o negócio.

## Modelagem do Sistema
O sistema adota uma arquitetura baseada em **fila de tarefas** e **thread pool**, garantindo paralelismo seguro e eficiente. Cada componente do pipeline possui uma **thread auxiliar dedicada** à criação de tarefas, enquanto a execução dessas tarefas é realizada por uma **pool de threads separada**.

Resumidamente, o funcionamento do sistema segue os seguintes passos:

1. Cada componente (extrator, tratador, loader) possui uma **thread auxiliar** que cria uma tarefa com a função a ser executada, os dados de entrada e o destino dos resultados.
2. A tarefa é colocada em uma **fila global de tarefas**, compartilhada por todo o sistema.
3. A **thread pool de execução** consome as tarefas da fila e realiza o processamento, interagindo com os buffers de entrada e saída de forma segura.

Esse modelo permite escalar o sistema conforme o número de componentes e threads disponíveis, mantendo **alto grau de paralelismo e uso eficiente dos recursos**.

---

### Estrutura do Repositório
O projeto é composto por:

- `framework/`: Diretório contendo todas as **classes base, abstrações e implementações** necessárias para o funcionamento do sistema (como extratores, tratadores, loaders, fila de tarefas, buffers, etc).
- `mock/`: Scripts Python responsáveis por **simular e gerar dados de entrada** para o sistema.
- `main.cpp`: Arquivo principal que **executa o pipeline completo**, integrando reservas de voos e hotéis simultaneamente.
- `pipe_voos.cpp`: Executa **apenas o pipeline de voos**, útil para testes e análise isolada desse fluxo.
- `pipe_hoteis.cpp`: Executa **apenas o pipeline de hotéis**, útil para testes e análise isolada desse fluxo.

##  Como Executar

###  Pré-requisitos
- C++20 compatível com `g++`
- Python 3.8+
- SQLite3

---
###  Comandos
#### 1. Clone este repositório:
```sh
git clone https://github.com/gtironi/computacao_escalavel.git
cd computacao_escalavel
```

<details><summary>Rodar com o GRPC</summary>
#### 2. Instale o grpc:
```sh
./run_grpc.sh
```

Durante a execução, selecione as opções **1**, **2** e **3**, nessa ordem.

#### 3. Executando o servidor e o cliente

Após a instalação, execute novamente o script `./run_grpc.sh` e:

* Pressione **4** para iniciar o **servidor**.
* Pressione **5** para iniciar o **cliente**.
</details>

<details><summary>Rodar o pipeline sem o GRPC</summary>
#### 2. Execute o simulador (opcional)
<details>

1. Crie e ative uma venv:
```sh
python -m venv venv
source venv/bin/activate  # No Windows: venv\Scripts\activate
```

2. Instale as dependências:
```sh
pip install -r requirements.txt
```

3. Execute todos os scripts Python dentro da pasta `mock/`:
```sh
python mock/*.py
```
</details>


####  3. Instale as dependencias:
```sh
./requirements.sh
```
#### 4. Compile os arquivos do framework e a `main.cpp`:
```sh
g++ -o programa main.cpp -std=c++20 -lsqlite3
```

Caso deseje que os DataFrames sejam exibidos pelo Loader, ative a flag global `PRINT_OUTPUT_DFS`:
```sh
bool PRINT_OUTPUT_DFS = true;
```
Essa flag está definida como false por padrão, mas pode ser facilmente alterada para visualizar os resultados durante a execução.

Caso deseje que o TRIGGER seja ativado, ative a flag global `TRIGGER`:
```sh
bool TRIGGER = true;
```
Essa flag está definida como false por padrão, mas pode ser facilmente alterada para ativar o funcionamento dos triggers na pipeline.

Caso deseje alterar o número de threads utilizadas, altere o valor global `N_THREADS`:
```sh
int N_THREADS = 7;
```

#### 5. Execute o programa:
```sh
./programa
```
<details><summary>Executar as pipelines isoladas</summary>
Pipeline apenas de voos:

```sh
g++ -o voos pipe_voos.cpp -std=c++20 -lsqlite3
./voos
```

Pipeline apenas de hotéis:

```sh
g++ -o hoteis pipe_hoteis.cpp -std=c++20 -lsqlite3
./hoteis
```
</details>
</details>


## 👥 Autores
Projeto desenvolvido por:

- [Gustavo Tironi](https://github.com/gtironi)
- [Kauan Mariani](https://github.com/kauanmaf)
- [Matheus Carvalho](https://github.com/MatCarvalho21)
- [Pedro Henrique Coterli](https://github.com/PedroPHC25)
- [Sillas Rocha](https://github.com/scrocha)
