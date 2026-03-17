#include <iostream>
#include <vector>
#include <omp.h>
#include <chrono>

double wtime()
{
    return std::chrono::duration<double>(
               std::chrono::high_resolution_clock::now().time_since_epoch())
        .count();
}

void init(std::vector<double> &mtrx, std::vector<double> &vec, int N)
{
#pragma omp parallel for
    for (int i = 0; i < N; i++)
    {
        for (int j = 0; j < N; j++)
        {
            mtrx[i * N + j] = 1.2;
        }
        vec[i] = 2.1;
    }
}

void matMulVec(std::vector<double> &mtrx, std::vector<double> &vec, std::vector<double> &result, int N, int num_threads)
{
#pragma omp parallel
    {
        int chunk = N / num_threads;
        int curr = omp_get_thread_num();
        int start = curr * chunk;
        int end = (curr == num_threads - 1) ? N : start + chunk;

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
}

int main()
{
    std::vector<int> num_threads = {1, 2, 4, 6, 8, 16, 20, 40};
    std::vector<int> N = {20000, 40000};
    int repit = 500;

    for (int n : N)
    {
        std::cout << "                   N:" << n << std::endl;

        std::vector<double> mtrx(n * n), vec(n), result(n);
        init(mtrx, vec, n);

        for (int nt : num_threads)
        {
            double whole_time = 0.0;
            omp_set_num_threads(nt);

            for (int i = 0; i < repit; i++)
            {
                std::fill(result.begin(), result.end(), 0.0);
                double serial_time = wtime();
                matMulVec(mtrx, vec, result, n, nt);
                serial_time = wtime() - serial_time;

                whole_time += serial_time;
            }
            whole_time /= repit;

            std::cout << "Потоков: " << nt << " Время(c): " << whole_time << std::endl;
        }
    }

    return 0;
}