// использовать метод простой итерации
// Главная диагональ матрицы А = 2.0, остальные эл. = 1.0
// Все элементы вектора b равны N+1
// В этом случае решением будет вектор эл. которого = 1.0

// Ax=b
// преобразование решения на каждом шаге : x_n+1 = x_n - t(Ax_n - b)
// t - const = 0.01 или -0.01, параметр метода
// критерий завершения счета ||Ax_n - b|| / ||b|| < 10^-5
// где ||u|| = sqrt(sum(u_i^2, i = [0, N-1]))

// для каждого распараллеливаемого цикла создается
// отдельная параллельная секция #pragma omp parallel for

#include <iostream>
#include <vector>
#include <omp.h>
#include <cmath>
#include <chrono>
#include <algorithm>

double wtime()
{
    return std::chrono::duration<double>(
               std::chrono::high_resolution_clock::now().time_since_epoch())
        .count();
}

void init(std::vector<double> &A, std::vector<double> &b, int n)
{
#pragma omp parallel for
    for (int i = 0; i < n; i++)
    {
        for (int j = 0; j < n; j++)
            A[i * n + j] = (i == j) ? 2.0 : 1.0;
        b[i] = n + 1.0;
    }
}

void slau(std::vector<double> &A, std::vector<double> &b, std::vector<double> &x, int n)
{
    const double t = 0.01 / n;
    const double eps = 1e-5;
    const int max_iter = 10000;
    double norm_b = 0.0;

    std::vector<double> r(n);

#pragma omp parallel for reduction(+ : norm_b)
    for (int i = 0; i < n; i++)
        norm_b += b[i] * b[i];
    norm_b = std::sqrt(norm_b);

    for (int iter = 0; iter < max_iter; iter++)
    {
#pragma omp parallel for
        for (int i = 0; i < n; i++)
        {
            double sum = 0.0;
            for (int j = 0; j < n; j++)
                sum += A[i * n + j] * x[j];

            r[i] = sum - b[i];
        }

        double norm_r = 0.0;
#pragma omp parallel for reduction(+ : norm_r)
        for (int i = 0; i < n; i++)
            norm_r += r[i] * r[i];
        norm_r = std::sqrt(norm_r);

        if (norm_b != 0 && norm_r / norm_b < eps)
            break;

#pragma omp parallel for
        for (int i = 0; i < n; i++)
            x[i] = x[i] - t * r[i];
    }
}

int main()
{
    int N = 5500;
    std::vector<double> A(N * N), b(N), x(N);
    std::vector<int> num_threads = {1, 2, 4, 8, 16, 20, 26, 32, 40, 57, 64, 73, 80};
    int repit = 10;
    double first_time = 0.0;

    std::cout << "N: " << N << std::endl;

    for (int nt : num_threads)
    {
        double whole_time = 0.0;
        omp_set_num_threads(nt);

        init(A, b, N);

        for (int i = 0; i < repit; i++)
        {
            std::fill(x.begin(), x.end(), 0.0);

            double serial_time = wtime();
            slau(A, b, x, N);
            serial_time = wtime() - serial_time;

            whole_time += serial_time;
        }

        whole_time /= repit;
        if (nt == 1)
        {
            first_time = whole_time;
            std::cout << "Потоков: " << nt << " Время(c): " << whole_time << std::endl;
        }
        else
            std::cout << "Потоков: " << nt << " Время(c): " << whole_time << " Ускорение: " << first_time / whole_time << std::endl;
    }

    return 0;
}