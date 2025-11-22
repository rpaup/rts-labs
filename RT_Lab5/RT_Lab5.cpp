#include <iostream>
#include <thread>
#include <vector>
#include <string>
#include <chrono>
#include <atomic>
#include <algorithm> 

const int GLUTTONY = 10;
const int EFFICIENCY_FACTOR = 11;

const int EXPLOSION_LIMIT = 10000;


class CustomMutex {
private:
    std::atomic<bool> _locked;

public:
    CustomMutex() : _locked(false) {}

    void lock() {
        bool expected = false;
        while (!_locked.compare_exchange_weak(expected, true, std::memory_order_acquire)) {
            expected = false;
            std::this_thread::yield();
        }
    }

    void unlock() {
        _locked.store(false, std::memory_order_release);
    }
};

std::vector<int> dishes = { 3000, 3000, 3000 };
std::vector<int> eaten_nuggets = { 0, 0, 0 };

CustomMutex my_mutex;

std::atomic<bool> simulation_running(true);
std::string outcome = "Кук уволился сам! (Прошло 5 дней)";

std::atomic<int> current_round(0);
std::atomic<int> finished_eaters_count(3);

void cook_thread() {
    while (simulation_running) {
        while (finished_eaters_count.load() < 3) {
            if (!simulation_running) return;
            std::this_thread::yield();
        }

        my_mutex.lock();
        if (!simulation_running) { my_mutex.unlock(); break; }

        for (int i = 0; i < 3; ++i) {
            dishes[i] += EFFICIENCY_FACTOR;
        }

        my_mutex.unlock();

        finished_eaters_count.store(0);
        current_round.fetch_add(1);
    }
}

void fat_man_thread(int id) {
    int last_round_seen = 0;

    while (simulation_running) {
        while (current_round.load() <= last_round_seen) {
            if (!simulation_running) return;
            std::this_thread::yield();
        }

        my_mutex.lock();
        if (!simulation_running) { my_mutex.unlock(); break; }

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

        my_mutex.unlock();

        std::this_thread::sleep_for(std::chrono::milliseconds(10));

        last_round_seen = current_round.load();
        finished_eaters_count.fetch_add(1);
    }
}

int main() {
    setlocale(LC_ALL, "Russian");

    std::cout << "GLUTTONY: " << GLUTTONY << std::endl;
    std::cout << "EFFICIENCY_FACTOR: " << EFFICIENCY_FACTOR << std::endl;
    std::cout << "------------------------------------------" << std::endl;

    std::thread cook(cook_thread);
    std::vector<std::thread> fat_men;
    for (int i = 0; i < 3; ++i) {
        fat_men.emplace_back(fat_man_thread, i);
    }

    auto start_time = std::chrono::steady_clock::now();
    while (simulation_running) {
        auto current_time = std::chrono::steady_clock::now();
        if (std::chrono::duration_cast<std::chrono::seconds>(current_time - start_time).count() >= 5) {
            simulation_running = false;
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    if (cook.joinable()) cook.join();
    for (auto& t : fat_men) {
        if (t.joinable()) t.join();
    }

    std::cout << "------------------------------------------" << std::endl;
    std::cout << "Симуляция завершена!" << std::endl;
    std::cout << "ИТОГ: " << outcome << std::endl;
    std::cout << "------------------------------------------" << std::endl;

    for (int i = 0; i < 3; ++i) {
        std::cout << "Толстяк #" << i + 1 << " съел: " << eaten_nuggets[i]
            << " | Осталось: " << dishes[i] << std::endl;
    }

    return 0;
}