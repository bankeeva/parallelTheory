#include <iostream>
#include <vector>
#include <mutex>
#include <fstream>
#include <string>
#include <sstream>
#include <random>
#include <iomanip>
#include <functional>
#include <cmath>

#include "server.h"
#include "client.h"

template <typename T>
T fun_sqrt(T arg)
{
    return std::sqrt(arg);
}

template <typename T>
T fun_sin(T arg)
{
    return std::sin(arg);
}

template <typename T>
T fun_pow(T x, T y)
{
    return std::pow(x, y);
}

std::vector<double> random_data(int N, double minn = 0.0, double maxx = 1000.0)
{
    std::mt19937 gen(std::random_device{}());
    std::uniform_real_distribution<double> dist(minn, maxx);

    std::vector<double> vec(N);
    for (int i = 0; i < N; ++i)
        vec[i] = dist(gen);

    return vec;
}

void test(int N, std::string filename, std::ofstream &test_file)
{
    std::ifstream file(filename);
    if (!file.is_open())
    {
        std::cerr << "Не удалось открыть файл\n";
        return;
    }

    std::string line;
    int tests_correct = 0;
    std::getline(file, line);

    while (std::getline(file, line))
    {
        if (line.empty())
            continue;

        std::istringstream iss(line);
        std::string name, id, res, arg1, arg2;
        iss >> name >> id >> res >> arg1 >> arg2;

        double val = std::stod(res);

        if (name == "fun_sqrt")
        {
            double chek = fun_sqrt(std::stod(arg1));
            if (std::abs(val - chek) <= 1e-9)
                tests_correct++;
            // else
            //     std::cout << "fun_sqrt " << val << " " << chek << " " << std::abs(val - chek) << std::endl;
        }
        else if (name == "fun_sin")
        {
            double chek = fun_sin(std::stod(arg1));
            if (std::abs(val - chek) <= 1e-9)
                tests_correct++;
            // else
            //     std::cout << "fun_sin " << val << " " << chek << " " << std::abs(val - chek) << std::endl;
        }
        else if (name == "fun_pow")
        {
            double chek = fun_pow(std::stod(arg1), std::stod(arg2));
            if (std::abs(val - chek) <= 1e-9)
                tests_correct++;
            // else
            //     std::cout << "fun_pow " << val << " " << chek << " " << std::abs(val - chek) << std::endl;
        }
    }

    if (tests_correct == N)
        test_file << "все тесты пройдены успешно" << std::endl;
    else
        test_file << "тестов пройдено: " << tests_correct << "/" << N << std::endl;

    file.close();
}

int main()
{
    int N1 = 9000, N2 = 9000, N3 = 9000;
    int num_threads = 1;

    std::mutex mut;

    std::ofstream test_file("test_results.txt");
    if (!test_file.is_open())
    {
        std::cerr << "Не удалось открыть файл\n";
        return 0;
    }

    std::vector<double> data1 = random_data(N1);
    std::vector<double> data2 = random_data(N2);
    std::vector<double> data31 = random_data(N3, 0.0, 10.0);
    std::vector<double> data32 = random_data(N3, 0.0, 4.0);

    std::vector<std::function<double()>> tasks1(N1), tasks2(N2), tasks3(N3);
    std::vector<std::string> args1(N1), args2(N2), args3(N3);

    for (int i = 0; i < N1; i++)
    {
        tasks1[i] = [val = data1[i]]()
        { return fun_sqrt<double>(val); };

        std::ostringstream oss;
        oss << std::setprecision(15) << data1[i];
        args1[i] = oss.str();
    }

    for (int i = 0; i < N2; i++)
    {
        tasks2[i] = [val = data2[i]]()
        { return fun_sin<double>(val); };

        std::ostringstream oss;
        oss << std::setprecision(15) << data2[i];
        args2[i] = oss.str();
    }

    for (int i = 0; i < N3; i++)
    {
        tasks3[i] = [x = data31[i], y = data32[i]]()
        { return fun_pow<double>(x, y); };

        std::ostringstream oss;
        oss << std::setprecision(15) << data31[i] << " " << data32[i];
        args3[i] = oss.str();
    }

    std::ofstream file("results.txt");
    if (!file.is_open())
    {
        std::cerr << "Не удалось открыть файл\n";
        return 1;
    }
    file << "Function name | ID | Result | Arguments" << std::endl;

    Server<> server(num_threads);
    server.start();

    Client<> client1(N1, tasks1, server, mut, file, "fun_sqrt", args1);
    Client<> client2(N2, tasks2, server, mut, file, "fun_sin", args2);
    Client<> client3(N3, tasks3, server, mut, file, "fun_pow", args3);

    client1.start();
    client2.start();
    client3.start();

    client1.wait();
    client2.wait();
    client3.wait();

    server.stop();
    file.close();

    test(N1 + N2 + N3, "results.txt", test_file);

    test_file.close();

    return 0;
}