#include "ThreadPool.hpp"
#include <stdexcept>

ThreadPool::ThreadPool(size_t numThreads) {
    if (numThreads == 0) {
        numThreads = 1;
    }

    workers.reserve(numThreads);

    for (size_t i = 0; i < numThreads; ++i) {
        workers.emplace_back([this]() {
            this->workerFunction();
        });
    }
}

ThreadPool::~ThreadPool() {
    stop = true;
    condition.notify_all();

    for (auto& worker : workers) {
        if (worker.joinable()) {
            worker.join();
        }
    }
}

void ThreadPool::workerFunction() {
    while (true) {
        std::function<void()> task;

        {
            std::unique_lock<std::mutex> lock(queueMutex);

            condition.wait(lock, [this]() {
                return stop || !tasks.empty();
            });

            if (stop && tasks.empty()) {
                return;
            }

            task = std::move(tasks.front());
            tasks.pop();
            ++activeTasks;
        }

        // Executa a tarefa fora do lock
        try {
            task();
        } catch (...) {
            // Engole exceções para evitar thread morrer silenciosamente
        }

        {
            std::unique_lock<std::mutex> lock(queueMutex);
            --activeTasks;

            if (tasks.empty() && activeTasks == 0) {
                completionCondition.notify_all();
            }
        }
    }
}

void ThreadPool::waitAll() {
    std::unique_lock<std::mutex> lock(queueMutex);

    completionCondition.wait(lock, [this]() {
        return tasks.empty() && activeTasks == 0;
    });
}