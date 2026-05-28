#include <boost/program_options.hpp>
#include <iostream>
#include <vector>
#include <cmath>
#include <chrono>

double wtime()
{
    return std::chrono::duration<double>(
               std::chrono::high_resolution_clock::now().time_since_epoch())
        .count();
}

void init_mtrx(int N, double* A)
{
    for (int i = 0; i < N*N; i++) A[i] = 0.0;

    A[0] = 10.0;
    A[N-1] = 20.0;
    A[(N-1)*N] = 20.0;
    A[(N-1)*N + (N-1)] = 30.0;

    for (int i = 0; i < N; i++){
        A[i] = 10.0 + (20.0 - 10.0) / (N-1) * i; 
        A[(N-1)*N + i] = 20.0 + (30.0 - 20.0) / (N-1) * i; 
        A[i*N] = 10.0 + (20.0 - 10.0) / (N-1) * i; 
        A[i*N + (N-1)] = 20.0 + (30.0 - 20.0) / (N-1) * i; 
    }
}

int main(int argc, char** argv){
    boost::program_options::options_description desc("Allowed options");
    desc.add_options()
        ("help,h", "вывести справку")
        ("size,s", boost::program_options::value<int>()->default_value(128), "размер сетки N x N")
        ("tol,t", boost::program_options::value<double>()->default_value(1e-6), "точность")
        ("maxiter,m", boost::program_options::value<int>()->default_value(1000000), "максимальное число итераций");

    boost::program_options::variables_map vm;
    boost::program_options::store(boost::program_options::parse_command_line(argc, argv, desc), vm);
    boost::program_options::notify(vm);

    if (vm.count("help")) {
        std::cout << desc << std::endl;
        return 0;
    }

    int N = vm["size"].as<int>();
    double tol = vm["tol"].as<double>();
    int max_iter = vm["maxiter"].as<int>();

    std::vector<double> A_(N * N, 0.0);
    std::vector<double> Anew_(N * N, 0.0);

    double* A = A_.data();
    double* Anew = Anew_.data();

    init_mtrx(N, A);
    init_mtrx(N, Anew);

    int iter = 0;
    double er = 1.0;

#pragma acc data copy(A[0:N*N], Anew[0:N*N])
    {
        double start_time = wtime();

        while (iter < max_iter && er > tol){
            er = 0.0;

#pragma acc parallel loop collapse(2) reduction(max:er) present(A[0:N*N], Anew[0:N*N])
            for (int i = 1; i < N - 1; i++) {
                for (int j = 1; j < N - 1; j++) {

                    Anew[i*N + j] = (A[i*N + j-1] + A[i*N + j+1] + 
                                    A[(i-1)*N + j] + A[(i+1)*N + j]) * 0.25;

                    double diff = fabs(Anew[i*N + j] - A[i*N + j]);
                    if (diff > er) er = diff;
                }
            }
            
            std::swap(A, Anew);
            iter++;
        }

        double runtime = wtime() - start_time;
        std::cout << "Iter: " << iter << " | Final error: " << er << "| Runtime: " << runtime << "sec" << std::endl;
    }

    if (N == 13 || N == 10){
        std::cout << "Final grid " << N << "x" << N << std::endl;
        for (int i = 0; i < N; i++){
            for (int j = 0; j < N; j++)
                std::cout << A[i*N + j] << " ";
            std::cout << std::endl;
        }
    }

    return 0;
}