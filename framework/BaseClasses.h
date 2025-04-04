#include "Buffer.h"
#include "Dataframe.h"
#include "Manager.h"
// using namespace std;

// Classe base para todos. Todos os stages precisam de uma task queue que será gerenciada pelo manager global
class Stage {
    protected:
        TaskQueue* taskqueue = nullptr;
        std::atomic<bool> running{true};
    public:
        // Função que serve para enviar tasks para a queue
        virtual void create_tasks() {
            while (running) {
                if (taskqueue) {
                    taskqueue->push_task([this]() { this->run(); });
                }
                // std::this_thread::sleep_for(std::chrono::milliseconds(10)); // evitar busy wait
            }
        }

        // Função que será escrita pelo usuário ao criar uma classe que herda de algum stage
        virtual void run() = 0;

        void set_taskqueue(TaskQueue* tq) { taskqueue = tq; }
        TaskQueue* get_taskqueue() const { return taskqueue; }

        virtual ~Stage() = default;
    
        
};

/**
 * @brief Classe base para extratores de dados.
 */
template <typename T>
class Extractor : public Stage {
    protected:
        Buffer<T> output_buffer;
        
    public:
        Buffer<T>& get_output_buffer() { return output_buffer; }
        virtual void run() override = 0;
};

template <typename T>
class Transformer : public Stage {
    protected:
        Buffer<T>& input_buffer;
        Buffer<T> output_buffer;
    
    public:
        explicit Transformer(T& in) : input_buffer(in) {}
        Buffer<T>& get_output_buffer() { return output_buffer; }
        virtual void run() override = 0;
};

template <typename T>
class Loader : public Stage {
    protected:
        Buffer<T>& input_buffer;

    public:
        explicit Loader(Buffer<T>& buffer) : input_buffer(buffer) {}
        virtual void run() override = 0;
};
    