#include <iostream>
#include <vector>
#include <omp.h>
#include <cmath>
#include <chrono>

double wtime()
{
    return std::chrono::duration<double>(
               std::chrono::high_resolution_clock::now().time_since_epoch())
        .count();
}

double func(double x)
{
    return exp(-x * x);
}

double integrate_omp(double a, double b, int nsteps, int num_threads)
{
    double sum = 0.0;
    double len_sigm = b - a;
    double h = len_sigm / nsteps;

#pragma omp parallel
    {
        double local_sum = 0.0;
        int curr = omp_get_thread_num();
        int chunk = nsteps / num_threads;
        int start = chunk * curr;
        int end = (curr == num_threads - 1) ? nsteps : start + chunk;

        for (int i = start; i < end; i++)
        {
            local_sum += func(a + h * (i + 0.5));
        }
#pragma omp atomic
        sum += local_sum;
    }
    return sum * h;
}

int main()
{
    std::vector<int> num_threads = {1, 2, 4, 7, 8, 16, 20, 40};
    std::vector<int> nsteps = {40000000};
    int repit = 1000;
    double beginning = -4.0, end = 4.0;
    double res = 0.0;

    for (int ns : nsteps)
    {
        std::cout << "              nsteps: " << ns << std::endl;
        for (int nt : num_threads)
        {
            double whole_time = 0.0;
            omp_set_num_threads(nt);

            for (int i = 0; i < repit; i++)
            {
                double serial_time = wtime();

                res = integrate_omp(beginning, end, ns, nt);
                serial_time = wtime() - serial_time;

                whole_time += serial_time;
            }
            whole_time /= repit;

            std::cout << "Потоков: " << nt << " Время(c): " << whole_time << std::endl;
        }
    }
    std::cout << std::endl
              << "Результат интегрирования: " << res << std::endl;

    return 0;
}
// Отрезок [a, b] разбивается на n равных частей шириной h = (b-a)/n
// На каждом подотрезке берётся значение функции в середине: f(a + h*(i + 0.5))
// Интеграл ≈ сумма площадей прямоугольников: sum *= h
// #pragma omp critical - (мьютекс семафор) код последовательно всеми потоками, становятся в очередь
// #pragma omp atomic - (машинная команда) только для ++ += -=
// (При увеличении nsteps ускорение растёт (больше работы на поток → меньше относительных накладных расходов)
// При малых задачах накладные расходы на потоки могут превышать выгоду)
// локальные переменные + atomic