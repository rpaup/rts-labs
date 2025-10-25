#include <iostream>
#include <thread>
#include <mutex>
#include <string>
#include <chrono>

// Глобальный мьютекс для защиты std::cout
std::mutex m;

// Функция, выполняемая в каждом потоке
void Func(std::string name) {
    // Используем long double для максимальной точности
    long double i = 0.0;
    const long double increment = 1e-9; // 10^(-9)

    // Засекаем время старта
    auto start_time = std::chrono::high_resolution_clock::now();

    // Цикл, который выполняется ровно 1 секунду
    while (std::chrono::high_resolution_clock::now() - start_time < std::chrono::seconds(1)) {
        i += increment;
    }

    // Блокируем мьютекс перед выводом в консоль
    m.lock();
    std::cout << "Thread " << name << " finished with i = " << i << std::endl;
    // Разблокируем мьютекс после вывода
    m.unlock();
}

int main() {
    // Устанавливаем высокую точность для вывода long double
    std::cout.precision(10);

    // Создаем и запускаем 3 потока
    std::thread thread1(Func, "t1");
    std::thread thread2(Func, "t2");
    std::thread thread3(Func, "t3");

    // Ожидаем завершения всех потоков
    thread1.join();
    thread2.join();
    thread3.join();

    return 0;
}