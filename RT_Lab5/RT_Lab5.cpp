#include <iostream>
#include <thread>
#include <vector>
#include <string>
#include <chrono>
#include <atomic>
#include <algorithm>

class CustomMutex {
private:
    std::atomic<bool> locked = { false };

public:
    void lock() {
        bool expected = false;
        // Пытаемся изменить locked с false на true.
        // compare_exchange_weak возвращает true, если замена удалась.
        // Если locked было true (занято), то expected станет true, и цикл продолжится.
        while (!locked.compare_exchange_weak(expected, true, std::memory_order_acquire)) {
            expected = false; // Сбрасываем expected обратно в false для следующей попытки

            // yield говорит планировщику ОС, что этот поток можно приостановить,
            // чтобы дать поработать другим. Это предотвращает 100% загрузку ядра пустым циклом.
            std::this_thread::yield();
        }
    }

    void unlock() {
        // Освобождаем мьютекс: просто пишем false
        locked.store(false, std::memory_order_release);
    }
};

const int EXPLOSION_LIMIT = 10000;

// Чтобы проверить работу без sleep, нужно поменять параметры, иначе все закончится за доли секунды.
// Вариант эксперимента "Кука уволили" (Толстяки едят быстрее, чем готовят):
const int GLUTTONY = 100;
const int EFFICIENCY_FACTOR = 5;

std::vector<int> dishes = { 3000, 3000, 3000 };
std::vector<int> eaten_nuggets = { 0, 0, 0 };

CustomMutex shared_mutex;

std::atomic<bool> simulation_running(true);
std::string outcome = "Кук уволился сам! (Время вышло)";

void cook_thread() {
    while (simulation_running) {
        shared_mutex.lock();

        if (simulation_running) {
            for (int i = 0; i < 3; ++i) {
                dishes[i] += EFFICIENCY_FACTOR;
            }
        }

        shared_mutex.unlock();

        std::this_thread::yield();
    }
}

void fat_man_thread(int id) {
    while (simulation_running) {
        if (eaten_nuggets[id] >= EXPLOSION_LIMIT) {
            std::this_thread::yield();
            continue;
        }

        shared_mutex.lock();
        if (!simulation_running) {
            shared_mutex.unlock();
            break;
        }

        int room_in_stomach = EXPLOSION_LIMIT - eaten_nuggets[id];
        int nuggets_to_eat = std::min({ dishes[id], GLUTTONY, room_in_stomach });

        if (nuggets_to_eat > 0) {
            dishes[id] -= nuggets_to_eat;
            eaten_nuggets[id] += nuggets_to_eat;

            if (dishes[id] <= 0) {
                outcome = "Кука уволили! (Тарелка опустела)";
                simulation_running = false;
            }
        }

        if (eaten_nuggets[0] >= EXPLOSION_LIMIT &&
            eaten_nuggets[1] >= EXPLOSION_LIMIT &&
            eaten_nuggets[2] >= EXPLOSION_LIMIT) {
            outcome = "Кук не получил зарплату! (Все толстяки лопнули)";
            simulation_running = false;
        }

        shared_mutex.unlock();
        std::this_thread::yield();
    }
}

int main() {
    setlocale(LC_ALL, "Russian");

    std::cout << "Лабораторная №5 с CustomMutex" << std::endl;

    std::thread cook(cook_thread);
    std::vector<std::thread> fat_men;
    for (int i = 0; i < 3; ++i) {
        fat_men.emplace_back(fat_man_thread, i);
    }

    while (simulation_running) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        static int ticks = 0;
        if (++ticks > 50) {
            simulation_running = false;
            break;
        }
    }

    cook.join();
    for (auto& t : fat_men) {
        t.join();
    }

    std::cout << "------------------------------------------" << std::endl;
    std::cout << "ИТОГ: " << outcome << std::endl;
    std::cout << "------------------------------------------" << std::endl;
    std::cout << "Финальное состояние:" << std::endl;
    for (int i = 0; i < 3; ++i) {
        std::cout << "Толстяк #" << i + 1 << " съел: " << eaten_nuggets[i]
            << " наггетсов. | Ост. на тарелке: " << dishes[i]
            << " наггетсов." << std::endl;
    }

    return 0;
}