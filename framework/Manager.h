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
#include "BaseClasses.h"
#include "TaskQueue.h"

// Classe do gerenciador das threads
template <typename T>
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
        // Indicador se o trabalho já foi iniciado
        bool running = false;
        // Listas de extratores, de transformadores e de carregadores do pipeline
        std::vector<Extractor<T>*> extractors;
        std::vector<Transformer<T>*> transformers;
        std::vector<Loader<T>*> loaders;

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
                    // Espera até o processo ser iniciado ou finalizado
                    std::unique_lock<std::mutex> lock(mtx);

                    cond.wait(lock, [this]
                    {
                        return running || finishedWork;
                    });

                    // Enquanto o trabalho não for finalizado...
                    while (!finishedWork)
                    {
                        // Libera o lock para as múltiplas threads começarem a trabalhar
                        lock.unlock();
                        // Tenta pegar a próxima tarefa da fila e a executa
                        std::function<void()> task = task_queue.pop_task();
                        if (task)
                        {
                            task();
                        }
                        // Bloqueia antes de verificar a condição de parada novamente
                        lock.lock();
                    }
                });
            }
        }

        // Métodos para adicionar extratores, transformadores e carregadores ao pipeline
        // Antes disso, informa pra eles a fila de tarefas na qual eles adicionarão tarefas
        void addExtractor(Extractor<T>* extractor)
        {
            extractor -> set_taskqueue(&task_queue);
            extractors.push_back(extractor);
        }
        void addTransformer(Transformer<T>* transformer)
        {
            transformer -> set_taskqueue(&task_queue);
            transformers.push_back(transformer);
        }
        void addLoader(Loader<T>* loader)
        {
            loader -> set_taskqueue(&task_queue);
            loaders.push_back(loader);
        }

        // Método para começar a executar o processo
        void run()
        {
            // Chama as threads para começarem a pegar coisas da fila de tarefas
            {
                std::lock_guard<std::mutex> lock(mtx);
                running = true;   
            }
            cond.notify_all();

            // Bloqueia o mutex para acessar a variável finishedWork
            std::unique_lock<std::mutex> lock(mtx);

            // Enquanto o trabalho não for finalizado...
            while (!finishedWork)
            {
                // Desbloqueia o mutex
                lock.unlock();
                // Manda todos os blocos de processo mandarem o que puderem pra fila de tarefas
                for (int i = 0; i < extractors.size(); i++)
                {
                    extractors[i] -> add_task_thread();
                    // std::cout << "Extractor" << i << std::endl;
                }
                for (int i = 0; i < transformers.size(); i++)
                {
                    transformers[i] -> add_task_thread();
                    // std::cout << "Transform" << i << std::endl;
                }
                for (int i = 0; i < loaders.size(); i++)
                {
                    loaders[i] -> add_task_thread();
                    // std::cout << "Loader" << i << std::endl;
                }
                // Rebloqueia o mutex pra reacessar 
                lock.lock();
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