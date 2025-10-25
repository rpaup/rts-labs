#include <iostream>
#include <thread>
#include <chrono>
#include <string>
#include <vector>

void calculate_factorials(const std::string& thread_name) {
    auto factorial = [](int n) {
        long long res = 1;
        for (int i = 1; i <= n; ++i) {
            res *= i;
        }
        return res;
        };

    const int N = 10;
    const int ITERATIONS = 10000000; 

    for (int i = 0; i < ITERATIONS; ++i) {
        factorial(N);
    }
}

int main() {
    // --- ПАРАЛЛЕЛЬНЫЙ ЗАПУСК ---
    std::cout << "--- Parallel execution ---" << std::endl;

    auto start_parallel = std::chrono::high_resolution_clock::now();

    // Создаем и запускаем 2 потока
    std::thread thread1(calculate_factorials, "t1");
    std::thread thread2(calculate_factorials, "t2");

    // Ожидаем завершения обоих потоков
    thread1.join();
    thread2.join();

    auto end_parallel = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration_parallel = end_parallel - start_parallel;
    std::cout << "Total parallel execution time: " << duration_parallel.count() << " seconds" << std::endl;
    std::cout << std::endl;


    // --- ПОСЛЕДОВАТЕЛЬНЫЙ ЗАПУСК ---
    std::cout << "--- Sequential execution ---" << std::endl;

    auto start_sequential = std::chrono::high_resolution_clock::now();

    // Вызываем функцию два раза подряд в главном потоке
    calculate_factorials("sequential_1");
    calculate_factorials("sequential_2");

    auto end_sequential = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration_sequential = end_sequential - start_sequential;
    std::cout << "Total sequential execution time: " << duration_sequential.count() << " seconds" << std::endl;

    return 0;
}