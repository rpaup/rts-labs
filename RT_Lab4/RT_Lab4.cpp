#include <iostream>
#include <thread>
#include <mutex>
#include <vector>
#include <string>
#include <chrono>
#include <atomic>
#include <algorithm>

const int EXPLOSION_LIMIT = 10000;

// Эксперимент 1 (Кука уволили): GLUTTONY = 100, EFFICIENCY_FACTOR = 5
// Эксперимент 2 (Толстяки лопнули): GLUTTONY = 100, EFFICIENCY_FACTOR = 1000
// Эксперимент 3 (Кук уволился сам): GLUTTONY = 10, EFFICIENCY_FACTOR = 11
const int GLUTTONY = 10;
const int EFFICIENCY_FACTOR = 11;

std::vector<int> dishes = { 3000, 3000, 3000 };
std::vector<int> eaten_nuggets = { 0, 0, 0 };
std::mutex shared_mutex;
std::atomic<bool> simulation_running(true);
std::string outcome = "Кук уволился сам! (Прошло 5 дней)";

void cook_thread() {
    while (simulation_running) {
        shared_mutex.lock();
        if (simulation_running) {
            for (int i = 0; i < 3; ++i) {
                dishes[i] += EFFICIENCY_FACTOR;
            }
        }
        shared_mutex.unlock();
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

void fat_man_thread(int id) {
    while (simulation_running) {
        shared_mutex.lock();
        if (!simulation_running) {
            shared_mutex.unlock();
            break;
        }

        // Если толстяк уже сыт, он больше не ест, а ждет остальных
        if (eaten_nuggets[id] >= EXPLOSION_LIMIT) {
            shared_mutex.unlock();
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            continue;
        }

        // Рассчитываем, сколько осталось до лимита
        int room_in_stomach = EXPLOSION_LIMIT - eaten_nuggets[id];
        // Определяем, сколько съесть: минимум из того, что есть на тарелке, аппетита и места в желудке
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
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

int main() {
    setlocale(LC_ALL, "Russian");

    std::cout << "Симуляция запущена с параметрами:" << std::endl;
    std::cout << "Обжорство (Толстяки едят): " << GLUTTONY << " наггетсов/раз" << std::endl;
    std::cout << "Эффективность (Повар готовит): " << EFFICIENCY_FACTOR << " наггетсов/партия на тарелку" << std::endl;
    std::cout << "------------------------------------------" << std::endl;

    std::thread cook(cook_thread);
    std::vector<std::thread> fat_men;
    for (int i = 0; i < 3; ++i) {
        fat_men.emplace_back(fat_man_thread, i);
    }

    std::this_thread::sleep_for(std::chrono::seconds(5));
    if (simulation_running) {
        simulation_running = false;
    }

    cook.join();
    for (auto& t : fat_men) {
        t.join();
    }

    std::cout << "------------------------------------------" << std::endl;
    std::cout << "Симуляция завершена!" << std::endl;
    std::cout << "ИТОГ: " << outcome << std::endl;
    std::cout << "------------------------------------------" << std::endl;
    std::cout << "Финальное состояние:" << std::endl;
    for (int i = 0; i < 3; ++i) {
        std::cout << "Толстяк #" << i + 1 << " съел: " << eaten_nuggets[i]
            << " наггетсов. | На тарелке #" << i + 1 << " осталось: " << dishes[i]
            << " наггетсов." << std::endl;
    }

    return 0;
}