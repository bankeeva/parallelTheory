#include <iostream>
#include <cmath>
#include <vector>

#if USE_DOUBLE
using choice = double;
#else
using choice = float;
#endif

int main()
{
    int N = 10000000;
    choice pi = std::acos(-1.0), sum = 0;

    std::vector<choice> sinVal(N);
    for (int i = 0; i < N; i++)
    {
        choice angle = 2.0 * pi * i / (N - 1);
        choice v = std::sin(angle);
        sinVal[i] = v;
        sum += sinVal[i];
    }

    std::cout << sum << std::endl;

    return 0;
}
