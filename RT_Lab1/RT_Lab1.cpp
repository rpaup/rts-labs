#include <iostream>
#include <ctime> 

long long factorial(int n) {
    long long result = 1;
    for (int i = 1; i <= n; ++i) {
        result *= i;
    }
    return result;
}

int main() {
    const int N = 10;
    const int ITERATIONS = 10000000; 

    std::cout << "Calculating " << N << "! " << ITERATIONS << " times..." << std::endl;

    clock_t start = clock();

    for (int i = 0; i < ITERATIONS; ++i) {
        factorial(N);
    }

    clock_t end = clock();

    double seconds = (double)(end - start) / CLOCKS_PER_SEC;

    std::cout << "Factorial value: " << factorial(N) << std::endl;
    std::cout << "Execution time: " << seconds << " seconds" << std::endl;

    return 0;
}