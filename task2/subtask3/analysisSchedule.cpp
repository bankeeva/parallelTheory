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

std::string schedule_to_string(omp_sched_t kind)
{
    switch (kind)
    {
    case omp_sched_static:
        return "static";
    case omp_sched_dynamic:
        return "dynamic";
    case omp_sched_guided:
        return "guided";
    default:
        return "unknown";
    }
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

void slau_v1(std::vector<double> &A, std::vector<double> &b, std::vector<double> &x, int n)
{
    const double t = 0.01 / n;
    const double eps = 1e-5;
    const int max_iter = 10000;
    double norm_b = 0.0;

    std::vector<double> r(n);

#pragma omp parallel for reduction(+ : norm_b) schedule(runtime)
    for (int i = 0; i < n; i++)
        norm_b += b[i] * b[i];
    norm_b = std::sqrt(norm_b);

    for (int iter = 0; iter < max_iter; iter++)
    {
#pragma omp parallel for schedule(runtime)
        for (int i = 0; i < n; i++)
        {
            double sum = 0.0;
            for (int j = 0; j < n; j++)
                sum += A[i * n + j] * x[j];

            r[i] = sum - b[i];
        }

        double norm_r = 0.0;
#pragma omp parallel for reduction(+ : norm_r) schedule(runtime)
        for (int i = 0; i < n; i++)
            norm_r += r[i] * r[i];
        norm_r = std::sqrt(norm_r);

        if (norm_b != 0 && norm_r / norm_b < eps)
            break;

#pragma omp parallel for schedule(runtime)
        for (int i = 0; i < n; i++)
            x[i] = x[i] - t * r[i];
    }
}

void slau_v2(std::vector<double> &A, std::vector<double> &b, std::vector<double> &x, int n)
{
    const double t = 0.01 / n;
    const double eps = 1e-5;
    const int max_iter = 10000;

    double norm_b = 0.0;
    std::vector<double> r(n);
    bool convergence = false;
    double glob_norm_r = 0.0;

#pragma omp parallel
    {
#pragma omp for reduction(+ : norm_b) schedule(runtime)
        for (int i = 0; i < n; i++)
            norm_b += b[i] * b[i];

#pragma omp single
        norm_b = std::sqrt(norm_b);

        for (int iter = 0; iter < max_iter; iter++)
        {
#pragma omp for schedule(runtime)
            for (int i = 0; i < n; i++)
            {
                double sum = 0.0;
                for (int j = 0; j < n; j++)
                    sum += A[i * n + j] * x[j];

                r[i] = sum - b[i];
            }

#pragma omp single
            glob_norm_r = 0.0;

#pragma omp for reduction(+ : glob_norm_r) schedule(runtime)
            for (int i = 0; i < n; i++)
                glob_norm_r += r[i] * r[i];

#pragma omp single
            {
                double norm_r = std::sqrt(glob_norm_r);
                convergence = (norm_b != 0.0 && norm_r / norm_b < eps);
            }

            if (convergence)
                break;

#pragma omp for schedule(runtime)
            for (int i = 0; i < n; i++)
                x[i] = x[i] - t * r[i];
        }
    }
}

int main()
{
    int N = 2000;
    std::vector<double> A(N * N), b(N), x(N);
    int nt = 8;
    int repit = 100;
    omp_set_num_threads(nt);

    std::vector<omp_sched_t> schedules = {
        omp_sched_static,
        omp_sched_dynamic,
        omp_sched_guided};

    std::vector<int> chunks = {0, 1, 10, 100, 250};

    std::cout << "Исследование параметров schedule для: N = " << N << ", num_threads = " << nt << std::endl;
    std::cout << std::endl;
    std::cout << "V1" << std::endl;

    init(A, b, N);

    for (omp_sched_t sch : schedules)
    {
        for (int chunk : chunks)
        {
            omp_set_schedule(sch, chunk);

            double whole_time = 0.0;

            for (int i = 0; i < repit; i++)
            {
                std::fill(x.begin(), x.end(), 0.0);

                double serial_time = wtime();
                slau_v1(A, b, x, N);
                serial_time = wtime() - serial_time;

                whole_time += serial_time;
            }

            whole_time /= repit;
            std::cout << "Параметр: (" << schedule_to_string(sch) << ", " << chunk << ") Время(c): " << whole_time << std::endl;
        }
    }

    std::cout << std::endl;
    std::cout << "V2" << std::endl;

    for (omp_sched_t sch : schedules)
    {
        for (int chunk : chunks)
        {
            omp_set_schedule(sch, chunk);

            double whole_time = 0.0;

            for (int i = 0; i < repit; i++)
            {
                std::fill(x.begin(), x.end(), 0.0);

                double serial_time = wtime();
                slau_v2(A, b, x, N);
                serial_time = wtime() - serial_time;

                whole_time += serial_time;
            }

            whole_time /= repit;
            std::cout << "Параметр: (" << schedule_to_string(sch) << ", " << chunk << ") Время(c): " << whole_time << std::endl;
        }
    }

    return 0;
}