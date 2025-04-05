#ifndef BASE_CLASSES_H
#define BASE_CLASSES_H
#include "Buffer.h"
#include "Dataframe.h"
#include "TaskQueue.h"
#include <utility>  // Para std::forward
#include <tuple>
#include <optional>
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
template <typename T>
class Extractor {
protected:
    Buffer<T> output_buffer;
    TaskQueue* taskqueue = nullptr;

public:
    virtual ~Extractor() = default;
    void set_taskqueue(TaskQueue* tq) { taskqueue = tq; }
    TaskQueue* get_taskqueue() const { return taskqueue; }

    Buffer<T>& get_output_buffer() { return output_buffer; }

    // Pure virtual function to be implemented by derived classes
    virtual T run() = 0;

    // create_task calls the virtual run()
    void create_task() {
        T data = run();
        output_buffer.push(data);
    }

    // Variadic version if needed (though virtual functions can't be templates)
    template <typename... Args>
    void create_task(Args&&... args) {
        T data = run(std::forward<Args>(args)...);
        output_buffer.push(data);
    }

    void enqueue_tasks() {
        while (true) {
            if (output_buffer.get_semaphore().get_count() > 0)
            {
                taskqueue->push_task([this]() {
                    this->create_task();
                });
                output_buffer.get_semaphore().wait();
            }
        }
    }

    // Variadic version if needed
    template <typename... Args>
    void enqueue_tasks(Args&&... args) {
        while (true) {
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

        void enqueue_tasks(){
            // Queremos apenas adicionar a task, caso o semáforo do output seja diferente de 0, e o do anterior não estiver full
            
            while (true)
            {
                // std::cout << "============================" << std::endl;
                // std::cout << output_buffer.get_semaphore().get_count() << std::endl;
                // std::cout << input_buffer.get_semaphore().get_count() << std::endl;
                // std::cout << "============================" << std::endl;
                if (output_buffer.get_semaphore().get_count() > 0)
                {
                    std::optional<T> maybe_value = input_buffer.pop();
                    if (!maybe_value.has_value()) {
                        // Timeout — decidir o que fazer: pular, logar, continuar...
                        break;
                    }
                    T value = std::move(*maybe_value);
                    taskqueue->push_task([this, val = std::move(value)]() mutable {
                        this->create_task(std::move(val));
                    });
                    output_buffer.get_semaphore().wait();
                }
            }
        }        

        Buffer<T>& get_output_buffer() { return output_buffer; }
        virtual T run(T dataframe) = 0;
        
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

    void enqueue_tasks(){
        // Queremos apenas adicionar a task, caso o semáforo do output seja diferente de 0, e o do anterior não estiver full
        while (true)
        {
            std::cout << "============================" << std::endl;
            std::cout << (input_buffer.get_semaphore().get_count()) << std::endl;
            std::cout << "============================" << std::endl;
            std::optional<T> maybe_value = input_buffer.pop();
            if (!maybe_value.has_value()) {
                // Timeout — decidir o que fazer: pular, logar, continuar...
                break;
            }
            T value = std::move(*maybe_value);
            taskqueue->push_task([this, val = std::move(value)]() mutable {
                this->create_task(std::move(val));
            });
       } 
    }  
       
    virtual void run(T value) = 0;

    // Redundante, mas para manter a consistência
    void create_task(T value) {
        run(std::move(value));
    }

};

#endif