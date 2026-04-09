#include <iostream>
#include <vector>
#include <chrono>
#include <thread>

double wtime()
{
    return std::chrono::duration<double>(
               std::chrono::high_resolution_clock::now().time_since_epoch())
        .count();
}

void init(std::vector<double> &mtrx, std::vector<double> &vec, int N, int start, int end)
{
    for (int i = start; i < end; i++)
    {
        for (int j = 0; j < N; j++)
        {
            mtrx[i * N + j] = 1.2 + i + j;
        }
        vec[i] = 2.1 + i;
    }
}

void matMulVec(std::vector<double> &mtrx, std::vector<double> &vec, std::vector<double> &result, int N, int start, int end)
{
    for (int i = start; i < end; i++)
    {
        double sum = 0.0;
        for (int j = 0; j < N; j++)
        {
            sum += mtrx[i * N + j] * vec[j];
        }
        result[i] = sum;
    }
}

int main()
{
    std::vector<int> num_threads = {1, 2, 4, 7, 8, 16, 20, 40};
    std::vector<int> N = {20000, 40000};
    int repit = 500;

    for (int n : N)
    {
        double time_first = 0.0;
        std::cout << "                   N:" << n << std::endl;

        for (int nt : num_threads)
        {
            std::vector<double> mtrx(n * n), vec(n);
            std::vector<double> result(n);
            {
                std::vector<std::jthread> threads;
                for (int tr = 0; tr < nt; tr++)
                {
                    int chunk = n / nt;
                    int start = tr * chunk;
                    int end = (tr == nt - 1) ? n : start + chunk;

                    threads.emplace_back(init, std::ref(mtrx), std::ref(vec),
                                         n, start, end);
                }
                for (auto &t : threads)
                    t.join();
            }

            double whole_time = 0.0;

            for (int i = 0; i < repit; i++)
            {
                double serial_time = wtime();
                std::vector<std::jthread> threads;

                for (int tr = 0; tr < nt; tr++)
                {
                    int chunk = n / nt;
                    int start = tr * chunk;
                    int end = (tr == nt - 1) ? n : start + chunk;

                    threads.emplace_back(matMulVec, std::ref(mtrx), std::ref(vec), std::ref(result),
                                         n, start, end);
                }
                for (auto &t : threads)
                    t.join();

                serial_time = wtime() - serial_time;
                whole_time += serial_time;
            }

            whole_time /= repit;
            if (nt == 1)
            {
                time_first = whole_time;
                std::cout << "Потоков: " << nt << " Время(c): " << whole_time << std::endl;
            }
            else
                std::cout << "Потоков: " << nt << " Время(c): " << whole_time << " Ускорение: " << time_first / whole_time << std::endl;
        }
    }

    return 0;
}
