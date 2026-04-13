#pragma once

#include <vector>
#include <thread>
#include <mutex>
#include <fstream>
#include <string>
#include <atomic>
#include <iostream>
#include <iomanip>
#include <functional>

template <typename Task = std::function<double()>, typename Result = double>
class Client
{
private:
    int num_tasks;
    std::vector<Task> tasks;
    Server<Task, Result> &server;
    std::string func_name;
    std::jthread worker_thread;

    std::mutex &mut;
    std::ofstream &file;

    std::vector<size_t> ids;
    std::atomic<bool> err{false};

    std::vector<std::string> args;

    void worker()
    {
        if (!err)
        {
            for (int i = 0; i < num_tasks; i++)
                ids.emplace_back(server.add_task(std::move(tasks[i])));

            for (int i = 0; i < num_tasks; i++)
            {
                Result res = server.request_result(ids[i]);
                {
                    std::lock_guard<std::mutex> lock(mut);
                    file << func_name << " " << ids[i] << " " << std::setprecision(15) << res << " " << args[i] << std::endl;
                }
            }
        }
    }

public:
    Client(int num_tasks, std::vector<Task> tasks, Server<Task, Result> &server, std::mutex &mut, std::ofstream &file, std::string func_name_, std::vector<std::string> args)
        : num_tasks(num_tasks), tasks(std::move(tasks)), server(server), mut(mut), file(file), func_name(func_name_), args(std::move(args))
    {
        if (this->tasks.empty())
        {
            std::cout << func_name << ": нет задачи" << std::endl;
            err = true;
        }
        if (this->num_tasks <= 5 || this->num_tasks >= 10000)
        {
            std::cout << func_name << ": колличество задач от 5 до 10000" << std::endl;
            err = true;
        }
    }

    void start()
    {
        worker_thread = std::jthread(&Client::worker, this);
    }

    void wait()
    {
        if (worker_thread.joinable())
            worker_thread.join();
    }
};