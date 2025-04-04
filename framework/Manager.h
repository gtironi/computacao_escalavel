#ifndef MANAGER_H
#define MANAGER_H

#include <iostream>
#include <queue>
#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <atomic>

// Classe da fila de tarefas
class TaskQueue
{
    private:
        // Fila de tarefas como uma fila de funções
        std::queue<std::function<void()>> tasks;
        std::mutex mtx;
        std::condition_variable cond;
        // Booleano para controlar quando o processo deve terminar
        bool finishedWork = false;

    public:
        // Método para adicionar uma nova tarefa à fila
        void push_task(std::function<void()> task)
        {
            {
                // Trava o mutex
                std::lock_guard<std::mutex> lock(mtx);
                // Adiciona a tarefa
                tasks.push(task);
            }
            // Acorda as threads esperando por algo aparecer na fila
            cond.notify_one();
        }

        // Método para pegar uma tarefa da fila
        std::function<void()> pop_task()
        {
            // Trava o mutex
            std::unique_lock<std::mutex> lock(mtx);
            // Se a fila estiver vazia, espera até ser acordado e
            // ter aparecido algo ou o processo ter finalizado
            cond.wait(lock, [this] 
                {
                    return !tasks.empty() || finishedWork;
                });
            // Se o processo for parar e a fila tiver acabado, não retorna nada
            if (finishedWork && tasks.empty())
            {
                return nullptr;
            }
            // Pega uma tarefa e a remove da fila
            std::function<void()> task = tasks.front();
            tasks.pop();
            
            return task;
        }

        // Método para encerrar todo o processo
        void shutdown()
        {
            {
                // Trava o mutex
                std::lock_guard<std::mutex> lock(mtx);
                // Indica que o trabalho foi encerrado
                finishedWork = true;
            }
            // Notifica todas as threads adormecidas
            cond.notify_all();
        }

        // Método para checar se a fila está vazia
        bool is_empty()
        {
            // Trava o mutex
            std::lock_guard<std::mutex> lock(mtx);
            // Retorna o resultado
            return tasks.empty();
        }
};

// Classe do gerenciador das threads
class Manager
{
    private:
        // Vetor das threads
        std::vector<std::thread> threads;
        // Fila de tarefas
        TaskQueue task_queue;
        std::mutex mtx;
        std::condition_variable cond;
        // Indicador se o trabalho foi interrompido
        bool finishedWork = false;

    public:
        // Método construtor
        Manager(int num_threads)
        {
            // Para cada thread...
            for (int i = 0; i < num_threads; i++)
            {
                // Cria ela
                threads.emplace_back([this]
                {
                    // Enquanto o trabalho não for finalizado...
                    while (!finishedWork)
                    {
                        // Tenta pegar a próxima tarefa da fila e a executa
                        std::function<void()> task = task_queue.pop_task();
                        if (task)
                        {
                            task();
                        }
                    }
                });
            }
        }

        

        // Método para encerrar o processo
        void stop()
        {
            // Seta a variável de trabalho finalizado como true
            {
                std::lock_guard<std::mutex> lock(mtx);
                finishedWork = true;
            }
            // Notifica todas as threads adormecidas
            cond.notify_all();
            // Finaliza a fila de tarefas
            task_queue.shutdown();
            // Espera todas as threads terminarem
            for (auto& thread : threads)
            {
                if (thread.joinable())
                {
                    thread.join();
                }
            }
        }

        // Destrutor
        ~Manager()
        {
            stop();
        }
};

#endif // MANAGER_H