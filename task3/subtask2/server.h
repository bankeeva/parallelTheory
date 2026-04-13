#pragma once

#include <queue>
#include <thread>
#include <mutex>
#include <unordered_map>
#include <atomic>
#include <functional>
#include <limits>
#include <iostream>
#include <utility>

template <typename Task = std::function<double()>, typename Result = double>
class Server
{
    std::queue<std::pair<size_t, Task>> tasks;
    std::unordered_map<size_t, Result> results;

    std::mutex mtx_queue;
    std::mutex mtx_result;

    std::vector<std::jthread> worker_threads;
    size_t num_threads;

    std::atomic<size_t> id_counter{0};
    std::atomic<bool> is_running{false};

    void worker(std::stop_token stoken)
    {
        while (!stoken.stop_requested())
        {
            size_t id_task = std::numeric_limits<size_t>::max();
            Task task;

            {
                std::lock_guard<std::mutex> lock(mtx_queue);

                if (!tasks.empty())
                {
                    id_task = tasks.front().first;
                    task = std::move(tasks.front().second);
                    tasks.pop();
                }
            }

            if (id_task != std::numeric_limits<size_t>::max())
            {
                Result res = task();

                {
                    std::lock_guard<std::mutex> lock(mtx_result);
                    results[id_task] = std::move(res);
                }
            }
        }
    }

public:
    Server(size_t num_threads)
        : num_threads(num_threads)
    {

        if (this->num_threads == 0 || this->num_threads > 76)
        {
            std::cout << "Введено некорректное количество потоков, по умолчанию = 1" << std::endl;
            this->num_threads = 1;
        }
    }

    void start()
    {
        is_running = true;
        for (size_t i = 0; i < num_threads; i++)
            worker_threads.emplace_back([this](std::stop_token stoken)
                                        { this->worker(stoken); });
    }

    size_t add_task(Task task)
    {
        if (!is_running)
            return std::numeric_limits<size_t>::max();

        size_t id_task = id_counter.fetch_add(1);

        {
            std::lock_guard<std::mutex> lock(mtx_queue);
            tasks.push({id_task, std::move(task)});
        }

        return id_task;
    }

    Result request_result(size_t id_task)
    {
        while (true)
        {
            {
                std::lock_guard<std::mutex> lock(mtx_result);

                if (results.find(id_task) != results.end())
                {
                    Result res = std::move(results[id_task]);
                    results.erase(id_task);

                    return res;
                }

                if (!is_running)
                    return Result{};
            }
        }
    }

    void stop()
    {
        is_running = false;
        for (auto &thread : worker_threads)
            thread.request_stop();
    }
};