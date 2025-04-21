// Classe base genérica para transformação de dados em um pipeline paralelo
// T: Tipo dos dados processados (ex: Dataframe, estrutura customizada, etc.)
template <typename T>
class Transformer {
protected:
    // Buffers de entrada (ponteiros, pois podem ser compartilhados entre componentes)
    std::vector<Buffer<T>*> input_buffers;

    // Buffers de saída (criados pela própria classe)
    std::vector<Buffer<T>> output_buffers;

    // Fila de tarefas onde serão enfileiradas as transformações
    TaskQueue* taskqueue = nullptr;

    // Número de buffers de saída e índice do próximo a ser usado
    int numOutputBuffers;
    int nextOutputBuffer = 0;

    // Número de buffers de entrada
    int numInputBuffers;

    // Histórico de dataframes usados nas transformações, útil quando há múltiplas entradas
    std::vector<T> historyDataframes;
    // Vetor das estatísticas internas do transformer
    std::vector<float> stats;
    // Mutexes para a atualização das estatísticas e dos dataframes de histórico
    std::mutex statsMtx;
    std::mutex dfsMtx;

private:
    // Método para fazer a atualização das estatísticas
    void aggStats(std::vector<float> newStats)
    {
        std::lock_guard<std::mutex> lock(statsMtx);
        // Se o vetor não estiver inicializado, inicializa-o
        if (stats.size() == 0)
        {
            stats.resize(newStats.size());
        }
        // Incrementa os valores
        for (int i = 0; i < stats.size(); i++)
        {
            stats[i] += newStats[i];
        }
    }

    // void group_by()

public:
    /**
     * Construtor do Transformer
     * @param in - Buffers de entrada
     * @param num_outputs - Número de buffers de saída a serem criados
     */
    Transformer(std::vector<Buffer<T>*> in, int num_outputs = 1)
        : input_buffers(in), output_buffers(num_outputs), 
          numOutputBuffers(num_outputs), numInputBuffers(input_buffers.size()) {
        
        // Inicializa o histórico de entradas se houver mais de um buffer de entrada
        if (numInputBuffers > 1) {
            historyDataframes.resize(numInputBuffers);
        }
    }

    /**
     * @brief Retorna o próximo buffer de saída disponível (avança o índice)
     * @throws std::out_of_range se exceder o número de buffers disponíveis
     * @return Buffer de output
     */
    Buffer<T>& get_output_buffer() {
        if (nextOutputBuffer >= numOutputBuffers) {
            std::cout << "ERROR: NUMBER OF USED BUFFERS EXCEEDED NUMBER OF CREATED BUFFERS!" << std::endl;
            throw std::out_of_range("Número de buffers excedido!");
        }
        return output_buffers[nextOutputBuffer++];
    }

    /**
     * Retorna o buffer de saída por índice (sem alterar o estado do Transformer)
     * @param index - índice do buffer de saída desejado
     */
    Buffer<T>& get_output_buffer_by_index(int index) { 
        return output_buffers[index]; 
    }

    /**
     * Método principal que enfileira as tarefas na fila de execução.
     * Realiza o controle de semáforos, histórico e empacotamento das entradas para processar.
     */
    void enqueue_tasks() {
        while (true) {
            // Verifica se ainda existem dados nos buffers de entrada
            bool canContinue = false;
            for (int i = 0; i < input_buffers.size(); i++) {
                if (!input_buffers[i]->atomicGetInputDataFinished()) {
                    canContinue = true;
                }
            }

            if (canContinue) {
                // Verifica se todos os buffers de saída possuem espaço disponível
                bool canSendTask = true;
                for (int i = 0; i < numOutputBuffers; i++) {
                    if (get_output_buffer_by_index(i).get_semaphore().get_count() <= 0) {
                        canSendTask = false;
                        break;
                    }
                }

                if (canSendTask) {
                    std::optional<T> maybe_value;
                    int currentInputBuffer = -1;

                    // Tenta extrair dados de algum buffer de entrada
                    for (int i = 0; i < numInputBuffers; i++) {
                        maybe_value = input_buffers[i]->pop(numInputBuffers > 1);
                        if (maybe_value.has_value()) {
                            currentInputBuffer = i;
                            break;
                        }
                    }

                    // Caso não tenha encontrado, tenta novamente verificando se ainda há dados não finalizados
                    if (!maybe_value.has_value()) {
                        for (int i = 0; i < numInputBuffers; i++) {
                            if (!input_buffers[i]->atomicGetInputDataFinished()) {
                                maybe_value = input_buffers[i]->pop();
                                currentInputBuffer = i;
                                break;
                            }
                        }
                    }

                    // Se conseguiu extrair algo
                    if (maybe_value.has_value()) {
                        T value = std::move(*maybe_value);

                        // Armazena o histórico se houver múltiplas entradas
                        if (numInputBuffers > 1) {
                            {
                                std::lock_guard<std::mutex> lock(dfsMtx);
                                historyDataframes[currentInputBuffer].hStack(value);
                            }
                        }

                        // Prepara argumentos para a transformação
                        std::vector<std::shared_ptr<T>> args(numInputBuffers);
                        for (int i = 0; i < numInputBuffers; i++) {
                            if (i == currentInputBuffer) {
                                args[i] = std::make_shared<T>(std::move(value));
                            } else {
                                args[i] = std::make_shared<T>(historyDataframes[i]);
                            }
                        }

                        // Enfileira tarefa na fila de execução
                        taskqueue->push_task([this, args = std::move(args)]() mutable {
                            std::vector<T*> raw_args;
                            for (auto& ptr : args) {
                                raw_args.push_back(ptr.get());
                            }
                            this->create_task(std::move(raw_args));
                        });
                    }
                }
            } else {
                // Nenhum dado restante nos buffers de entrada
                break;
            }
        }

        // Finaliza os buffers de saída após o fim do processamento
        finishBuffer();
    }

    /**
     * @brief Método abstrato que será implementado por subclasses para aplicar a lógica de transformação.
     * @param dataframe - vetor de ponteiros para os dados de entrada
     * @return T - resultado da transformação
     */
    virtual T run(std::vector<T*> dataframe) = 0;

    // Método "abstrato" do cálculo das estatísticas
    virtual std::vector<float> calculateStats(std::vector<T*> dataframe)
    {
        return std::vector<float>{};
    };

    // Getter das estatísticas
    std::vector<float> getStats()
    {
        std::lock_guard<std::mutex> lock(statsMtx);
        return stats;
    }

    /**
     * @brief Envolve a execução de `run` e o envio dos dados para os buffers de saída.
     * @param value - vetor de ponteiros para os dados de entrada
     */
    void create_task(std::vector<T*> value) {
        // Calcula e agrega as estatísticas
        std::vector<float> currentStats = calculateStats(value);
        aggStats(currentStats);
        
        T data = run(value);
        for (int i = 0; i < numOutputBuffers; i++) {
            // Incrementa o semáforo e espera até que tenha vaga
            get_output_buffer_by_index(i).get_semaphore().wait();
            // Coloca no buffer
            get_output_buffer_by_index(i).push(data);
        }
    }

    virtual ~Transformer() = default;

    // Setter da fila de tarefas
    void set_taskqueue(TaskQueue* tq) { taskqueue = tq; }

    // Getter da fila de tarefas
    TaskQueue* get_taskqueue() const { return taskqueue; }

    /**
     * @brief Finaliza todos os buffers de saída, indicando que não haverá mais dados.
     */
    void finishBuffer() {
        for (int i = 0; i < numOutputBuffers; i++) {
            get_output_buffer_by_index(i).finalizeInput();
        }
    }
};


// class GroupbyTransformer : public Transformer
// {

// }