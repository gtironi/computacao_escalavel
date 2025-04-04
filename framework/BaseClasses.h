#include "Buffer.h"
#include "Dataframe.h"
#include "Manager.h"
#include <utility>  // Para std::forward
#include <tuple>
// using namespace std;

// // Classe base para todos. Todos os stages precisam de uma task queue que será gerenciada pelo manager global
// class Stage {
//     protected:
//         TaskQueue* taskqueue = nullptr;
//         // std::atomic<bool> running{true};
//     public:

//         // Função que será escrita pelo usuário ao criar uma classe que herda de algum stage
//         virtual void run() = 0;

//         // Função que é um wrapper de run que será mandado para ser executada
//         virtual void create_task() = 0;

//         void set_taskqueue(TaskQueue* tq) { taskqueue = tq; }
//         TaskQueue* get_taskqueue() const { return taskqueue; }
        
//         virtual ~Stage() = default;
       
// };

/**
 * @brief Classe base para extratores de dados.
 */


// Classe base template usando CRTP
template <typename Derived, typename T>
class Extractor {
protected:
    Buffer<T> output_buffer;
    TaskQueue* taskqueue = nullptr;

public:
    virtual ~Extractor() = default;
    void set_taskqueue(TaskQueue* tq) { taskqueue = tq; }
    TaskQueue* get_taskqueue() const { return taskqueue; }

    Buffer<T>& get_output_buffer() { return output_buffer; }

    // Delega a implementação para a classe derivada
    template <typename... Args>
    T run(Args&&... args) {
        return static_cast<Derived*>(this)->run(std::forward<Args>(args)...);
    }

    // create_task repassa os argumentos para run
    template <typename... Args>
    void create_task(Args&&... args) {
        T data = run(std::forward<Args>(args)...);
        output_buffer.push(data);
    }

    // add_task_thread cria tarefas assíncronas repassando os argumentos
    template <typename... Args>
    void add_task_thread(Args&&... args) {
        while (output_buffer.get_semaphore().get_count() > 0) {
            // Captura os argumentos e os repassa para create_task
            taskqueue->push_task([this, ...args = std::forward<Args>(args)]() mutable {
                this->create_task(std::forward<Args>(args)...);
            });
            output_buffer.get_semaphore().wait();
        }
    }
    
};

template <typename T>
class Transformer {
    protected:
        Buffer<T>& input_buffer;
        Buffer<T> output_buffer;
        TaskQueue* taskqueue = nullptr;
        
    
    public:
        explicit Transformer(Buffer<T>& in) : input_buffer(in) {}

        void add_task_thread(){
            // Queremos apenas adicionar a task, caso o semáforo do output seja diferente de 0, e o do anterior não estiver full
            while (!output_buffer.get_semaphore().get_count() && input_buffer.get_semaphore().get_count() != input_buffer.get_max_size())
            {
                T value = input_buffer.pop();
                taskqueue->push_task([this, val = std::move(value)]() mutable {
                    this->create_task(std::move(val));
                });
                output_buffer.get_semaphore().wait();
            } 
        }        

        Buffer<T>& get_output_buffer() { return output_buffer; }
        T run(T dataframe) override = 0;
        
        
        void create_task(T value) {
            T data = run(value);
            output_buffer.push(data);
        }

        virtual ~Transformer() = default;
        void set_taskqueue(TaskQueue* tq) { taskqueue = tq; }
        TaskQueue* get_taskqueue() const { return taskqueue; }
};


template <typename T>
class Loader {
    protected:
        Buffer<T>& input_buffer;
        TaskQueue* taskqueue = nullptr;

    public:
    explicit Loader(Buffer<T>& buffer) : input_buffer(buffer) {}
    virtual ~Loader() = default;
    void set_taskqueue(TaskQueue* tq) { taskqueue = tq; }
    TaskQueue* get_taskqueue() const { return taskqueue; }

    void add_task_thread(){
        // Queremos apenas adicionar a task, caso o semáforo do output seja diferente de 0, e o do anterior não estiver full
        while (input_buffer.get_semaphore().get_count() != input_buffer.max_size)
        {
            T value = input_buffer.pop();
            taskqueue->push_task([this, val = std::move(value)]() mutable {
                this->create_task(std::move(val));
            });
       } 
    }  
       
    virtual void run() override = 0;

    // Redundante, mas para manter a consistência
    void create_task(T value) {
        run(value);
    }

};
    